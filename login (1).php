<?php
// Hardcoded credentials for testing
$correct_username = "admin";
$correct_password = "wormix";

// Check if the form was submitted
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    // Retrieve username and password from POST data
    $username = isset($_POST["username"]) ? $_POST["username"] : '';
    $password = isset($_POST["password"]) ? $_POST["password"] : '';

    // Compare with hardcoded credentials
    if ($username === $correct_username && $password === $correct_password) {
        // Redirect to homepage
        header('Location: homepage.html');
        exit();
    } else {
        // Redirect back to the login page
        http_response_code(401);
        echo "<script>alert('Wrong password!'); window.location.href='index.html';</script>";
        exit();
    }
} else {
    // Handle direct access to this script
    echo "No data submitted!";
}
?>
