SDRaw
=====

*** This Library has been modified to make use of the Arduino Due Entended SPI Features
*** As a result you can only use this library on the Arduino Due digital pins 4, 10 or 52 for the Slave Select

Use SD Card as EEPROM Arduino Library

A lot of work is still needed on this library to make it handle other data types ( other than String ).

At the moment you can write a string and read a string from 512byte sectors on the SD Card.

Usage:

writestring(String, SD Card Sector); // writes the "String" to the SD Card Sector of your choosing.

String = readstring(SD Card Sector); // Reads from the SD Card sector and returns a string. 

Performance:

With a standard 2GB Micro SD Card and a Sparkfun SD Shield ( with connecting wires )

Read Speed is between 1570 and 1700 microseconds
Write Speed is between 6000 and 8000 microseconds

This is with the SPI.setClockDivider set at 3 ( 28 Mhz ) for read and writes. Any faster and it cannot read properly.