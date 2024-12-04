#!/bin/bash

echo "Content-type: text/html"
echo ""

# Titlul răspunsului
echo "<html><body>"
echo "<h1>Profile updated successfully!</h1>"

# Verificăm dacă există variabila CONTENT_LENGTH
if [ -z "$CONTENT_LENGTH" ]; then
    echo "<p>No content length provided.</p>"
else
    # Citim datele din cerere
    read -n $CONTENT_LENGTH body

    # Afisam parametrii primiți
    echo "<p>Received data:</p>"
    
    # Decomprimăm datele URL-encoded și le afișăm
    echo "<ul>"
    # Procesăm fiecare pereche key-value
    for param in $(echo $body | tr '&' '\n'); do
        key=$(echo $param | cut -d'=' -f1)
        value=$(echo $param | cut -d'=' -f2- | sed 's/%20/ /g' | sed 's/%40/@/g')  # Decodificăm %20 și %40
        echo "<li><b>$key</b>: $value</li>"
    done
    echo "</ul>"
fi

# Mesaj de confirmare final
echo "<p>Profile successfully updated!</p>"

echo "</body></html>"

# Mesaj de debugging pentru server (se trimite în stderr)
echo "Debug: Scriptul CGI a terminat" >&2
