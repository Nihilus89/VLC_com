/*
 * receiver.c
 *
 *  Created on: Mar 18, 2015
 *      Author: Administrator
 */

#include "global.h"

//int message[MSGSIZE/2] = {0};
//int readings[MSGSIZE] = {0};
//int buffer[MSGSIZE] = {0};
//char bytes[nSIZE] = {0};

int start_ACK = 1, check_ACK = 0;

int pr[24] = {0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1};

int check = 0;

int myAddr = 1;
//int myAddr = 2;
//int myAddr = 3;

// Calibration values
int cal[4];
// Init flag
int start = 1;


// Timer counter
int var_r=0;



// Frame attributes
int lth, dst;

void timer_handler_r(void)
{
	var_r++;
}


int receiving_mode()   // recciving_mode()
{
	int i, n=0;

	A0 = mraa_aio_init(0);
	A1 = mraa_aio_init(1);
	A2 = mraa_aio_init(2);
	A3 = mraa_aio_init(3);
	mraa_aio_context A[4] = {A0, A1, A2, A3};

	if(start)	// only run once
	{
		for(i=0; i<4; i++)
			calibrate(A[i], i);
		ack_counter = 0;
		length_ACK = 0;
		start = 0;
		var_r = 0;
	}

	//Enter stand-by mode, ready to receive
	printf("Standy-by status\n");
	if(start_timer(10, &timer_handler_r))	// start the timer
		printf("\n timer error\n");
	n = standby(A); // wait here
	printf("\n ***n = %d ***\n", n);

	// // Here we enter the receiving mode. "n" is the diode where the incoming signal was detected
	printf("Receiving...\n");
	var_r = 0;	// reset the timer's counter

    for (i = 0; i < MSGSIZE; i++)
    {
    	readings[i] = mraa_aio_read(A[n]);
        while(var_r<2)	// wait for 20 (2*10) msec
        	usleep(1);	// relieve the CPU
        var_r = 0;		// reset the counter for the next iteration
    }
    stop_timer();		// all bits have been received, the timer can stop

    printf("Transmission ended\n\n");

    // Data processing mode
    demod(n); 	// process what's 1 and what's 0 from the readings using the ambient threshold
    getInfo();	// get the frames attributes

    if (check == 0 || check == -1){
		// Print the message
		int a;	// return value of the print message routine. It points where the message starts, so where to look for the CRC
		a = printMSG();

		if(CRC_check(a))	// check the CRC generated and received and print the appropriate message
		{
			printf("The message was received successfully\n");
			sleep(1);
			SENDACK;
		}

		else
		{
			check = 1 ;
			printf("The message was received with errors\n");
		}
    }

    // Exit
    return check;

}


/*-----------------------------------------------------------------------------------------------------------------------*/

void calibrate(mraa_aio_context diode, int n)
{
	int i, sum = 0;
	for(i=0; i<10; i++)
	{
		sum += mraa_aio_read(diode);
		usleep(1*1000);
	}
	cal[n] = sum/i;
	printf("Ambient average A%d: %d\n",n, cal[n]);
}

/*-----------------------------------------------------------------------------------------------------------------------*/

int standby(mraa_aio_context D[4])
{
	int i, temp[4], max = 0, ret = 0;
	do
	{
		var_r = 0;
		for(i=0; i<4; i++)
		{
			temp[i] = mraa_aio_read(D[i]);
			if(temp[i]>max)
			{
				max = temp[i];
				ret = i;
			}
		}

		}

	while(max < cal[ret] + THR);
	return ret;
}

/*-----------------------------------------------------------------------------------------------------------------------*/

void getInfo(void)
{
	int i, m_source, m_length, m_destination;
	printf("Preamble is: ");
	for(i=0; i<24; i++)
	{
		printf("%d", msg_binary_buffer[i]);
	}
	printf("\n");

	// checking the preamble
	i = 0;
	check = 0;
	do{
		if (msg_binary_buffer[i] != pr[i])
			check = 1;
		i++;
	}while( (check == 0) && (i < 24) );

	printf("SFD is: ");
	for(i=24; i<32; i++)
	{
		printf("%d", msg_binary_buffer[i]);
	}
	printf("\n");

	m_length = arr_to_dec(32);
	printf("Length is: %d\n",m_length);

	m_destination = arr_to_dec(40);
	printf("Destination is: %d\n",m_destination);

	if ((m_destination != myAddr) && (check == 0)){
		check = -1;
	}

	printf("My ID is %d\nThe destination ID is %d\n", myAddr,m_destination);

	m_source = arr_to_dec(48);
	printf("Source is: %d\n",m_source);
}

/*-----------------------------------------------------------------------------------------------------------------------*/

void demod(int x)
{
    int j = 0, i;
    for (i = 0; i < MSGSIZE; i++)
    {
    	if(readings[i]>(cal[x] + THR))
    		signal_buffer[i] = 1;
		else
			signal_buffer[i] = 0;
    }

    for (i = 0; i < MSGSIZE; i = i + 2)
    {
    	if(signal_buffer[i] == 1 )
    		msg_binary_buffer[j] = 0;
    	else
    		msg_binary_buffer[j] = 1;
    	j++;
    }
}

/*-----------------------------------------------------------------------------------------------------------------------*/

