# YaDMa
Yet another Door Manager

Here you must play a bit of compilation.

with python, create a database you need:
$python setup.python

aes_decrypt and aes_encrypt are to decrypt and encypt the keys from the nodemcu device. pleas set the key with the random values you wish, line 671 or 675

aes_decrypt_back and aes_encrypt_back are to decrypt and encypt the keys back to the nodemcuu device. please set the key with the random values you wish, line 671 or 675
please be sure the key is not the same in both way, as hacker might then hack the system easily with a man on the middle attack.

compile every c files:
$gcc -o aes_decrypt aes_decrypt.c
$gcc -o aes_encrypt aes_encrypt.c
$gcc -o aes_decrypt_back aes_decrypt_back.c
$gcc -o aes_encrypt_back aes_encrypt_back.c

Here we come to the funny part... as depending of your nfc reader. you can install and use mifare library and freefare library for C.
you can use PN532 in HSU mode (high speed uart)
in set_auth, you can change on line 60 and 61, the  pwd, and the pack. Please change with the same on the nodeMCU too. This is a no copy security.
Then compile set_auth.c:
$gcc -o set_auth set_auth.c

If your reach here, that's great.
Last step is to add users by doing:
$python ass_user.python
Then set the card, answer the question, and set the card again on the reader.

If you want to make any copy or change on the database, just use any tools to read the sqlite file.