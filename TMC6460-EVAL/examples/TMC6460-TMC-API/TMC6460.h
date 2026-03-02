/*******************************************************************************
* Copyright © 2025 Analog Devices Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

/*
 * TMC6460.h
 *
 *  Created on: 31.7.2023
 *      Author: LH
 */

#ifndef TMC_IC_TMC6460_H_
#define TMC_IC_TMC6460_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "TMC6460_HW_Abstraction.h"

enum ICBusType {
    IC_BUS_SPI,
    IC_BUS_UART,

    IC_BUS_END_
};

// => TMC-API wrapper
extern void tmc6460_readWriteSPI(uint16_t icID, uint8_t *data, size_t dataLength);
extern bool tmc6460_readWriteUART(uint16_t icID, uint8_t *data, size_t writeLength, size_t readLength);
extern enum ICBusType tmc6460_getBusType(uint16_t icID);

int32_t tmc6460_readRegister(uint16_t icID, uint16_t address, uint32_t *readValue);
int32_t tmc6460_writeRegister(uint16_t icID, uint16_t address, uint32_t writeValue);

static inline uint32_t tmc6460_fieldExtract(uint32_t registerValue, TMC6460RegisterField field)
{
    uint32_t fieldValue = (registerValue & field.mask) >> field.shift;

    if (field.isSigned)
    {
        // Apply signedness conversion
        uint32_t baseMask = field.mask >> field.shift;
        uint32_t signMask = baseMask & (~baseMask >> 1);
        fieldValue = (fieldValue ^ signMask) - signMask;
    }

    return fieldValue;
}

static inline int32_t tmc6460_fieldRead(uint16_t icID, TMC6460RegisterField field, uint32_t *readValue)
{
    int32_t err = tmc6460_readRegister(icID, field.addr, readValue);
    if (err < 0)
        return err;

    *readValue = tmc6460_fieldExtract(*readValue, field);

    return 0;
}

static inline uint32_t tmc6460_fieldUpdate(uint32_t registerValue, TMC6460RegisterField field, uint32_t writeValue)
{
    registerValue &= ~field.mask;
    registerValue |= (writeValue << field.shift) & field.mask;

    return registerValue;
}

static inline int32_t tmc6460_fieldWrite(uint16_t icID, TMC6460RegisterField field, uint32_t writeValue)
{
    uint32_t value = 0;
    int32_t err = tmc6460_readRegister(icID, field.addr, &value);
    if (err < 0)
        return err;

    value = tmc6460_fieldUpdate(value, field, writeValue);

    return tmc6460_writeRegister(icID, field.addr, value);
}

static inline bool tmc6460_fieldInRange(TMC6460RegisterField field, uint32_t fieldValue)
{
    if (field.isSigned)
    {
        if ((int32_t) fieldValue > (int32_t) (field.mask >> (field.shift + 1)))
            return false;

        if ((int32_t) fieldValue < (int32_t) (~(field.mask >> (field.shift + 1))))
            return false;
    }
    else
    {
        if (fieldValue > (field.mask >> field.shift))
            return false;
    }

    return true;
}

#endif /* TMC_IC_TMC6460_H_ */
