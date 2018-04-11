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

#ifndef W5500_H_
#define W5500_H_

#define F_CPU 32000000UL

#define FAIL				0
#define NO					FAIL
#define OK					1
#define YES					OK


typedef struct structure1
{
	unsigned char b0;
	unsigned char b1;
	unsigned char b2;
	unsigned char b3;
	unsigned int socketPort;
}IPaddressAndPort;

typedef struct structure2
{
	unsigned char b0;
	unsigned char b1;
	unsigned char b2;
	unsigned char b3;
}address;

typedef struct structure5
{
	unsigned char b0;
	unsigned char b1;
	unsigned char b2;
	unsigned char b3;
	unsigned char b4;
	unsigned char b5;
}MACaddress;




//SOURCE IP ADDRESS
#define IP0		192
#define IP1		168
#define IP2		1
#define IP3		4

//SUBNET MASK ADDRESS
#define MASK0	255
#define MASK1	255
#define MASK2	255
#define MASK3	0

//GATEWAY ADDRESS
#define GW0		192
#define GW1		168
#define GW2		1
#define GW3		1

//MAC ADDRESS
#define MAC0	0x00
#define MAC1	0x08
#define MAC2	0xDC
#define MAC3	0x0D
#define MAC4	0x42
#define MAC5	0xEA


#define RX_BUFFER_SIZE	1024UL //in bytes
#define WAIT_FOR_DATA_RECEIVE	10000	//how long is the connection open before timeout occurs (cca in milliseconds) //max 65535

#define CONNECTION_KEEP_ALIVE	0x16
#define CONNECTION_CLOSE		0x17

#define CALCULATE_LENGTH		0xFFFF





// Wiznet W5500 Register Addresses COMMON REGISTER
#define MR				0x0000   // Mode Register
#define GAR				0x0001   // Gateway Address: 0x0001 to 0x0004
#define SUBR			0x0005   // Subnet mask Address: 0x0005 to 0x0008
#define SHAR			0x0009   // Source Hardware Address (MAC): 0x0009 to 0x000E
#define SIPR			0x000F   // Source IP Address: 0x000F to 0x0012
#define RMSR			0x001A   // RX Memory Size Register
#define TMSR			0x001B   // TX Memory Size Register


// Wiznet W5500 Register Addresses SOCKET REGISTER
#define Sn_RX_WR		0
#define Sn_TX_RD		0
#define Sn_MR			0x0000 //socket n mode register r/w 0x0000, 0x00
#define Sn_CR			0x0001 //socket n command register r/w 0x0001, 0x00
#define Sn_SR			0x0003 //socket n status register
#define Sn_PORT0		0x0004
#define Sn_PORT1		0x0005
#define Sn_DIPR0		0x000C
#define Sn_DPORT0		0x0010
#define Sn_DPORT1		0x0011


// Sn_PORT
// commands for SOCKET REGISTER
#define Sn_MR_TCP		0b00000001 // TCP mode
#define Sn_CR_OPEN		0x01 // open port, p69
#define Sn_CR_LISTEN	0x02
#define Sn_CR_CONNECT	0x04
#define Sn_CR_DISCON	0x08
#define Sn_CR_CLOSE		0x10


#define Sn_RECV			0x40// RECV command
#define Sn_SEND			0x20// SEND command

// pointers and memory
#define Sn_TX_FSR_H		0x0020 //(Socket n TX Free Size Register)[R][0x0800]
#define Sn_TX_FSR_L		0x0021 //(Socket n TX Free Size Register)[R][0x0800]
#define Sn_TX_RD_H		0x0022 //(Socket n TX Read Pointer Register)[R][0x0000]
#define Sn_TX_RD_L		0x0023 //(Socket n TX Read Pointer Register)[R][0x0000]
#define Sn_TX_WR_H		0x0024 //(Socket n TX Write Pointer Register)[R/W][0x0000]
#define Sn_TX_WR_L		0x0025 //(Socket n TX Write Pointer Register)[R/W][0x0000]

#define Sn_RX_RSR_H		0x0026 //(Socket n RX Received Size Register)[R][0x0000]
#define Sn_RX_RSR_L		0x0027 //(Socket n RX Received Size Register)[R][0x0000]
#define Sn_RX_RD_H		0x0028 //(Socket n RX Read Data Pointer Register)[R/W][0x0000]
#define Sn_RX_RD_L		0x0029 //(Socket n RX Read Data Pointer Register)[R/W][0x0000]
#define Sn_RX_WR_H		0x002A //(Socket n RX Write Pointer Register)[R][0x0000]
#define Sn_RX_WR_L		0x002B //(Socket n RX Write Pointer Register)[R][0x0000]

#define SOC0_REG		0b00001
#define SOC1_REG		0b00101
#define SOC2_REG		0b01001
#define SOC3_REG		0b01101
#define SOC4_REG		0b10001
#define SOC5_REG		0b10101
#define SOC6_REG		0b11001
#define SOC7_REG		0b11101

#define SOCK_CLOSED			0x00
#define SOCK_INIT			0x13
#define SOCK_LISTEN			0x14
#define SOCK_ESTABLISHED	0x17
#define SOCK_CLOSE_WAIT		0x1C
#define SOCK_UDP			0x22
#define SOCK_MACRAW			0x42
#define SOCK_SYNSENT		0x15
#define SOCK_SYNRECV		0x16
#define SOCK_FIN_WAIT		0x18
#define SOCK_CLOSING		0x1A
#define SOCK_TIME_WAIT		0x1B
#define SOCK_LAST_ACK		0x1D
#define SOCK_DOH			0xEF
#define SOCK_MESSUP			0xFF


//Public prototypes

void ethernetInit(address IPaddress, address mask, address gateway, MACaddress MACadr);//set IP, Mask, Gateway and MAC address

//TCP server and client

void TCPserver(unsigned char socket, unsigned int socketPort);
void TCPclient(unsigned char socket, unsigned int sourceSocketPort, IPaddressAndPort server, unsigned long command);

#endif /* W5500_H_ */