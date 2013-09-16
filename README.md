SDRaw
=====

Use SD Card as EEPROM Arduino Library

A lot of work is still needed on this library to make it handle other data types ( other than String ).

At the moment you can write a string and read a string from 512byte sectors on the SD Card.

Usage:

writestring(String, SD Card Sector); // writes the "String" to the SD Card Sector of your choosing.

String = readstring(SD Card Sector); // Reads from the SD Card sector and returns a string. 
