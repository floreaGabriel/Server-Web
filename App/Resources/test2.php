<?php
$name = htmlspecialchars($_GET['name'] ?? 'N/A');
$email = htmlspecialchars($_GET['email'] ?? 'N/A');

echo "<!DOCTYPE html>
<html lang='en'>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>GET Request Processed</title>
</head>
<body>
    <h1>GET Request Processed</h1>
    <p><strong>Name:</strong> $name</p>
    <p><strong>Email:</strong> $email</p>
</body>
</html>";
?>
