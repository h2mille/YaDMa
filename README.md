# YaDMa
Yet another Door Manager

This is a still in developpement door manager system for fablabs, openspace, and CAC40 companies.

Idea is simple:
-a raspberry server with a SQLite database of people who can go in
-one or many esp8266 clients, one per door, who check to the server if each nfc card can come in.
-all in wifi, with aes-256bit asymetrical encrypted communication, for maximum safety

What is needed?
-a raspberry pi or linux server
-one nodemcu per door
-one MFRC522 board per door (connected by SPI)
-one PN532 board for the server, to easyier add clients (by uart)
-ndef213 beacons (one per user)


How to use it?
Right now, all is working but control system of each door.
NodeMCU part:
-connect the MFRC522 board to it by spi (usual pinout)+ pin writen on top of the ino file
-use MFRC522 library
In the ino file, change:
-wifi connection settings 
-if requiered: pinout
-IP of the Linux server
-both Encrypt and Decrypt keys
And upload the file to the eps8266

On the raspberry side:
You will need apache+php server, and python 3.*
You will also need libnfc(and so cmake), and nfc-bindings to use nfc.
-Make it and keep it safe from hackers.
-configure to enable on your device SPI, UART, or I2C depending of the protocol you wish (I use UART)
-connect to the network
-copy "home" files to your home directory
-copy "php" file to /var/www/html/index.php, and remove index.html
-modify end of c files for the new keys (in aes_encrypt/aes_decrypt, use esp encrypt key, and in the 2 other files, the other key)
-update in the php file, and python files database name
-launch python setup.py to create the database
-Use add_user.py to add a user.

Still to do:
-better settings for configuring each doors
-add a dafe unique key in sucured memory of ncf card to check they are not copies
-add servo control system on the nodemcu
-add scripts to remove or modify users from database
-log files to check usage of the door
