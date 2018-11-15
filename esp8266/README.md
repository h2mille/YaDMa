# YaDMa
Yet another Door Manager

Here we come to the nodemcu side.
Please connect the nodeMCU to:
A0=> a free wire as antenna
D1=> button to free the door from the inside
SPI usual pin:
D5=>SCK              (14 internal pin)
D6=>MISO             (12 internal pin)
D7=>MOSI             (13 internal pin)
D8=>SS               (15 internal pin)
D3=>reader RESET pin ( 0 internal pin)

you can plug the A1-16 servo to the normal tx and rx pin (D0&D1),
but provide the power supply from an external source (12v)

Now in the YadoM.ino, please make this changes:
encrypt(from arduino to server) and decryp key(from server to arduino) on lines 34 and 36
Please be sure the key are the same on the server. And be sure encrypt and decrypt keys are different
This will avoid man on the middle easy hack.

On linek 250 and 254, please change the NFC password and pack data to the same than on the server.
Note that there is two lines, this is to create cards with différent allowance.
The second one is to the person access only if there is already someone in there.

Now, you can compile and flash the nodemcu.
