/*###############################################################
 # Rob Brown                                                    #
 # Elite Engineering WA Pty Ltd.                                #
 # www.elitewa.com.au                                           #
 #                                                              #
 # License : Released under GPL v3                              #
 ###############################################################*/

#include "SDRaw.h"
#include <SPI.h>

SDRaw::SDRaw(byte pin) {
	_pin = pin;
}

unsigned char SDRaw::SD_reset() {
	unsigned char response;
	unsigned int retry = 0;

	SPI.begin(_pin);
	SPI.setDataMode(_pin, SPI_MODE0);
	SPI.setBitOrder(_pin, MSBFIRST);
	SPI.setClockDivider(_pin, 7);

	do {
		response = SD_sendCommand(0, 0); //send 'reset & go idle' command
		if (++retry > 254)
			return 1;
	} while (response != 0x01);

	retry = 0;
	do {
		response = SD_sendCommand(1, 0); //activate card's initialization process
		if (++retry > 254)
			return 2;
	} while (response);

	retry = 0;
	do {
		response = SD_sendCommand(16, 512); //set block size to 512
		if (++retry > 10)
			return 8; //time out  
	} while (response);

	return 0;

}//end of SD_reset function

String SDRaw::readstring(unsigned long Rstartblock) {
	unsigned char response;
	unsigned int i, retry = 0;
	char buffer[512];
	String readstring;
	String resp;

	response = SD_reset(); // reset the sd card   
	resp += response;
	SPI.setClockDivider(_pin, 3); //use fast speed for read block
	if (response != 0)
		return resp; //board wont reset return error
	do {
		response = SD_sendCommand(17, Rstartblock << 9); //read a Block command
		//block address converted to starting address of 512 byte Block	
		if (++retry > 10)
			return "3";

	} while (response);

	retry = 0;
	/*Read and write commands have data transfers associated with them. Data is being transmitted or
	 received via data tokens. All data bytes are transmitted MSB first.
	 First byte: Start Block
	 1 1 1 1 1 1 1 0
	 Bytes 2-513 (depends on the data block length): User data; Last two bytes: 16 bit CRC.*/

	while (SPI.transfer(_pin, 0xff) != 0xfe) { //wait for start block token 0xfe  
		if (retry++ > 600)
			return "4"; //end time out for token
	} //end receive token

	for (i = 0; i < 512; i++) //read 512 bytes
		buffer[i] = SPI.transfer(_pin, 0xff, SPI_CONTINUE);
	SPI.transfer(_pin, 0xff, SPI_CONTINUE); //receive incoming CRC (16-bit), CRC is ignored here
	SPI.transfer(_pin, 0xff, SPI_CONTINUE);
	SPI.transfer(_pin, 0xff); //extra 8 clock pulses

	for (int i = 0; i < 512; i++) {
		char ch = buffer[i];
		if (ch == '^') {
			break;
		}
		if (ch != 0x00) {
			readstring += ch;
		}
	}

	return readstring;
} //end of read block

unsigned char SDRaw::writestring(String writestring, unsigned long Wstartblock) {
	unsigned char response;
	unsigned int i, retry = 0;
	char buffer[512];
	writestring += '^';
	writestring.toCharArray(buffer, writestring.length() + 1);

	response = SD_reset(); // reset the sd card
	SPI.setClockDivider(_pin, 3); //use fast speed for write block

	if (response != 0)
		return response; //board wont reset return error
	do {
		response = SD_sendCommand(24, Wstartblock << 9); //write a Block command
		if (++retry > 10)
			return 5; //time out

	} while (response);

	SPI.transfer(_pin, 0xfe, SPI_CONTINUE); //Send start block token 0xfe (0x11111110)
	for (i = 0; i < 512; i++) //send 512 bytes data
		SPI.transfer(_pin, buffer[i], SPI_CONTINUE);
	SPI.transfer(_pin, 0xff, SPI_CONTINUE); //transmit dummy CRC (16-bit), CRC is ignored here
	SPI.transfer(_pin, 0xff, SPI_CONTINUE);
	response = SPI.transfer(_pin, 0xff);

	/*Every data block written to the card will be acknowledged by a data response token. It is one byte long
	 and has the following format:
	 x x x 0 Status 1
	 The meaning of the status bits is defined as follows:
	 010 - Data accepted.
	 101 - Data rejected due to a CRC error.
	 110 - Data Rejected due to a Write Error*/

	if ((response & 0x1f) != 0x05) {
		return 6;
	}
	/*Every data block has a prefix of \u2019Start Block\u2019 token (one byte).
	 After a data block has been received, the card will respond with a data-response token. If the data block
	 has been received without errors, it will be programmed. As long as the card is busy programming, a
	 continuous stream of busy tokens will be sent to the host (effectively holding the DataOut line low).*/
	while (!SPI.transfer(_pin, 0xff)) //wait for SD card to complete writing and get idle
		if (retry++ > 60000) {
			return 7;
		}

	return 0;
} //end of write a block of data

unsigned char SDRaw::SD_sendCommand(unsigned char cmd, unsigned long arg) {
	/*The response token is sent by the card after every command with the exception of SEND_STATUS
	 commands. It is one byte long, and the MSB is always set to zero. The other bits are error indications,
	 an error being signaled by a 1. The structure of the R1 format is given in Figure 7-9. The meaning of the
	 flags is defined as following:
	 In idle state: The card is in idle state and running the initializing process.
	 Erase reset: An erase sequence was cleared before executing because an out of erase sequence
	 command was received.
	 Illegal command: An illegal command code was detected.
	 Communication CRC error: The CRC check of the last command failed.
	 Erase sequence error: An error in the sequence of erase commands occurred.
	 Address error: A misaligned address that did not match the block length was used in the command.
	 Parameter error: The command\u2019s argument (e.g. address, block length) was outside the allowed*/
	unsigned char response, retry = 0, i;

	for (i = 0; i < 5; i++) //presend calls
		SPI.transfer(_pin, 0xff, SPI_CONTINUE); //need wakeup calls
	SPI.transfer(_pin, cmd | 0x40, SPI_CONTINUE); //send command, first two bits always '01'
	SPI.transfer(_pin, arg >> 24, SPI_CONTINUE);
	SPI.transfer(_pin, arg >> 16, SPI_CONTINUE);
	SPI.transfer(_pin, arg >> 8, SPI_CONTINUE);
	SPI.transfer(_pin, arg, SPI_CONTINUE);
	SPI.transfer(_pin, 0x95); //crc does not matter except for cmd0

	while ((response = SPI.transfer(_pin, 0xff)) == 0xff) //wait response
		if (retry++ > 254)
			break; //time out error

	return response; //return state
}