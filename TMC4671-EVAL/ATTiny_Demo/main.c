/*******************************************************************************
* Copyright © 2018 TRINAMIC Motion Control GmbH & Co. KG
* (now owned by Analog Devices Inc.),
*
* Copyright © 2024 Analog Devices Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

//  Created on: 15.02.2018	Author: ED

/*
 * TMC4671-Wheel-Wheel-Demo
 *
 */

#include "Definitions.h"
#include "SysTick.h"

#define MOTOR_BOTTOM 	0
#define MOTOR_TOP 		1

// Macros for IO access
#define CS_MOTOR_BOTTOM_LOW()	 	PORTB &= ~(1<<PB3)
#define CS_MOTOR_BOTTOM_HIGH() 		PORTB |= (1<<PB3)
#define CS_MOTOR_TOP_LOW() 			PORTB &= ~(1<<PB4)
#define CS_MOTOR_TOP_HIGH()		 	PORTB |= (1<<PB4)
#define SCK_LOW() 					PORTB &= ~(1<<PB2)
#define SCK_HIGH()					PORTB |= (1<<PB2)
#define MOSI_LOW()					PORTB &= ~(1<<PB0)
#define MOSI_HIGH()					PORTB |= (1<<PB0)
#define MISO  						(PINB & (1<<PB1))

#define MOTOR_BOTTOM_ONLY
//#define N_POSITION_TEST

// PI parameter
#define TORQUE_PI 					(uint32_t)0x13881F40
#define VELOCITY_PI					(uint32_t)0x251005E8//0x271000C8
#define POSITION_PI					(uint32_t)0x01400000

// limits
#define LIMIT_FLUX_TORQUE			(uint16_t)3000//4000
#define LIMIT_UQ_UD					(uint16_t)32767
#define LIMIT_VELOCITY				(uint16_t)100//1300

// encoder/position initialization
#define ENCODER_INIT_VOLTAGE 			(uint16_t)6000
#define Motor_BOTTOM_Position_Offset 	(uint16_t)44830//63600
#define Motor_TOP_Position_Offset 		(uint16_t)50400//59450

unsigned char spi_readWriteByte(unsigned char data)
{
	// mask for bit position
	unsigned char nMask = 0x80;
	unsigned char reply = 0;
	for (int k=0; k<8; k++)
	{
		SCK_LOW();

		if (data & nMask)
			MOSI_HIGH();
		else
			MOSI_LOW();

		if (MISO)
		{
			reply |= nMask;
		}
		SCK_HIGH();

		// update bit position
		nMask >>= 1;
	}
	return reply;
}

void spi_writeInt(uint8_t motor, uint8_t address, int32_t value)
{
	SCK_HIGH();

	// select motor
	if (motor == MOTOR_BOTTOM)
		CS_MOTOR_BOTTOM_LOW();
	else if (motor == MOTOR_TOP)
		CS_MOTOR_TOP_LOW();

	// write address
	spi_readWriteByte(address|0x80);

	// write value
 	uint8_t buf[4];
 	buf[0] = value & 0xFF;
 	value >>= 8;
 	buf[1] = value & 0xFF;
 	value >>= 8;
 	buf[2] = value & 0xFF;
 	value >>= 8;
 	buf[3] = value & 0xFF;

	spi_readWriteByte(buf[3]);
	spi_readWriteByte(buf[2]);
	spi_readWriteByte(buf[1]);
	spi_readWriteByte(buf[0]);

	// deselect motor
	if (motor == MOTOR_BOTTOM)
		CS_MOTOR_BOTTOM_HIGH();
	else if (motor == MOTOR_TOP)
		CS_MOTOR_TOP_HIGH();
}

uint32_t spi_readInt(uint8_t motor, uint8_t address)
{
	SCK_HIGH();

	// select motor
	if (motor == MOTOR_BOTTOM)
		CS_MOTOR_BOTTOM_LOW();
	else if (motor == MOTOR_TOP)
		CS_MOTOR_TOP_LOW();

	// select address with cleared write bit
	spi_readWriteByte(address & 0x7F);

	uint32_t value = 0;
 	value = spi_readWriteByte(0);
 	value = value << 8;
 	value |= spi_readWriteByte(0);
 	value = value << 8;
 	value |= spi_readWriteByte(0);
 	value = value << 8;
 	value |= spi_readWriteByte(0);

	// deselect motor
	if (motor == MOTOR_BOTTOM)
		CS_MOTOR_BOTTOM_HIGH();
	else if (motor == MOTOR_TOP)
		CS_MOTOR_TOP_HIGH();

	return value;
}

