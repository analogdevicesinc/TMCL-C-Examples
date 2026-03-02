/*******************************************************************************
* Copyright © 2025 Analog Devices Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#include <string.h> // For memcpy

#include "TMC6460.h"

// Internal bus implementations
static int32_t readRegisterSPI(uint16_t icID, uint16_t address, uint32_t *readValue);
static int32_t writeRegisterSPI(uint16_t icID, uint16_t address, uint32_t writeValue);
static int32_t readRegisterUART(uint16_t icID, uint16_t address, uint32_t *readValue);
static int32_t writeRegisterUART(uint16_t icID, uint16_t address, uint32_t writeValue);

// Constants
#define SPI_WRITE_BIT     0x80
#define UART_SYNC_BITS    0x42
#define UART_STREAM_WRITE 0x41
#define UART_WRITE_BIT    0x08

// Register access functions
int32_t tmc6460_readRegister(uint16_t icID, uint16_t address, uint32_t *readValue)
{
    enum ICBusType bus = tmc6460_getBusType(icID);

    if (bus == IC_BUS_SPI)
    {
        return readRegisterSPI(icID, address, readValue);
    }
    else if (bus == IC_BUS_UART)
    {
        return readRegisterUART(icID, address, readValue);
    }
    else
    {
        return -1;
    }

    return 0;
}

int32_t tmc6460_writeRegister(uint16_t icID, uint16_t address, uint32_t writeValue)
{
    enum ICBusType bus = tmc6460_getBusType(icID);

    if (bus == IC_BUS_SPI)
    {
        return writeRegisterSPI(icID, address, writeValue);
    }
    else if (bus == IC_BUS_UART)
    {
        return writeRegisterUART(icID, address, writeValue);
    }
    else
    {
        return -1;
    }

    return 0;
}

// SPI communication
static int32_t readRegisterSPI(uint16_t icID, uint16_t address, uint32_t *readValue)
{
    uint8_t data[6] = { 0 };

    // Write address, writeNRead bit cleared
    data[0] = (address >> 8) & 0x03;
    data[1] = address & 0xFF;

    // Send the read request
    tmc6460_readWriteSPI(icID, &data[0], sizeof(data));

    // Rewrite address, writeNRead bit
    data[0] = (address >> 8) & 0x03;
    data[1] = address & 0xFF;

    // Send another request to receive the read reply
    tmc6460_readWriteSPI(icID, &data[0], sizeof(data));

    *readValue = ((uint32_t) data[2] << 24)
               | ((uint32_t) data[3] << 16)
               | ((uint32_t) data[4] <<  8)
               | ((uint32_t) data[5]);

    return 0;
}

static int32_t writeRegisterSPI(uint16_t icID, uint16_t address, uint32_t writeValue)
{
    uint8_t data[6] = { 0 };

    // Write address, writeNRead bit set
    data[0] = SPI_WRITE_BIT | ((address >> 8) & 0x03);
    data[1] = address & 0xFF;
    data[2] = writeValue >> 24;
    data[3] = writeValue >> 16;
    data[4] = writeValue >>  8;
    data[5] = writeValue & 0xFF;

    // Send the write request
    tmc6460_readWriteSPI(icID, &data[0], sizeof(data));

    return 0;
}
// UART communication
static int32_t readRegisterUART(uint16_t icID, uint16_t address, uint32_t *readValue)
{
    size_t writeSize = 2;
    size_t readSize  = 6;

    uint8_t data[7] = { 0 };

    data[0] = UART_SYNC_BITS | ((address >> 4) & 0x30);
    data[1] = address & 0xFF;


    if (!tmc6460_readWriteUART(icID, &data[0], writeSize, readSize))
        return -2;

    *readValue = ((uint32_t) data[2] << 24)
               | ((uint32_t) data[3] << 16)
               | ((uint32_t) data[4] <<  8)
               | ((uint32_t) data[5]);

    return 0;
}

static int32_t writeRegisterUART(uint16_t icID, uint16_t address, uint32_t writeValue)
{
    size_t writeSize = 6;
    size_t readSize  = 5;

    uint8_t data[7] = { 0 };

    data[0] = UART_SYNC_BITS | UART_WRITE_BIT | ((address >> 4) & 0x30);
    data[1] = address & 0xFF;
    data[2] = (writeValue >> 24) & 0xFF;
    data[3] = (writeValue >> 16) & 0xFF;
    data[4] = (writeValue >>  8) & 0xFF;
    data[5] = (writeValue      ) & 0xFF;

    if (!tmc6460_readWriteUART(icID, &data[0], writeSize, readSize))
        return -2;
    

    return 0;
}