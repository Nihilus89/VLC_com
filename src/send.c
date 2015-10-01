/*
 * send.c
 *
 *  Created on: Mar 24, 2015
 *      Author: Vassilis
 */
#include "global.h"

int mod_counter = 0;

int ini_FLG = 1;

void timer_handler(void)
{
	var++;
}



void sending(void)
{
	int i;
	if(ini_FLG)
	{
		for(i=0; i<3; i++)
			modulation(Preamble[i]);
		modulation(SFD);
		modulation(length);
		modulation(destination);
		modulation(source);
		for(i=0; i<64; i++)
			modulation(payload[i]);
		CRC_Result = malloc(2*sizeof(char));
		crc CRC_A;
		CRC_A = F_CRC_CalculaCheckSum((unsigned char *)payload, length - 1);
		*(unsigned short*)CRC_Result = CRC_A;
		printf("CRC result:0x%X\n", CRC_A);
		for(i=0; i<2; i++)
			modulation(CRC_Result[i]);
		ini_FLG = 0;
	}
	blink();
	//SENDACK;
}

void modulation(char c)
{
        unsigned short i;

         for(i=0;i<8;i++)
     	{
        	 if ( c  &  0x80)
	  		 	 {buffer_s[mod_counter] = 0; buffer_s[mod_counter + 1] = 1;}
        	 else
	  		 	 {buffer_s[mod_counter] = 1; buffer_s[mod_counter + 1] = 0;}
        	 c <<= 1;
        	 mod_counter += 2;
     	}
}

void blink(void)
{
	unsigned short j;

    start_timer(10, &timer_handler);
    var = 0;
    for( j=0 ; j<=mod_counter-1 ; j++ )                  // light the LED !!
    {
    	mraa_gpio_write(gpio0, buffer_s[j]);
    	mraa_gpio_write(gpio1, buffer_s[j]);
        mraa_gpio_write(gpio2, buffer_s[j]);
        mraa_gpio_write(gpio3, buffer_s[j]);
        while(var<2)
        	usleep(1);
        var = 0;
    }
    stop_timer();
    mraa_gpio_write(gpio0, 0);
	mraa_gpio_write(gpio1, 0);
	mraa_gpio_write(gpio2, 0);
	mraa_gpio_write(gpio3, 0);
}

void send_ACK(void)
{
	int i;
	for(i=0; i<3; i++)
		modulation_ACK(Preamble[i]);
	modulation_ACK(SFD);
	modulation_ACK(length_ACK);
}

void modulation_ACK(char c)
{
    unsigned short i;

     for(i=0;i<8;i++)
 	{
    	 if ( c  &  0x80)
  		 	 {buffer_ACK[ack_counter] = 0; buffer_ACK[ack_counter + 1] = 1;}
    	 else
  		 	 {buffer_ACK[ack_counter] = 1; buffer_ACK[ack_counter + 1] = 0;}
    	 c <<= 1;
    	 ack_counter += 2;
 	}
}

void blink_ACK(void)
{
	unsigned short j;

    start_timer(10, &timer_handler);
    var = 0;
    for( j=0 ; j<=ack_counter-1 ; j++ )                  // light the LED !!
    {
    	mraa_gpio_write(gpio0, buffer_ACK[j]);
    	mraa_gpio_write(gpio1, buffer_ACK[j]);
        mraa_gpio_write(gpio2, buffer_ACK[j]);
        mraa_gpio_write(gpio3, buffer_ACK[j]);
        while(var<2)
        	usleep(1);
        var = 0;
    }
    stop_timer();
    mraa_gpio_write(gpio0, 0);
	mraa_gpio_write(gpio1, 0);
	mraa_gpio_write(gpio2, 0);
	mraa_gpio_write(gpio3, 0);
}

void forward(void)
{
	unsigned short j;

    start_timer(10, &timer_handler);
    var = 0;
    for( j=0 ; j<=1168-1 ; j++ )                  // light the LED !!
    {
    	mraa_gpio_write(gpio0, signal_buffer[j]);
    	mraa_gpio_write(gpio1, signal_buffer[j]);
        mraa_gpio_write(gpio2, signal_buffer[j]);
        mraa_gpio_write(gpio3, signal_buffer[j]);
        while(var<2)
        	usleep(1);
        var = 0;
    }
    stop_timer();
    mraa_gpio_write(gpio0, 0);
	mraa_gpio_write(gpio1, 0);
	mraa_gpio_write(gpio2, 0);
	mraa_gpio_write(gpio3, 0);
}


void gpio_init(void)
{
	   gpio3 = mraa_gpio_init(13);
	   gpio2 = mraa_gpio_init(12);
	   gpio1 = mraa_gpio_init(11);
	   gpio0 = mraa_gpio_init(10);

	   mraa_gpio_dir(gpio0, MRAA_GPIO_OUT);
	   mraa_gpio_dir(gpio1, MRAA_GPIO_OUT);
	   mraa_gpio_dir(gpio2, MRAA_GPIO_OUT);
	   mraa_gpio_dir(gpio3, MRAA_GPIO_OUT);
}

