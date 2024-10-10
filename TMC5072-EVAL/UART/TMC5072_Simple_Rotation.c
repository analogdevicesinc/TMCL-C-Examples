/*******************************************************************************
* Copyright © 2024 Analog Devices Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/


#include "TMC5072_Simple_Rotation.h"

/*
 * Configures the registers with the right settings that are needed for rotating the motor.
 * E.g Enabling driver, setting IRUN current etc.
 */
void initAllMotors(uint16_t icID) {
    //2-phase configuration Motor 1
    tmc5072_writeRegister(icID, TMC5072_CHOPCONF_1, 0x00010135);
    tmc5072_writeRegister(icID, TMC5072_IHOLD_IRUN_1, 0x00071400);

    //2-phase configuration Motor 2
    tmc5072_writeRegister(icID, TMC5072_CHOPCONF_2, 0x00010135);
    tmc5072_writeRegister(icID, TMC5072_IHOLD_IRUN_2, 0x00071400);


    //Reset positions
    tmc5072_writeRegister(icID, TMC5072_RAMPMODE_1, TMC5072_MODE_POSITION);
    tmc5072_writeRegister(icID, TMC5072_XTARGET_1, 0);
    tmc5072_writeRegister(icID, TMC5072_XACTUAL_1, 0);
    tmc5072_writeRegister(icID, TMC5072_RAMPMODE_2, TMC5072_MODE_POSITION);
    tmc5072_writeRegister(icID, TMC5072_XTARGET_2, 0);
    tmc5072_writeRegister(icID, TMC5072_XACTUAL_2, 0);

    //Standard values for speed and acceleration
    tmc5072_writeRegister(icID, TMC5072_VSTART_1, 1);
    tmc5072_writeRegister(icID, TMC5072_A1_1, 5000);
    tmc5072_writeRegister(icID, TMC5072_V1_1, 26843);
    tmc5072_writeRegister(icID, TMC5072_AMAX_1, 5000);   
    tmc5072_writeRegister(icID, TMC5072_VMAX_1, 100000);
    tmc5072_writeRegister(icID, TMC5072_DMAX_1, 5000);
    tmc5072_writeRegister(icID, TMC5072_D1_1, 5000);
    tmc5072_writeRegister(icID, TMC5072_VSTOP_1, 10);

    tmc5072_writeRegister(icID, TMC5072_VSTART_2, 1);
    tmc5072_writeRegister(icID, TMC5072_A1_2, 5000);
    tmc5072_writeRegister(icID, TMC5072_V1_2, 26843);
    tmc5072_writeRegister(icID, TMC5072_AMAX_2, 5000);
    tmc5072_writeRegister(icID, TMC5072_VMAX_2, 100000);
    tmc5072_writeRegister(icID, TMC5072_DMAX_2, 5000);
    tmc5072_writeRegister(icID, TMC5072_D1_2, 5000);
    tmc5072_writeRegister(icID, TMC5072_VSTOP_2, 10);
}

