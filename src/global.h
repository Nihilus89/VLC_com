/*
 * global.h
 *
 *  Created on: Mar 23, 2015
 *      Author: Administrator
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

/*
 ***************************************************

					Includes

 **************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "mraa/gpio.h"
#include "mraa/aio.h"
#include "crc.h"
#include "timer.h"

/*
 ***************************************************

					Macros

 **************************************************
 */

#define THR 50	// Threshold to distinct 1 from 0

// Frame macros
#define BYTES 73
#define MSGSIZE BYTES*16
#define nSIZE BYTES+1
#define ACKSIZE 5*16

#define SENDACK 	blink_ACK()

/*
 ***************************************************

					Variables

 **************************************************
 */

char signal_buffer[MSGSIZE];
char buffer_s[MSGSIZE];
int readings[MSGSIZE];
int msg_binary_buffer[MSGSIZE/2];
char message[nSIZE];

mraa_gpio_context gpio0, gpio1, gpio2, gpio3;	// Blinking pins
mraa_aio_context A0, A1, A2, A3, R;		// Photodiodes


int var;

// Message
//char Preamble[3] = {0x0F, 0x0F, 0x0F};
//char SFD = 0xAA;
//char length = 0x40;
//char destination = 0x02;
//char source = 0x01;
//char payload[65] = "Now, we can talk to people in Greece from Sweden via vlc boards!";
char Preamble[3];
char SFD;
char length;
char destination;
char source;
char payload[65];
char* CRC_Result;

// ACK
char length_ACK;


char buffer_ACK_W[ACKSIZE];
int readings_ACK[ACKSIZE];
int message_ACK[ACKSIZE/2];
char buffer_ACK[ACKSIZE];


int mod_counter, ack_counter;


/*
 ***************************************************

			Function prototypes

 **************************************************
 */
void sys_init(void);
void gpio_init(void);
void modulation(char c);
int arr_to_dec(int start);
void calibrate(mraa_aio_context diode, int n);
int standby(mraa_aio_context D[4]);
void demod(int x);
int printMSG(void);
void getInfo(void);
int CRC_check(int n);
int receiving_mode(void);
void blink(void);
void send_ACK(void);
void modulation_ACK(char c);
void blink_ACK(void);
int standby_ACK(mraa_aio_context D[4]);
void getInfo_ACK(void);
void demod_ACK(int x);
int arr_to_dec_ACK(int start);
int get_ACK();
void sending(void);
void forward(void);

// Timers
void timer_handler(void);
void timer_handler_r(void);

/*
 ***************************************************

			Function bodies

 **************************************************
 */
#endif /* GLOBAL_H_ */
