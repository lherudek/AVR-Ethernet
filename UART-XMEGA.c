/**
 * @author  Lukas Herudek
 * @email   lukas.herudek@gmail.com
 * @version v1.0
 * @ide     Atmel Studio 6.2
 * @license GNU GPL v3
 * @brief   UART library for AVR XMEGA
 * @verbatim
   ----------------------------------------------------------------------
    Copyright (C) Lukas Herudek, 2018
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.
     
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.
	
	<http://www.gnu.org/licenses/>
@endverbatim
 */
 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include "W5500.h"
#include "UART-XMEGA.h"


void UART_init(void)
{
	PORTF.DIRSET = PIN3_bm;
	PORTF.DIRCLR = PIN2_bm;
	PORTF.OUTSET = PIN3_bm | PIN2_bm;
	
	USARTF0.CTRLA = USART_RXCINTLVL_LO_gc;//enable receive interrupt
	USARTF0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;//enable RX and TX units
	USARTF0.CTRLC = USART_CHSIZE_8BIT_gc;//8bit char size
	USARTF0.BAUDCTRLA = (F_CPU/(16*UART_BAUDRATE))-1;//calculate closest match to defined baud rate
	USARTF0.BAUDCTRLB = 0;
}

void UART_TX(unsigned char TX_data)
{
	while(!(USARTF0_STATUS & USART_DREIF_bm));//Wait until DATA buffer is empty
	USARTF0_DATA = TX_data;
}

unsigned char UART_RX(void)
{
	while(!(USARTF0_STATUS & USART_RXCIF_bm));//Wait until buffer is full
	return USARTF0_DATA;
}

void UART_TX_string(unsigned char* TX_string)
{
	while(*TX_string)
	UART_TX(*TX_string++);
}

void sendString(char data[])
{
	unsigned char i = 0;
	while (data[i] != '\0')
	{
		UART_TX(data[i]);
		i++;
	}
}


void printOctetDec(unsigned char octet)
{
	unsigned char hundreds = octet / 100;
	octet = octet % 100;
	unsigned char tens = octet / 10;
	octet = octet % 10;
	unsigned char ones = octet;
	UART_TX(hundreds + 48);
	UART_TX(tens + 48);
	UART_TX(ones + 48);
}

void printOctetHex(unsigned char octet)
{
	// yes I know I should do a bitshift instead
	unsigned char sixteens = octet / 16;
	if(sixteens > 9)
	sixteens+=7; // to skip to A,B,C,D,E,F
	octet = octet % 16;
	unsigned char ones = octet;
	if(ones > 9)
	ones+=7; // to skip to A,B,C,D,E,F
	UART_TX(sixteens + 48);
	UART_TX(ones + 48);
}

char processString(char data[], unsigned char toPrint)
{
	unsigned char i = 0;
	char last = '0';
	while (data[i] != '\0')
	{
		last = data[i];
		/*if(toPrint)
			UART_TX(last);*/
		i++;
	}
	return last;
}




