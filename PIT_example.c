/******************************************************************************/
/* PIT_Example_2017.C: Distance Measurement w/PIT INTERRUPT
Name: Adam Worley, Mashroor Rashid                                            */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2006 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
                  
#include <AT91SAM7SE512.H>              /* AT91SAM7SE512 definitions          */
#include "AT91SAM7SE-EK.h"           /* AT91SAM7SE-EK board definitions    */

#include "Exp1_solution.h"
#include "USART0_solution.h"
#include "TC_Assembly.h" // initialization functions for the timer/counters
#include "Integer_Convert.h"     /* Functions for converting integers to ASCII strings */
#include <stdio.h>

/*********************************************************************
* Constants
*********************************************************************/
#define POS_PULSE_OUT (1<<15)
#define NEG_PULSE_OUT (1<<16)
#define OUTPUT_40K    (1<<1)
#define DUP_OUTPUT    (1<<0)
#define CAPTURE_INPUT (1<<26)

#define TIMER_OUTPUTS (POS_PULSE_OUT|NEG_PULSE_OUT|OUTPUT_40K|DUP_OUTPUT|CAPTURE_INPUT)

#undef  AT91B_LED_MASK
#define AT91B_LED_MASK      (AT91B_LED1|AT91B_LED2)

// 250ms * 47923200Hz / 16 = 748800
#define PIT_MR_VALUE ((748800 << 0) | (1 << 24) | (1 << 25)) // 250ms, interrupts enabled, timer enabled

#define PIOB_PID_BIT (1<<3)

#define TXRDY_BIT (1 << 1)

/*********************************************************************
*	Global Variables
**********************************************************************/
unsigned int print_array_g[50];
unsigned int print_index_g;
unsigned int intervals_g; // tracks the number of PIT intervals
unsigned int num_overflows_g;
/******************************************************************************************
*    Interrupt Service Routines
*******************************************************************************************/
__irq void SPURIOUS_ISR (void)
{
	AT91C_BASE_AIC->AIC_EOICR = 0;
}

__irq void PITC_ISR (void)
{
	
	unsigned int temp = AT91C_BASE_PITC->PITC_PIVR; // clear PITS

	intervals_g++;
	
  if(intervals_g == 2)
    AT91C_BASE_PIOA->PIO_SODR = AT91B_POWERLED; // light on

  else if(intervals_g == 4)
    AT91C_BASE_PIOA->PIO_CODR = AT91B_POWERLED; // light off
	
	else if(intervals_g == 6)
    AT91C_BASE_PIOA->PIO_SODR = AT91B_POWERLED; // light on

  else if(intervals_g == 8)
  {
    AT91C_BASE_PIOA->PIO_CODR = AT91B_POWERLED; // light off
		AT91C_BASE_TC2->TC_IER = (1 << 0)|(1 << 6);
		AT91C_BASE_TCB->TCB_BCR = 1;	// start timers
		num_overflows_g = 0;
    intervals_g = 0; // reset intervals
  }
	
	AT91C_BASE_AIC->AIC_EOICR = 0;
}

__irq void USART0_ISR(void)
{	
	unsigned int status_value = AT91C_BASE_US0->US_CSR;
	unsigned int temp = print_array_g[print_index_g];
	
	if(temp != 0 && (status_value & (1 << 1)))
	{
		AT91C_BASE_US0->US_THR = temp; //write to transmit hold register
		
		print_array_g[print_index_g] = 0; //to clear current index 
		print_index_g++;
	}
	
	else if(temp == 0)
	{
		AT91C_BASE_US0->US_IDR = TXRDY_BIT;
	}
	
	AT91C_BASE_AIC->AIC_EOICR = 0;
}

