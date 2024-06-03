/*******************************************************************************
* Copyright © 2018 TRINAMIC Motion Control GmbH & Co. KG
* (now owned by Analog Devices Inc.),
*
* Copyright © 2024 Analog Devices Inc. All Rights Reserved. This software is
* proprietary & confidential to Analog Devices, Inc. and its licensors.
*******************************************************************************/

//  Created on: 15.02.2018	Author: ED

#include "SysTick.h"

unsigned int msCounter = 0;

/* timer_0 interrupt handler */
ISR(TIM1_COMPA_vect)
{
	msCounter++;
}

/* init timer_0 */
void systick_init(void)
{
	// configure Timer_0
	TCCR1 |= (1<<CS01); 	// prescaler 8
	TCCR1 |= (1<<WGM01);	// use CTC Modus
	OCR1A = 150-1;   		// (0,25ms interrupt)

	// enable compare interrupt
	TIMSK |= (1<<OCIE1A);
	msCounter = 0;
}

unsigned int systick_getTimer(void)
{
	return msCounter;
}
