/**
 * @author  Lukas Herudek
 * @email   lukas.herudek@gmail.com
 * @version v1.0
 * @ide     Atmel Studio 6.2
 * @license GNU GPL v3
 * @brief   Wiznet W5500 library for AVR XMEGA
 * @verbatim
	Wiznet W5500 library for AVR XMEGA - TCP server and client implemented
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
 
 
#include <stdint.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include "UART-XMEGA.h"
#include "W5500.h"


//Private prototypes

void ethernetSPIinit(void);
void ethernetSPItx8(unsigned char data);
unsigned char ethernetSPIrx8();
void ethernetSPItx16(unsigned int data);
void ethernetTXdata8(unsigned int address, unsigned char block, unsigned char data);
unsigned char ethernetRXdata8(unsigned int address, unsigned char block);
unsigned int ethernetRXdata16(unsigned char lsbAddr, unsigned char socket);
void ethernetTXdata16(unsigned char lsbAddr, unsigned char socket, unsigned int data);
void ethernetWrite4Bytes(unsigned char byte0, unsigned char byte1, unsigned char byte2, unsigned char byte3, unsigned int reg);
void ethernetWrite6Bytes(unsigned char byte0, unsigned char byte1, unsigned char byte2, unsigned char byte3, unsigned char byte4, unsigned char byte5, unsigned int reg);
//////////////////////////////////////////////////////////////////////////
unsigned char ethernetGetStatus(unsigned char socket);
void ethernetSetStatus(unsigned char socket, unsigned char data);
unsigned char ethernetIsEstablished(unsigned char socket);
unsigned char ethernetCheckIfReceivedData(unsigned char socket);
unsigned int ethernetSocketReceiveData(unsigned char socket, char data[]);
void ethernetSendData(unsigned char socket, char data[], unsigned int length);
void ethernetSendText(unsigned char socket, const char data[]);
void ethernetSendTextf(unsigned char socket, char *data, ...);
unsigned char ethernetCheckIfFINreceived(unsigned char socket);
unsigned char ethernetCheckIfCloseOrTimeout(unsigned char socket);
void ethernetSocketDisconnect(unsigned char socket);
void ethernetSocketClose(unsigned char socket);
unsigned char ethernetSocketOpen(unsigned char socket, unsigned int socketPort);
unsigned char ethernetSocketListen(unsigned char socket);
void ethernetPrintSocketStatus(unsigned char socket);
void ethernetSocketConnect(unsigned char socket, IPaddressAndPort server);//Write IP address and server port


//TCP server and client
unsigned char TCPserverInit(unsigned char socket, unsigned int socketPort);
unsigned char serverProcessReceivedData(unsigned char socket, char data[], unsigned int length);
void sendHTMLHeader(unsigned char socket);
unsigned char clientSendCommand(unsigned char socket, unsigned long command);
unsigned char clientProcessReceivedData(unsigned char socket, char data[], unsigned int length, unsigned long *command);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//change this part if using on other platforms

#define CS_ENABLE() (PORTE_OUTCLR = 0b00010000);_delay_us(1);
#define CS_DISABLE() (PORTE_OUTSET = 0b00010000)

void ethernetSPIinit(void)
{
	PORTE_DIRSET = 0b10110000;
	PORTE_DIRCLR = 0b01000000;
	PORTE_OUTSET = 0b00010000;
	PORTE_OUTCLR = 0b11100000;
	
	//SPIE_CTRL = 0b11010010;//CLK2x, Enable, data order, master mode, transfer mode, prescaler//CLK/8
	SPIE_CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | SPI_MODE_0_gc | SPI_PRESCALER_DIV4_gc;
	//SPIE_CTRL = 0b01010011;//CLK2x, Enable, data order, master mode, transfer mode, prescaler//CLK/128
	SPIE_INTCTRL = 0b00000000;//interrupt level = int disabled
	//SPIC_DATA = 0;
}

void ethernetSPItx8(unsigned char data)
{
	SPIE_DATA = data;//sending data
	while(!(SPIE_STATUS&0b10000000));//wait for data transfer complete
}

unsigned char ethernetSPIrx8()
{
	SPIE_DATA = 0xFF;//dummy byte
	while(!(SPIE_STATUS&0b10000000));//wait for data transfer complete

	return SPIE_DATA;//returning received data
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


void ethernetSPItx16(unsigned int data)
{
	ethernetSPItx8(data >> 8);
	ethernetSPItx8(data & 0x00FF);
}

void ethernetTXdata8(unsigned int address, unsigned char block, unsigned char data)
{
	CS_ENABLE();
	ethernetSPItx16(address);
	ethernetSPItx8((block << 3) + 0b00000101);//enable write //one byte size
	ethernetSPItx8(data);
	CS_DISABLE();
}

unsigned char ethernetRXdata8(unsigned int address, unsigned char block)
{
	unsigned char RXdata;

	CS_ENABLE();
	ethernetSPItx16(address);
	ethernetSPItx8((block << 3) + 0b00000001);//enable read //one byte size
	RXdata = ethernetSPIrx8();
	CS_DISABLE();
	return RXdata;//returning received data
}

unsigned int ethernetRXdata16(unsigned char lsbAddr, unsigned char socket)
{
	return ((ethernetRXdata8(lsbAddr-1, socket)<<8) + ethernetRXdata8(lsbAddr, socket));
}

void ethernetTXdata16(unsigned char lsbAddr, unsigned char socket, unsigned int data)
{
	ethernetTXdata8(lsbAddr, socket, data&0x00FF);
	ethernetTXdata8(lsbAddr-1, socket, data>>8);
}


void ethernetWrite4Bytes(unsigned char byte0, unsigned char byte1, unsigned char byte2, unsigned char byte3, unsigned int reg)
{
	ethernetTXdata8(reg, 0, byte0);
	ethernetTXdata8(reg+1, 0, byte1);
	ethernetTXdata8(reg+2, 0, byte2);
	ethernetTXdata8(reg+3, 0, byte3);
}

void ethernetWrite6Bytes(unsigned char byte0, unsigned char byte1, unsigned char byte2, unsigned char byte3, unsigned char byte4, unsigned char byte5, unsigned int reg)
{
	ethernetTXdata8(reg, 0, byte0);
	ethernetTXdata8(reg+1, 0, byte1);
	ethernetTXdata8(reg+2, 0, byte2);
	ethernetTXdata8(reg+3, 0, byte3);
	ethernetTXdata8(reg+4, 0, byte4);
	ethernetTXdata8(reg+5, 0, byte5);
}

//////////////////////////////////////////////////////////////////////////
unsigned char ethernetGetStatus(unsigned char socket)
{
	return ethernetRXdata8(Sn_SR, socket);
}

void ethernetSetStatus(unsigned char socket, unsigned char data)
{
	ethernetTXdata8(Sn_CR, socket, data);
}

unsigned char ethernetIsEstablished(unsigned char socket)
{
	if(ethernetGetStatus(socket) == SOCK_ESTABLISHED)
		return OK;
	else
		return FAIL;
}

unsigned char ethernetCheckIfReceivedData(unsigned char socket)
{
	if(ethernetRXdata16(Sn_RX_RSR_L, socket) != 0)//some data was received
		return OK;
	else
		return FAIL;
}

unsigned int ethernetSocketReceiveData(unsigned char socket, char data[])
{
	unsigned int length, i;
	unsigned int readPtr = ethernetRXdata16(Sn_RX_RD_L, socket);//get read address
	
	do //length register in W5500 can change value during receiving, so read until value is not changed = receive complete
	{
		length = ethernetRXdata16(Sn_RX_RSR_L, socket);
	}while(length != ethernetRXdata16(Sn_RX_RSR_L, socket));
	
	CS_ENABLE();
	ethernetSPItx16(readPtr);
	ethernetSPItx8(((socket + 2) << 3) + 0b00000000);// +2 to get RXBUF //enable read //variable data size
	
	for(i=0; i<length; i++)
	{
		data[i] = ethernetSPIrx8();
		UART_TX(data[i]);
//////////////////////////////////////////////////////////////////////////
	}
	
	CS_DISABLE();
	data[i] = '\0';
	
	ethernetTXdata16(Sn_RX_RD_L, socket, readPtr+length);
	ethernetSetStatus(socket, Sn_RECV);
	
	//return RX data length
	return (length);// >0 if some data was received
}

void ethernetSendData(unsigned char socket, char data[], unsigned int length)
{
	unsigned int i;
	unsigned int writePtr = ethernetRXdata16(Sn_TX_WR_L, socket);//get the TX Write Pointer
	
	if(length == CALCULATE_LENGTH)
	{
		length = strlen(data);
	}
	
	CS_ENABLE();
	ethernetSPItx16(writePtr);
	ethernetSPItx8(((socket + 1) << 3) + 0b00000100);//+1 to get TXBUF //enable write //variable data size
	
	for(i=0; i<length; i++)
	{
		ethernetSPItx8(data[i]);
	}
	CS_DISABLE();
	
	ethernetTXdata16(Sn_TX_WR_L, socket, writePtr + i);
	ethernetSetStatus(socket, Sn_SEND);
}

void ethernetSendText(unsigned char socket, const char data[])
{
	unsigned int i=0;
	unsigned int writePtr = ethernetRXdata16(Sn_TX_WR_L, socket);//get the TX Write Pointer
	
	CS_ENABLE();
	ethernetSPItx16(writePtr);
	ethernetSPItx8(((socket + 1) << 3) + 0b00000100);//+1 to get TXBUF //enable write //variable data size

	while(pgm_read_byte(data))
	{
		ethernetSPItx8(pgm_read_byte(data++));
		i++;
	}
	CS_DISABLE();
	
	ethernetTXdata16(Sn_TX_WR_L, socket, writePtr + i);
	ethernetSetStatus(socket, Sn_SEND);
}

void ethernetSendTextf(unsigned char socket, char *data, ...)
{
	char buffer[128];//how long string it can process
	unsigned int i=0;
	unsigned int writePtr = ethernetRXdata16(Sn_TX_WR_L, socket);//get the TX Write Pointer
	
	va_list pArgs;
	va_start(pArgs, data);
	vsnprintf(buffer, (sizeof(buffer)/sizeof(buffer[0])) - 1, data, pArgs);
	va_end(pArgs);
	
	CS_ENABLE();
	ethernetSPItx16(writePtr);
	ethernetSPItx8(((socket + 1) << 3) + 0b00000100);//+1 to get TXBUF //enable write //variable data size

	while(buffer[i])
	{
		ethernetSPItx8(buffer[i++]);
	}
	CS_DISABLE();
	
	ethernetTXdata16(Sn_TX_WR_L, socket, writePtr + i);
	ethernetSetStatus(socket, Sn_SEND);
}

unsigned char ethernetCheckIfFINreceived(unsigned char socket)
{
	if(ethernetGetStatus(socket) == SOCK_CLOSE_WAIT)
		return OK;
	else
		return FAIL;
}

unsigned char ethernetCheckIfCloseOrTimeout(unsigned char socket)
{
	if(ethernetGetStatus(socket) == SOCK_CLOSED)
		return OK;
	else
		return FAIL;
}

void ethernetSocketDisconnect(unsigned char socket)
{
	ethernetSetStatus(socket, Sn_CR_DISCON);
}

void ethernetSocketClose(unsigned char socket)
{
	ethernetSetStatus(socket, Sn_CR_CLOSE);
}

unsigned char ethernetSocketOpen(unsigned char socket, unsigned int socketPort)
{
	ethernetTXdata8(Sn_MR, socket, Sn_MR_TCP);//set to TCP mode
	ethernetTXdata16(Sn_PORT1, socket, socketPort);
	ethernetTXdata8(Sn_CR, socket, Sn_CR_OPEN);//open
		
	if(ethernetGetStatus(socket) != SOCK_INIT)
	{
		ethernetSocketClose(socket);
		return FAIL;
	}
	else
	{
		return OK;
	}
}

unsigned char ethernetSocketListen(unsigned char socket)
{
	ethernetSetStatus(socket, Sn_CR_LISTEN);//listen
	
	if(ethernetGetStatus(socket) != SOCK_LISTEN)
	{
		ethernetSocketClose(socket);
		return FAIL;
	}
	else
	{
		return OK;
	}
}

void ethernetInit(address IPaddress, address mask, address gateway, MACaddress MACadr)//set IP, Mask, Gateway and MAC address
{
	ethernetSPIinit();
	
	ethernetWrite4Bytes(IPaddress.b0, IPaddress.b1, IPaddress.b2, IPaddress.b3, SIPR);//SOURCE IP ADDRESS
	ethernetWrite4Bytes(mask.b0, mask.b1, mask.b2, mask.b3, SUBR);//SUBNET MASK ADDRESS
	ethernetWrite4Bytes(gateway.b0, gateway.b1, gateway.b2, gateway.b3, GAR);//GATEWAY ADDRESS
	ethernetWrite6Bytes(MACadr.b0, MACadr.b1, MACadr.b2, MACadr.b3, MACadr.b4, MACadr.b5, SHAR);//MAC ADDRESS
}

void ethernetPrintSocketStatus(unsigned char socket)
{
	switch(ethernetGetStatus(socket))
	{
		case SOCK_CLOSED: UART_TX('A'); break;//"SOCK_CLOSED"
		case SOCK_INIT: UART_TX('B'); break;//"SOCK_INIT"
		case SOCK_LISTEN: UART_TX('C'); break;//"SOCK_LISTEN"
		case SOCK_ESTABLISHED: UART_TX('D'); break;//"SOCK_ESTABLISHED"
		case SOCK_CLOSE_WAIT: UART_TX('E'); break;//"SOCK_CLOSE_WAIT"
		case SOCK_UDP: UART_TX('F'); break;//"SOCK_UDP"
		case SOCK_MACRAW: UART_TX('G'); break;//"SOCK_MACRAW"
		case SOCK_SYNSENT: UART_TX('H'); break;//"SOCK_SYNSENT"
		case SOCK_SYNRECV: UART_TX('I'); break;//"SOCK_SYNRECV"
		case SOCK_FIN_WAIT: UART_TX('J'); break;//"SOCK_FIN_WAIT"
		case SOCK_CLOSING: UART_TX('K'); break;//"SOCK_CLOSING"
		case SOCK_TIME_WAIT: UART_TX('L'); break;//"SOCK_TIME_WAIT"
		case SOCK_LAST_ACK: UART_TX('M'); break;//"SOCK_LAST_ACK"
		case SOCK_DOH: UART_TX('N'); break;//"SOCK_DOH"
		case SOCK_MESSUP: UART_TX('O'); break;//"SOCK_MESSUP"
		default: UART_TX('P'); break;
	}
}

void ethernetSocketConnect(unsigned char socket, IPaddressAndPort server)//Write IP address and server port
{
	//ethernetSPIinit();
	ethernetTXdata8(Sn_DIPR0, socket, server.b0);
	ethernetTXdata8(Sn_DIPR0+1, socket, server.b1);
	ethernetTXdata8(Sn_DIPR0+2, socket, server.b2);
	ethernetTXdata8(Sn_DIPR0+3, socket, server.b3);
	
	ethernetTXdata16(Sn_DPORT1, socket, server.socketPort);
	
	ethernetSetStatus(socket, Sn_CR_CONNECT);
	
//	sendString("\nClient init OK\n");
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//TCP server and client


void sendHTMLHeader(unsigned char socket)
{
	ethernetSendText(socket, PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html; charset=windows-1250\r\n\r\n"));
	ethernetSendText(socket, PSTR("<title>My Little Server</title>\r\n"));
}


//NOTE: \r\n line break style for HTTP headers, RFC2616
//NOTE: \r\n\r\n

unsigned char serverProcessReceivedData(unsigned char socket, char data[], unsigned int length)
{
	if(strstr(data, "GET /"))//someone probably wants HTML response...
	{
		if(strstr(data, "GET / "))//main page
		{
			sendHTMLHeader(socket);
			ethernetSendText(socket, PSTR("<h1>This is an experimental server based on W5500</h1>\r\n"));
			ethernetSendText(socket, PSTR("<h4>Help: lukas.herudek@gmail.com / +420 604 837 437</h4>\r\n\r\n"));
			return CONNECTION_CLOSE;
		}
		else if(strstr(data, "/info "))//main page
		{
			sendHTMLHeader(socket);
			ethernetSendText(socket, PSTR("<h3>What did you expect? Some kind of info?</h3>\r\n"));
			ethernetSendText(socket, PSTR("<h4>Help: lukas.herudek@gmail.com / +420 604 837 437</h4>\r\n\r\n"));
			return CONNECTION_CLOSE;
		}
		else
		{
			sendHTMLHeader(socket);
			ethernetSendText(socket, PSTR("<h1>Error 404 - NOT FOUND</h1>\r\n"));
			ethernetSendText(socket, PSTR("<h4>Help: lukas.herudek@gmail.com / +420 604 837 437</h4>\r\n\r\n"));
			return CONNECTION_CLOSE;
		}		
	}
	else//probably just TCP communication
	{
		if(strstr(data, "HELLO"))
		{
			ethernetSendText(socket, PSTR("HELLO 2 YOU!"));
			ethernetSendText(socket, PSTR("AND AGAIN!"));
			return CONNECTION_CLOSE;
		}
		else if(strstr(data, "GET"))
		{
			ethernetSendText(socket, PSTR("GET me if you can!"));
			return CONNECTION_CLOSE;
		}
		else
		{
			ethernetSendText(socket, PSTR("Unknown command"));
			return CONNECTION_CLOSE;
		}
	}
}

void TCPserver(unsigned char socket, unsigned int socketPort)
{
	char RXbuffer[RX_BUFFER_SIZE];
	unsigned int i, length;
	static unsigned int timeoutAlive=0;
	
	if(ethernetIsEstablished(socket) == OK)
	{
		timeoutAlive++;
		
		if(ethernetCheckIfReceivedData(socket) == OK)
		{
			for(i=0; i<RX_BUFFER_SIZE; i++)		RXbuffer[i] = 0;//clear buffer
			timeoutAlive = 0;
			
			length = ethernetSocketReceiveData(socket, RXbuffer);
			if(serverProcessReceivedData(socket, RXbuffer, length) == CONNECTION_CLOSE)
			{
				ethernetSocketDisconnect(socket);
			}
		}
		else
		{
			_delay_ms(1);//for timeout recognition 
		}
	}

	if(ethernetCheckIfFINreceived(socket) == OK)
	{
		ethernetSocketDisconnect(socket);
	}

	if(ethernetCheckIfCloseOrTimeout(socket) == OK  || (timeoutAlive > WAIT_FOR_DATA_RECEIVE))//try 10000 times to receive data, then close socket
	{
		timeoutAlive = 0;
		ethernetSocketDisconnect(socket);
		ethernetSocketClose(socket);//close this socket
		
		TCPserverInit(socket, socketPort);//open and listen to this socket on this port
	}
}

unsigned char TCPserverInit(unsigned char socket, unsigned int socketPort)
{
	if(ethernetSocketOpen(socket, socketPort) == FAIL)	return FAIL;//check if opening socket was successful
	if(ethernetSocketListen(socket) == FAIL)	return FAIL;//check if listening settings set was successful
	
	return OK;
}


unsigned char clientSendCommand(unsigned char socket, unsigned long command)
{
	switch(command)
	{
		//case 0: ethernetSendText(socket, PSTR("This is client!!!\r\n")); return CONNECTION_KEEP_ALIVE;//return CONNECTION_CLOSE;
		
		//case 0: ethernetSendText(socket, PSTR("POST /connect/recive HTTP/1.1\r\nHost: server0.pi-chacka.tipa.eu\r\nContent-Type: application/json\r\nContent-Length: 76\r\n\r\n{\"hostname\":\"terminal2\",\"password\":\"Yn6n9HkjGJ\",\"command\":\"update_datetime\"}")); return CONNECTION_KEEP_ALIVE;//funguje!
		//case 0: ethernetSendText(socket, PSTR("POST /connect/recive HTTP/1.1\r\nHost: server0.pi-chacka.tipa.eu:28080\r\nContent-Type: application/json\r\nContent-Length: 90\r\n\r\n{\"hostname\":\"terminal2\",\"password\":\"Yn6n9HkjGJ\",\"command\":\"update_user!\",\"user_id\":\"9992\"}")); return CONNECTION_KEEP_ALIVE;//má jít i zvenku
		//case 0: ethernetSendText(socket, PSTR("POST /connect/recive HTTP/1.1\r\nHost: server0.pi-chacka.tipa.eu:28080\r\nContent-Type: application/json\r\nContent-Length: 70\r\n\r\n{\"hostname\":\"terminal2\",\"password\":\"Yn6n9HkjGJ\",\"command\":\"get_names\"}")); return CONNECTION_KEEP_ALIVE;
		//case 0: ethernetSendText(socket, PSTR("POST /connect/recive HTTP/1.1\r\nHost: server0.pi-chacka.tipa.eu\r\n\r\n")); return CONNECTION_KEEP_ALIVE;
		//case 0: ethernetSendText(socket, PSTR("GET /index.html HTTP/1.1\r\nHost: stavebnice.tipa.eu\r\n\r\n")); return CONNECTION_KEEP_ALIVE;
		
		
		case 0: ethernetSendText(socket, PSTR("POST /connect/recive HTTP/1.1\r\nHost: server0.pi-chacka.tipa.eu:28080\r\nContent-Type: application/json\r\nContent-Length: 124\r\n\r\n{\"hostname\":\"terminal2\",\"password\":\"Yn6n9HkjGJ\",\"command\":\"insert_new_atro\",\"user_id\":\"9990\",\"type\":\"work\",\"subtype\":\"stop\"}")); return CONNECTION_KEEP_ALIVE;
		
		case 1:	ethernetSendTextf(socket, "Pi is %f or cca %d\r\n", 3.141, 3); return CONNECTION_KEEP_ALIVE;
		case 2: ethernetSendData(socket, "Hello Server World!!!\r\n", CALCULATE_LENGTH);return CONNECTION_KEEP_ALIVE;
	}
	return CONNECTION_CLOSE;
}

unsigned char clientProcessReceivedData(unsigned char socket, char data[], unsigned int length, unsigned long *command)
{
	if(strstr(data, "HELLO"))
	{
		ethernetSendText(socket, PSTR("HELLO 2 YOU!"));
		ethernetSendText(socket, PSTR("AND AGAIN!"));
		*command = 0;
		return CONNECTION_KEEP_ALIVE;
	}
	else if(strstr(data, "GET"))
	{
		ethernetSendText(socket, PSTR("GET me if you can!"));
		return CONNECTION_CLOSE;
	}
	else
	{
		ethernetSendText(socket, PSTR("Unknown command"));
		return CONNECTION_CLOSE;
	}
}

void TCPclient(unsigned char socket, unsigned int sourceSocketPort, IPaddressAndPort server, unsigned long command)
{
	char RXbuffer[RX_BUFFER_SIZE];
	unsigned int i, length;
	unsigned int timeout=0;
	unsigned char dataSent=0;
	
	if(ethernetSocketOpen(socket, sourceSocketPort) == FAIL)	return;//check if opening socket was successful
	
	ethernetSocketConnect(socket, server);//connect to server
	
	while(1)
	{
		if(ethernetIsEstablished(socket) == OK)
		{
			if(dataSent == 0)
			{
				dataSent++;
				if(clientSendCommand(socket, command) == CONNECTION_CLOSE)
				{
					ethernetSocketDisconnect(socket);
				}
			}
			
			if(ethernetCheckIfReceivedData(socket) == OK)
			{
				for(i=0; i<RX_BUFFER_SIZE; i++)		RXbuffer[i] = 0;//clear buffer
				timeout = 0;
				
				length = ethernetSocketReceiveData(socket, RXbuffer);
				
				if(clientProcessReceivedData(socket, RXbuffer, length, &command) == CONNECTION_CLOSE)
				{
					ethernetSocketDisconnect(socket);
				}
				else
				{
					dataSent=0;//ensures new command will be sent in next loop
				}
			}
		}

		if(ethernetCheckIfFINreceived(socket) == OK)
		{
			ethernetSocketDisconnect(socket);
		}

		if(ethernetCheckIfCloseOrTimeout(socket) == OK  || (timeout > WAIT_FOR_DATA_RECEIVE))//try 10000 times to connect, then close socket
		{
			ethernetSocketDisconnect(socket);
			ethernetSocketClose(socket);//close this socket
			return;
		}
		
		timeout++;
		_delay_us(100);//for timeout recognition 
	}
}