__irq void TC2_ISR(void)
{
	unsigned int temp;
	unsigned long num_counts;
	float distance;
	unsigned long max_counts = 65536;
	unsigned int distance_int;
	unsigned int distance_frac;
	unsigned int temp_array[50];
	unsigned int index;
	
	temp = AT91C_BASE_TC2->TC_SR & 0xFFFFF;

	if(temp & 1) //check COVFS
	{
		num_overflows_g++;
	}
	
	if((temp & (1 << 6)) && (num_overflows_g <= 20)) //check LDRBS
	{
		//calc distance
		num_counts = (num_overflows_g * max_counts) + AT91C_BASE_TC2->TC_RA;
		distance = (num_counts * (.0000000417334) * 1129.9)/2;
		
		distance_int = (unsigned int)distance;
		distance_frac = (unsigned int)((distance-distance_int)*100);
		Integer_ASCII_unsigned(distance_int,temp_array);
		
		index = 0;
		print_index_g = 0;
	
		while(temp_array[index] != 0)
		{
			print_array_g[print_index_g] = temp_array[index];
			index++;
			print_index_g++;
		}
		
		print_array_g[print_index_g] = '.';
		print_index_g++;
		Integer_ASCII_Fixed_Width(distance_frac,2,temp_array);
		index = 0;
	
		while(temp_array[index] != 0)
		{
			print_array_g[print_index_g] = temp_array[index];
			index++;
			print_index_g++;
		}
		print_array_g[print_index_g] = '\n';
		print_array_g[print_index_g+1] = '\r';
		print_array_g[print_index_g+2] = 0;
		print_index_g = 0;
		
		AT91C_BASE_TC2->TC_IDR = (1 << 0)|(1 << 6);
		AT91C_BASE_US0->US_IER = TXRDY_BIT;
	}
	
	else if(num_overflows_g > 20)
	{
		//stop timers and re enable for next measurement
		
		AT91C_BASE_TC2->TC_CCR = (1 << 1);
		AT91C_BASE_TC2->TC_CCR = (1 << 0);
		
		AT91C_BASE_TC0->TC_CCR = (1 << 1);
		AT91C_BASE_TC0->TC_CCR = (1 << 0);
		
		AT91C_BASE_TC1->TC_CCR = (1 << 1);
		AT91C_BASE_TC1->TC_CCR = (1 << 0);
	
	}
	
	AT91C_BASE_AIC->AIC_EOICR = 0;
}

void AIC_INIT(void)
{
	AT91C_BASE_AIC->AIC_SMR[1] = (7 << 0) | (1<< 5); // highest priority, Positive Edge triggered since internal
	AT91C_BASE_AIC->AIC_SVR[1] = (unsigned int)PITC_ISR; //put address of PIT_ISR into vector reg
	AT91C_BASE_AIC->AIC_SMR[6] = (5 << 0) | (0 << 5); //USART
	AT91C_BASE_AIC->AIC_SVR[6] = (unsigned int)USART0_ISR; 
	AT91C_BASE_AIC->AIC_SMR[14] = (6 << 0) | (0 << 5); //TC2
	AT91C_BASE_AIC->AIC_SVR[14] = (unsigned int)TC2_ISR; 
	
	AT91C_BASE_AIC->AIC_SPU = (unsigned int)SPURIOUS_ISR;
	AT91C_BASE_AIC->AIC_IECR = (1 << 1) | (1 << 6) | (1<<14); //SYSC, PID1 bit, PID6 USART, PID14 TC2
	AT91C_BASE_US0->US_IER = TXRDY_BIT;
	AT91C_BASE_TC2->TC_IER = (1 << 0) | (1 << 6); // Enable timer overflows and LDRBS interrupts
}

/*
 * Main Program
 */

int main (void) 
{
	char test[14] = "Hello World\n\r";
	unsigned int i;
	unsigned int j;
	
	for(i = 0; i < 50; i++) //initialize array to 0's
	{
		print_array_g[i] = 0;
	}
	
	for(j = 0; j < sizeof(test); j++)
	{
		print_array_g[j] = test[j];
	}
	
	print_index_g = 0;

  // Configure the PIO Lines for LEDs as Outputs
	USER_LEDS_INIT();
	POWERLED_INIT();
	
	// Initialize Timers
  AT91C_BASE_PMC->PMC_PCER = PIOB_PID_BIT;

	TC0_INIT(); // Initialize TC0 to create 40KHz waveform
  TC1_INIT(); // Initialize TC1 to create 101us active high pulse and 1ms active low pulse
	TC2_INIT(); // Initialize TC2 in capture mode	
	
  AT91C_BASE_TCB->TCB_BMR = (2); // Connect TIOA1 to XC0  
	
	
	// Initialize the Advanced Interrupt Controller
	AIC_INIT();

	// Initialize IO for powerLED control and switch input
	SWITCH_INIT();
	USART0_INIT();
	
	// Initialize the Periodic Interval Timer
	AT91C_BASE_PITC->PITC_PIMR = PIT_MR_VALUE;
	
  // Loop forever
  for (;;) 
	{
		
			
	}
}
