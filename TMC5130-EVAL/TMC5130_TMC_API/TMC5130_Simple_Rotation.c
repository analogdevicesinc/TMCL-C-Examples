/*******************************************************************************
* Copyright © 2024 Analog Devices Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#include "TMC5130_Simple_Rotation.h"

/*
 * Configures the registers with the right settings that are needed for rotating the motor.
 * E.g Enabling driver, setting IRUN current etc.
 */
void initAllMotors(uint16_t icID)
{
    tmc5130_writeRegister(icID, TMC5130_GCONF, 0x000000C0);      // GCONF
    tmc5130_writeRegister(icID, TMC5130_SLAVECONF, 0x00000300);  // SLAVECONF (for UART address if needed)
    tmc5130_writeRegister(icID, TMC5130_IHOLD_IRUN, 0x0000AF08); // IHOLD_IRUN
    tmc5130_writeRegister(icID, TMC5130_CHOPCONF, 0x10000053);   // CHOPCONF
    tmc5130_writeRegister(icID, TMC5130_TPOWERDOWN, 0x00000014);
    tmc5130_writeRegister(icID, TMC5130_PWMCONF, 0xC10D0024);
}
