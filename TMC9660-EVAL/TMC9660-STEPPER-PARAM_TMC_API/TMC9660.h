/*******************************************************************************
* Copyright © 2025 Analog Devices Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/


#ifndef TMC_IC_TMC9660_H_
#define TMC_IC_TMC9660_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
//#include "TMC9660_HW_Abstraction.h"

/*******************************************************************************
* API Configuration Defines
* These control optional features of the TMC-API implementation.
* These can be commented in/out here or defined from the build system.
*******************************************************************************/

// Uncomment if you want to save space.....
// and put the table into your own .c file
//#define TMC_API_EXTERNAL_CRC_TABLE 1


/******************************************************************************/

typedef enum {
    TMC9660_BUS_SPI,
    TMC9660_BUS_UART,
} TMC9660BusType;

typedef struct {
    uint8_t device;
    uint8_t host;
} TMC9660BusAddresses;

typedef enum {
    TMC9660_CMD_MST = 3,
    TMC9660_CMD_SAP = 5,
    TMC9660_CMD_GAP = 6,
    TMC9660_CMD_GET_VERSION = 136,
    // ToDo complete this list
} TMC9660Command;

typedef enum {
    TMC9660_PARAMSTATUS_OK = 100,
    // ToDo complete this list
} TMC9660ParamStatus;

// => TMC-API wrapper
//extern void tmc9660_readWriteSPI(uint16_t icID, uint8_t *data, size_t dataLength);
extern bool tmc9660_readWriteUART(uint16_t icID, uint8_t *data, size_t writeLength, size_t readLength);

extern TMC9660BusType tmc9660_getBusType(uint16_t icID);
extern TMC9660BusAddresses tmc9660_getBusAddresses(uint16_t icID);
// => TMC-API wrapper

/*** TMC9660 Parameter Mode ***************************************************/
int32_t tmc9660_param_sendCommand(uint16_t icID, uint8_t cmd, uint16_t type, uint8_t index, uint32_t *value);
int8_t tmc9660_boot_sendCommand(uint16_t icID, uint8_t cmd, uint32_t *value);

uint32_t tmc9660_param_getParameter(uint16_t icID, uint16_t type);
bool tmc9660_param_setParameter(uint16_t icID, uint16_t type, uint32_t value);

/******************************************************************************/

#endif /* TMC_IC_TMC9660_H_ */
