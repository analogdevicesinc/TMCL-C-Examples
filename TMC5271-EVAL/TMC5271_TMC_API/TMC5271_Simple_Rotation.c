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
}