void wait(uint16_t time)
{
	uint32_t startTime = systick_getTimer();
	while ((systick_getTimer()-startTime) < time){;}
}

void configureMotor(uint8_t motor)
{
	// Type of motor &  PWM configuration
	spi_writeInt(motor, TMC4671_MOTOR_TYPE_N_POLE_PAIRS, 0x00020032);
	spi_writeInt(motor, TMC4671_PWM_POLARITIES, 0x00000000);
	spi_writeInt(motor, TMC4671_PWM_SV_CHOP, 0x00000007);
	spi_writeInt(motor, TMC4671_PWM_MAXCNT, 0x00000F9F);
	spi_writeInt(motor, TMC4671_PWM_BBM_H_BBM_L, 0x00000808);

	// ADC configuration
	spi_writeInt(motor, TMC4671_dsADC_MCFG_B_MCFG_A, 0x00100010);
	spi_writeInt(motor, TMC4671_dsADC_MCLK_A, 0x20000000);
	spi_writeInt(motor, TMC4671_dsADC_MCLK_B, 0x00000000);
	spi_writeInt(motor, TMC4671_dsADC_MDEC_B_MDEC_A, 0x00430040); // Decimation configuration register.

	//ADC scale & offset
	spi_writeInt(motor, TMC4671_ADC_I0_SCALE_OFFSET, 0x010082AC);
	spi_writeInt(motor, TMC4671_ADC_I1_SCALE_OFFSET, 0x01008286);
	spi_writeInt(motor, TMC4671_ADC_I_SELECT, 0x18000100);

	//Encoder configuration
	spi_writeInt(motor, TMC4671_ABN_DECODER_MODE, 0x00000000); // Control bits how to handle ABN decoder signals.
	spi_writeInt(motor, TMC4671_ABN_DECODER_PPR, 0x00008000);
	spi_writeInt(motor, TMC4671_ABN_DECODER_COUNT_N, 0x00000000);
	spi_writeInt(motor, TMC4671_ABN_DECODER_PHI_E_PHI_M_OFFSET, 0x00000000);

	// selections
	spi_writeInt(motor, TMC4671_VELOCITY_SELECTION, 9);  // phi_m_abn
	spi_writeInt(motor, TMC4671_POSITION_SELECTION, 9);  // phi_m_abn

	// set limits
	spi_writeInt(motor, TMC4671_PIDOUT_UQ_UD_LIMITS, LIMIT_UQ_UD);  // 23767
	spi_writeInt(motor, TMC4671_PID_TORQUE_FLUX_LIMITS, LIMIT_FLUX_TORQUE);
	spi_writeInt(motor, TMC4671_PID_VELOCITY_LIMIT, LIMIT_VELOCITY);

	// set PI parameter
	spi_writeInt(motor, TMC4671_PID_FLUX_P_FLUX_I, TORQUE_PI); 				// P and I parameter for the flux regulator.
	spi_writeInt(motor, TMC4671_PID_TORQUE_P_TORQUE_I, TORQUE_PI); 			// P and I parameter for the torque regulator.
	spi_writeInt(motor, TMC4671_PID_VELOCITY_P_VELOCITY_I, VELOCITY_PI); 	// P and I parameter for the velocity regulator.
	spi_writeInt(motor, TMC4671_PID_POSITION_P_POSITION_I, POSITION_PI); 	// P parameter for the position regulator.
}

