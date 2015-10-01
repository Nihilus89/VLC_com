/*
 * main.c
 *
 *  Created on: Mar 25, 2015
 *      Author: Vassilis
 */


#include "global.h"

int init_FLG = 1;

void sys_init(void)
{
	int i;
	F_CRC_InicializaTabla();
	for(i=0; i<3; i++)
		Preamble[i] = 0x0F;
	SFD = 0xAA;
	length = 0x40;
	length_ACK = 0x00;
	destination = 0x03;
	source = 0x01;
	strncpy(payload, "Now, we can talk to people in Greece from Sweden via vlc boards!", 65);
	gpio_init();
	send_ACK();
}



int main(void)
{
	int receive = 0;
		if(init_FLG)
			sys_init();
	sending();

	while(get_ACK())
	{
		sending();
		usleep(1000);
	}

	printf("\n***************\nSender OUT\n***************\n");
	return 0;

	while(1)
	{
		receive = receiving_mode();

		if(receive == 1)
			printf("Wrong preamble detected\n");
		else if(receive == -1)
		{
			printf("Forwarding message\n");
			sleep(1);
			forward();
			while(get_ACK())
			{
				forward();
				usleep(1000);
			}
		}

		else
			printf("Message for this node received\n");
	}



	printf("END\n");
	return 0;
}
