/*******************************************************************************
* Copyright © 2020 TRINAMIC Motion Control GmbH & Co. KG
* (now owned by Analog Devices Inc.),
*
* Copyright © 2023-2025 Analog Devices Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

/*
 * Author: Trinamic Software Team
 *
 *  Functions to handle shifting/masking values into registers.
 *  See include/memoryMap for actual register maps.
 */

#ifndef REGISTERS_H_
#define REGISTERS_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint32_t mask;
    uint8_t shift;
    uint32_t address;
    bool isSigned;
} RegisterField32;

typedef struct
{
    uint8_t mask;
    uint8_t shift;
    uint32_t address;
    bool isSigned;
} RegisterField8;

typedef struct
{
    uint16_t mask;
    uint8_t shift;
    uint32_t address;
    bool isSigned;
} RegisterField16;

#define ACCESS_ONCE(x) *((volatile typeof(x) *) (&x))

static inline uint32_t field_extract32(uint32_t data, RegisterField32 field)
{
    uint32_t value = (data & field.mask) >> field.shift;

    if (field.isSigned)
    {
        // Apply signedness conversion
        uint32_t baseMask = field.mask >> field.shift;
        uint32_t signMask = baseMask & (~baseMask >> 1);
        value = (value ^ signMask) - signMask;
    }

    return value;
}

static inline uint32_t field_update32(uint32_t data, RegisterField32 field, uint32_t value)
{
    return (data & (~field.mask)) | ((value << field.shift) & field.mask);
}


static inline uint16_t field_extract16(uint16_t data, RegisterField16 field)
{
    uint16_t value = (ACCESS_ONCE(data) & field.mask) >> field.shift;

    if (field.isSigned)
    {
        // Apply signedness conversion
        uint16_t baseMask = field.mask >> field.shift;
        uint16_t signMask = baseMask & (~baseMask >> 1);
        value = (value ^ signMask) - signMask;
    }

    return value;
}

static inline uint16_t field_update16(uint16_t data, RegisterField16 field, uint16_t value)
{
    return (data & (~field.mask)) | ((value << field.shift) & field.mask);
}


static inline uint8_t field_extract8(uint8_t data, RegisterField8 field)
{
    uint8_t value = (ACCESS_ONCE(data) & field.mask) >> field.shift;

    if (field.isSigned)
    {
        // Apply signedness conversion
        uint8_t baseMask = field.mask >> field.shift;
        uint8_t signMask = baseMask & (~baseMask >> 1);
        value = (value ^ signMask) - signMask;
    }

    return value;
}

static inline uint8_t field_update8(uint8_t data, RegisterField8 field, uint8_t value)
{
    return (data & (~field.mask)) | ((value << field.shift) & field.mask);
}


#endif /* REGISTERS_H_ */