void encoderInit(uint8_t motor)
{
	// do Encoder initialization (Mode 0)
	spi_writeInt(motor, TMC4671_MODE_RAMP_MODE_MOTION, 0x00000008);	 	// use UQ_UD_EXT for motion
	spi_writeInt(motor, TMC4671_ABN_DECODER_PHI_E_PHI_M_OFFSET, 0);	 	// set ABN_DECODER_PHI_E_OFFSET to zero
	spi_writeInt(motor, TMC4671_PHI_E_SELECTION, 1);					// select phi_e_ext
	spi_writeInt(motor, TMC4671_PHI_E_EXT, 0);							// set the "zero" angle
	spi_writeInt(motor, TMC4671_UQ_UD_EXT, ENCODER_INIT_VOLTAGE);		// set an initialization voltage on UD_EXT (to the flux, not the torque!)
	spi_writeInt(motor, TMC4671_PID_POSITION_ACTUAL, 0);				// critical: needed to set ABN_DECODER_COUNT to zero

	uint32_t reply;

	// clear DECODER_COUNT and check
	do
	{
		// set internal encoder value to zero
		spi_writeInt(motor, TMC4671_ABN_DECODER_COUNT, 0);
		reply = spi_readInt(motor, TMC4671_ABN_DECODER_COUNT);
	} while (reply != 0);

	// use encoder
	spi_writeInt(motor, TMC4671_PHI_E_SELECTION, 0x00000003); 			// select phi_abn for rotor position angle phi_e

	// switch to velocity mode
	spi_writeInt(motor, TMC4671_MODE_RAMP_MODE_MOTION, 0x00000002);
	spi_writeInt(motor, TMC4671_PID_VELOCITY_TARGET, 10);

	// and search the N-Channel to clear the actual position
	spi_writeInt(motor, TMC4671_ABN_DECODER_PHI_E_PHI_M, 0);
	spi_writeInt(motor, TMC4671_ABN_DECODER_MODE, 0);

	spi_writeInt(motor, TMC4671_PID_VELOCITY_TARGET, 20); // test

	// clear DECODER_COUNT_N and check
	do
	{
		spi_writeInt(motor, TMC4671_ABN_DECODER_COUNT_N, 0);
		reply = spi_readInt(motor, TMC4671_ABN_DECODER_COUNT_N);
	} while (reply != 0);

	spi_writeInt(motor, TMC4671_PID_VELOCITY_TARGET, 30); // test

	// search N-channel
	do
	{
		reply = spi_readInt(motor, TMC4671_ABN_DECODER_COUNT_N);
		spi_writeInt(motor, TMC4671_PID_POSITION_ACTUAL, 0);
	} while (reply == 0);

	spi_writeInt(motor, TMC4671_PID_VELOCITY_TARGET, 40); // test

	// drive to zero position using position mode
	spi_writeInt(motor, TMC4671_PID_POSITION_TARGET, (motor==0) ? Motor_BOTTOM_Position_Offset : Motor_TOP_Position_Offset);
	spi_writeInt(motor, TMC4671_MODE_RAMP_MODE_MOTION, 3);
}

uint8_t positionReached(int32_t targetPosition, int32_t actualPosition, int32_t actualVelocity, int32_t maxPosDiff, int32_t maxVel)
{
	if ((labs((long)targetPosition-(long)actualPosition) <= maxPosDiff) & (labs((long)actualVelocity) <= maxVel))
	{
		return 1;
	}
	return 0;
}

