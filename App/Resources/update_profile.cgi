#!/bin/bash

echo "Content-type: text/html"
echo ""

echo "<html><body>"
echo "<h1>Profile updated successfully!</h1>"

# Citește parametrii din corpul cererii
while read line; do
    echo "<p>$line</p>"
done <&0

echo "</body></html>"
