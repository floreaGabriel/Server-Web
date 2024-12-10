<?php
// Asigurăm că cererea este POST
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Preluăm datele trimise prin POST
    $data = file_get_contents("php://input");

    // Parsăm datele din body-ul cererii
    parse_str($data, $parsedData); // Transformă datele URL-encoded într-un array

    $name = htmlspecialchars($parsedData['name'] ?? 'N/A');
    $email = htmlspecialchars($parsedData['email'] ?? 'N/A');

    // Creăm un mesaj de salvare
    $message = "Name: $name, Email: $email\n";

    // Salvăm datele într-un fișier (append la final)
    $file = 'data.txt';
    if (file_put_contents($file, $message, FILE_APPEND)) {
        // Returnăm un răspuns intuitiv la client
        echo "<h1>Data received and saved successfully!</h1>";
        echo "<p><strong>Name:</strong> $name</p>";
        echo "<p><strong>Email:</strong> $email</p>";
        echo "<p>Data has been saved in <code>$file</code>.</p>";
    } else {
        // Dacă salvarea eșuează
        echo "<h1>Error saving data!</h1>";
        echo "<p>There was an error saving your data. Please try again later.</p>";
    }
} else {
    // Mesaj pentru cererile care nu sunt POST
    echo "<h1>Invalid request</h1>";
    echo "<p>This script only handles POST requests.</p>";
}
?>
