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
    tmc5072_writeRegister(icID, TMC5072_CHOPCONF(0), 0x00010135);
    tmc5072_writeRegister(icID, TMC5072_IHOLD_IRUN(0), 0x00071400);

    //2-phase configuration Motor 2
    tmc5072_writeRegister(icID, TMC5072_CHOPCONF(1), 0x00010135);
    tmc5072_writeRegister(icID, TMC5072_IHOLD_IRUN(1), 0x00071400);


    //Reset positions
    tmc5072_writeRegister(icID, TMC5072_RAMPMODE(0), TMC5072_MODE_POSITION);
    tmc5072_writeRegister(icID, TMC5072_XTARGET(0), 0);
    tmc5072_writeRegister(icID, TMC5072_XACTUAL(0), 0);
    tmc5072_writeRegister(icID, TMC5072_RAMPMODE(1), TMC5072_MODE_POSITION);
    tmc5072_writeRegister(icID, TMC5072_XTARGET(1), 0);
    tmc5072_writeRegister(icID, TMC5072_XACTUAL(1), 0);

    //Standard values for speed and acceleration
    tmc5072_writeRegister(icID, TMC5072_VSTART(0), 1);
    tmc5072_writeRegister(icID, TMC5072_A1(0), 5000);
    tmc5072_writeRegister(icID, TMC5072_V1(0), 26843);
    tmc5072_writeRegister(icID, TMC5072_AMAX(0), 5000);   
    tmc5072_writeRegister(icID, TMC5072_VMAX(0), 100000);
    tmc5072_writeRegister(icID, TMC5072_DMAX(0), 5000);
    tmc5072_writeRegister(icID, TMC5072_D1(0), 5000);
    tmc5072_writeRegister(icID, TMC5072_VSTOP(0), 10);

    tmc5072_writeRegister(icID, TMC5072_VSTART(1), 1);
    tmc5072_writeRegister(icID, TMC5072_A1(1), 5000);
    tmc5072_writeRegister(icID, TMC5072_V1(1), 26843);
    tmc5072_writeRegister(icID, TMC5072_AMAX(1), 5000);
    tmc5072_writeRegister(icID, TMC5072_VMAX(1), 100000);
    tmc5072_writeRegister(icID, TMC5072_DMAX(1), 5000);
    tmc5072_writeRegister(icID, TMC5072_D1(1), 5000);
    tmc5072_writeRegister(icID, TMC5072_VSTOP(1), 10);
}

