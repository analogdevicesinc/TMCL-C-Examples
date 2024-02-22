/*******************************************************************************
* Copyright © 2023 Analog Devices Inc. All Rights Reserved. This software is
* proprietary & confidential to Analog Devices, Inc. and its licensors.
*******************************************************************************/


#include "TMC5271_Simple_Rotation.h"

/*
 * Configures the registers with the right settings that are needed for rotating both motors.
 * E.g Enabling drivers, setting IRUN current etc.
 */
void initAllMotors(uint16_t icID)
{
    // Set IREF_R2 and IREF_R3 to High for setting the REF resistor to 10k.
    // Run Current = 0.2A rms
    // Hold Current = 0.14A rms
    // Chopper Mode = SpreadCycle

    // Motor Configurations
    tmc5271_writeRegister(icID, TMC5271_GCONF, 0x00004002);     
    tmc5271_writeRegister(icID, TMC5271_DRV_CONF, 0x0000000D);
    tmc5271_writeRegister(icID, TMC5271_GLOBAL_SCALER, 0x0000FBFB);

    // Motor Current configurations
    tmc5271_writeRegister(icID, TMC5271_IHOLD_IRUN, 0x04010F0A);        // writing value 0x04011F0A = 67182346 = 0.0 to address 10 = 0x12(IHOLD_IRUN)
    tmc5271_writeRegister(icID, TMC5271_CHOPCONF, 0x10410153);          // writing value 0x10410153 = 272695635 = 0.0 to address 39 = 0x38(CHOPCONF)
    tmc5271_writeRegister(icID, TMC5271_AMAX, 51200);                   // writing value to address 21 = 0x20(AMAX)
}