/*###############################################################
 # Rob Brown                                                    #
 # Elite Engineering WA Pty Ltd.                                #
 # www.elitewa.com.au                                           #
 #                                                              #
 # License : Released under GPL v3                              #
 ###############################################################*/

#ifndef SDRAW_h
#define SDRAW_h

#include <Arduino.h>

class SDRaw {
public:
	SDRaw(byte pin);
	String readstring(unsigned long Rstartblock);
	unsigned char writestring(String writestring, unsigned long Wstartblock);

private:
	byte _pin;
	unsigned char SD_reset();
	unsigned char SD_sendCommand(unsigned char cmd, unsigned long arg);
	unsigned char SPI_transmit(unsigned char data);
};

#endif
