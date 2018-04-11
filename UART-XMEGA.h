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

#define F_CPU			32000000UL
#define UART_BAUDRATE	9600UL


#ifndef UART_XMEGA_H
#define UART_XMEGA_H

void UART_init(void);
void UART_TX(unsigned char TX_data);
unsigned char UART_RX(void);
void UART_TX_string(unsigned char* TX_string);
void sendString(char data[]);
void printOctetDec(uint8_t octet);
void printOctetHex(uint8_t octet);
char processString(char data[], uint8_t toPrint);


#endif //UART_XMEGA_H