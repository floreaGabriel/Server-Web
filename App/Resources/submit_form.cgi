#!/bin/bash

# Asigură-te că trimiți capetele HTTP înainte de a trimite orice altceva
echo "Content-type: text/html"
echo "" # Este important să fie un newline aici înainte de a trimite corpul răspunsului

echo "<html><body>"
echo "<h1>Form submitted successfully!</h1>"

# Citește parametrii POST
name=$(echo "$QUERY_STRING" | sed 's/.*name=\([^&]*\).*/\1/')
email=$(echo "$QUERY_STRING" | sed 's/.*email=\([^&]*\).*/\1/')

# Decodifică parametrii URL-encoded
name=$(echo "$name" | sed 's/%20/ /g')
email=$(echo "$email" | sed 's/%40/@/g')

echo "<p>Name: $name</p>"
echo "<p>Email: $email</p>"

echo "</body></html>"