int main (void)
{
	// initialize port pins (pins PB0..PB5 are available)
	// PB0 - MOSI (output)
	// PB1 - MISO (input)
	// PB2 - SCK (output)
	// PB3 - /CS_TMC4671_MOTOR_0 (output)
	// PB4 - /CS_TMC4671_MOTOR_1 (output)
	// PB5 - Alive LED
	DDRB = (1<<DDB0) | (1<<DDB2) | (1<<DDB3) | (1<<DDB4);

	// init chip select
	CS_MOTOR_BOTTOM_HIGH();
	CS_MOTOR_TOP_HIGH();

	// configure timer0
	systick_init();

	//enable interrupts
	sei();

	wait(1000);

	// initialize motor at bottom
	spi_readInt(MOTOR_BOTTOM, TMC4671_CHIPINFO_DATA);
	configureMotor(MOTOR_BOTTOM);
	encoderInit(MOTOR_BOTTOM);

	wait(4000);

	// initialize motor at top
	spi_readInt(MOTOR_TOP, TMC4671_CHIPINFO_DATA);
	configureMotor(MOTOR_TOP);
	encoderInit(MOTOR_TOP);

	wait(1000);

	// main (infinite) loop
	int positionCounter = 0;
	int state = 0;
	int32_t lastTargetPositionBottom = Motor_BOTTOM_Position_Offset;
	int32_t lastTargetPositionTOP = Motor_TOP_Position_Offset;
	int32_t maxDiff = 100;
	int32_t maxVel = 20;
	while(1)
	{
#ifndef N_POSITION_TEST
		if (state == 0)
		{
			if (positionReached(lastTargetPositionBottom, spi_readInt(MOTOR_BOTTOM, TMC4671_PID_POSITION_ACTUAL),spi_readInt(MOTOR_BOTTOM, TMC4671_PID_VELOCITY_ACTUAL), maxDiff, maxVel))
			{
				// motor_0 reached target position
				state = 1;

#ifdef DEMO1
				// turn motor_0
				lastTargetPositionBottom = Motor_0_Position_Offset;
				spi_writeInt(MOTOR_0, TMC4671_PID_POSITION_TARGET, lastTargetPositionBottom);
#else
				// turn motor_1
				lastTargetPositionTOP = (positionCounter*65536) + Motor_TOP_Position_Offset;
				spi_writeInt(MOTOR_TOP, TMC4671_PID_POSITION_TARGET, lastTargetPositionTOP);
#endif
			}
		}
		else
		{
#ifdef DEMO1
			if (positionReached(lastTargetPositionBottom, spi_readInt(MOTOR_BOTTOM, TMC4671_PID_POSITION_ACTUAL),spi_readInt(MOTOR_BOTTOM, TMC4671_PID_VELOCITY_ACTUAL), maxDiff, maxVel))
#else
			if (positionReached(lastTargetPositionTOP, spi_readInt(MOTOR_TOP, TMC4671_PID_POSITION_ACTUAL),spi_readInt(MOTOR_TOP, TMC4671_PID_VELOCITY_ACTUAL), maxDiff, maxVel))
#endif
			{
				// motor_0 reached target position
				state = 0;

				// ===== interfacing by TRMI =====
				// adjust max velocity of motor 1 by max velocity of motor 0
				//spi_writeInt(MOTOR_1, TMC4671_PID_VELOCITY_LIMIT, spi_readInt(MOTOR_0, TMC4671_PID_VELOCITY_LIMIT));

				// ===== adjust max velocity of motor 0 and 1 by reading ADC_AGPI =====

				// select ADC_AGPI_A for ADC_RAW_DATA
				spi_writeInt(MOTOR_BOTTOM, TMC4671_ADC_RAW_ADDR, 1);
				int32_t maxADC = spi_readInt(MOTOR_BOTTOM, TMC4671_ADC_RAW_DATA);
				maxADC = maxADC >> 16;
				maxADC = maxADC &0x0000FFFF;

				// adc range is between ~35.000 and ~48.000
				// remove adc offset
				maxADC = maxADC - 35000;

				// limit adc-offset between 0 and 13000
				if (maxADC < 0)
					maxADC = 0;

				if (maxADC > 13000)
					maxADC = 13000;

				// scale value to 60..1200rpm
				maxADC = 60 + (maxADC * 47 / 650);

				spi_writeInt(MOTOR_BOTTOM, TMC4671_PID_VELOCITY_LIMIT, maxADC);
				spi_writeInt(MOTOR_TOP, TMC4671_PID_VELOCITY_LIMIT, maxADC);

#ifdef DEMO1
				// turn motor_0
				lastTargetPositionBottom = 65536 + Motor_0_Position_Offset;
				spi_writeInt(MOTOR_0, TMC4671_PID_POSITION_TARGET, lastTargetPositionBottom);
#else
				// turn motor_0
				lastTargetPositionBottom = (positionCounter*65536) + Motor_BOTTOM_Position_Offset;
				spi_writeInt(MOTOR_BOTTOM, TMC4671_PID_POSITION_TARGET, lastTargetPositionBottom);

				positionCounter++;
#endif
			}
		}
#endif
	}
	return 0;
}
