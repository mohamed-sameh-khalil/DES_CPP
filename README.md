# DES_CPP
Implementation of DES algorithm using C++

To use the DES first call the function ```makekeys(key)``` with a 56 bit key to prepare keys needed for DES
if you need to change the key just call ```makekeys(key)``` with a new key

To encrypt or decrypt call ```DES(plain, choice)``` where ```plain``` is the 64 bit plain text and ```choice``` is 1 for encryption and 0 for decryption,and the returned value is 64 bit cipher/plain text