int printMSG(void)
{
	int i, n, size;

	size = sizeof(msg_binary_buffer)/sizeof(int);
	n = (size / 8) - 2; // minus the CRC
	for(i = 7; i < n; i++)
	{
		message[i-7] = arr_to_dec(8*i);
		//printf("Byte %d in ASCII: %d\n", i-7 + 1,message[i-7]);
	}
	printf("\nMessage: %s\n", message);
	return i;
}

/*-----------------------------------------------------------------------------------------------------------------------*/

int arr_to_dec(int start)
{
	int bitvalue = 128, total = 0, i = 0;
	for (i = start; i < start+8; i++)
	{
	  if (msg_binary_buffer[i])
		total += bitvalue;
	  bitvalue /= 2;
	}
	return total;
}

/*-----------------------------------------------------------------------------------------------------------------------*/

int CRC_check(int n)
{
	crc rec_code = 0, CRC_Result = 0, a, b;
	unsigned long temp; // intermediate var_r_riable type should be bigger to handle the shift
	int retval = 0;

	b = arr_to_dec(8*n);
	a = arr_to_dec(8*(n+1));
	temp = ((a<<8) | ((b)));
	rec_code = (crc) temp;
	printf("Received CRC: 0x%X\n", rec_code);


	F_CRC_InicializaTabla();
	CRC_Result = F_CRC_CalculaCheckSum((unsigned char *) message, strlen(message)-1);
	printf("Generated CRC: 0x%X\n", CRC_Result);

	if(rec_code == CRC_Result)
		retval = 1;
	return retval;
}



int get_ACK()   // receiving_mode()
{
	int i, n=0;

	A0 = mraa_aio_init(0);
	A1 = mraa_aio_init(1);
	A2 = mraa_aio_init(2);
	A3 = mraa_aio_init(3);
	mraa_aio_context A[4] = {A0, A1, A2, A3};

	if(start_ACK)	// only run once
	{
		for(i=0; i<4; i++)
			calibrate(A[i], i);
		start_ACK = 0;
	}

	//Enter stand-by mode, ready to receive
	printf("Waiting for ACK\n");
	if(start_timer(10, &timer_handler_r))	// start the timer
		printf("\n timer error\n");
	n = standby_ACK(A); // wait here
	if(n == -1)
	{
		stop_timer();
		return 1;
	}


	// // Here we enter the receiving mode. "n" is the diode where the incoming signal was detected
	printf("Getting ACK...\n");
	var_r = 0;	// reset the timer's counter

    for (i = 0; i < ACKSIZE; i++)
    {
    	readings_ACK[i] = mraa_aio_read(A[n]);
        while(var_r<2)	// wait for 20 (2*10) msec
        	usleep(1);	// relieve the CPU
        var_r = 0;		// reset the counter for the next iteration
    }
    stop_timer();		// all bits have been received, the timer can stop

    printf("ACK ended\n\n");

    // Data processing mode
    demod_ACK(n); 	// process what's 1 and what's 0 from the readings using the ambient threshold
    getInfo_ACK();	// get the frames attributes

    if (check_ACK == 0)
    	printf("The ACK was received successfully\n");
    else
		printf("The ACK was received with errors\n");

    // Exit
//    return check_ACK;
    return 0;

}


int standby_ACK(mraa_aio_context D[4])
{
	int i, temp[4], max = 0, ret = 0;
	var_r = 0;
	do
	{
		//printf("var_r = %d\n",var_r);
		if(var_r >= 1000)
			break;
		for(i=0; i<4; i++)
		{
			temp[i] = mraa_aio_read(D[i]);
			if(temp[i]>max)
			{
				max = temp[i];
				ret = i;
			}
		}

		}

	while(max < cal[ret] + THR);
	if(var_r >= 1000)
		ret = -1;
	return ret;
}


/*-----------------------------------------------------------------------------------------------------------------------*/

void getInfo_ACK(void)
{
	int i, a_lenght;
	printf("\n*******\nACK:\n*******\n");
	printf("Preamble is: ");
	for(i=0; i<24; i++)
	{
		printf("%d", message_ACK[i]);
	}
	printf("\n");

	// checking the preamble
	i = 0;
	check_ACK = 0;
	do{
		if (message_ACK[i] != pr[i])
			check_ACK = 1;
		i++;
	}while( (check_ACK == 0) && (i < 24) );

	printf("SFD is: ");
	for(i=24; i<32; i++)
	{
		printf("%d", message_ACK[i]);
	}
	printf("\n");

	a_lenght = arr_to_dec(32);
	printf("length_ACK is: %d\n",a_lenght);

	if(a_lenght)
		check_ACK = 1;
}

/*-----------------------------------------------------------------------------------------------------------------------*/

void demod_ACK(int x)
{
    int j = 0, i;
    for (i = 0; i < ACKSIZE; i++)
    {
    	if(readings_ACK[i]>(cal[x] + THR))
    		buffer_ACK_W[i] = 1;
		else
			buffer_ACK_W[i] = 0;
    }

    for (i = 1; i < ACKSIZE; i = i + 2)
    {
    	if(buffer_ACK_W[i] == 1 )
    		message_ACK[j] = 0;
    	else
    		message_ACK[j] = 1;
    	j++;
    }
}

/*-----------------------------------------------------------------------------------------------------------------------*/

int arr_to_dec_ACK(int start)
{
	int bitvalue = 128, total = 0, i = 0;
	for (i = start; i < start+8; i++)
	{
	  if (message_ACK[i])
		total += bitvalue;
	  bitvalue /= 2;
	}
	return total;
}
