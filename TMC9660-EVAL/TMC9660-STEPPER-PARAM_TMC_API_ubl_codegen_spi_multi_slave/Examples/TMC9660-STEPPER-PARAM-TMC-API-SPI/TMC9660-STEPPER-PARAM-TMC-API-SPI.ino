/*******************************************************************************
* Copyright © 2026 Analog Devices, Inc.
*******************************************************************************/

/**
 * TMC9660 Example: Multi-Slave SPI Communication
 * 
 * This example communicates with multiple TMC9660 slaves over the SPI protocol
 * using multiple chip select pins for independent device control.
 * Two different methods of accessing the TMC9660 are demonstrated: simple access
 * and pipeline access. The access times of both methods are recorded.
 * 
 * ===================== PINOUT =========================================
 * 
 * Arduino Pin          Function              Eval Board Pin    Signal
 * ─────────────────────────────────────────────────────────────────────
 * 51                   MOSI                  32                SPI1_SDI
 * 50                   MISO                  33                SPI1_SDO
 * 52                   SCK                   31                SPI1_SCK
 * 31                   CS1 (IC1)             31                SPI1_CSN
 * 30                   CS2 (IC2)             31                SPI1_CSN
 * GND                  Ground                2                 GND
 * +5V                  Power Supply          5                 +5V_USB
 * 41                   IC1_HOLD_FLASH        34                DIO12
 * 42                   IC2_HOLD_FLASH        34                DIO12
 * 40                   IC1_FAULTN_STATUS     18                DIO07
 * 39                   IC2_FAULTN_STATUS     18                DIO07
 * 49                   IC1_RESET_CTRL        19                DIO8
 * 48                   IC2_RESET_CTRL        19                DIO8
 * =====================================================================
 */

#include <SPI.h>

extern "C"
{
#include "TMC9660.h"
#include "TMC9660_PARAM_HW_Abstraction.h"
#include "TMC9660_BL_HW_Abstraction.h"
}

#define IC_1       0
#define IC_2       1
#define IC2_ENABLE 1
#define IC1_ENABLE 1

// Addon binary data for Addon ID: "SPI", Version v0.6
// Loaded from TMC9660_addon_spi_v6.hex
#define TMC9660_ADDON_ID      0x00495053 // "SPI" in little-endian ASCII
#define TMC9660_ADDON_VERSION ((0 << 16) | 6)
static const uint8_t tmc9660_addon_SPI_v0_6[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0x45, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61, 0x64, 0x64,
    0x5F, 0x6F, 0x6E, 0x3A, 0x20, 0x53, 0x50, 0x49, 0x00, 0x06, 0x00, 0x00, 0x00, 0x81, 0xBA, 0x00, 0x0D, 0x50, 0xC1,
    0xB8, 0x54, 0x17, 0x05, 0x00, 0x00, 0x13, 0x05, 0x85, 0xFD, 0xB7, 0x15, 0x02, 0x00, 0x93, 0x85, 0x05, 0xC0, 0x97,
    0x02, 0x00, 0x00, 0x93, 0x82, 0xC2, 0x0D, 0x01, 0x4E, 0x63, 0x1C, 0xB5, 0x02, 0x17, 0x03, 0x00, 0x00, 0x13, 0x03,
    0x23, 0x3D, 0xB3, 0x03, 0x53, 0x40, 0x37, 0xCE, 0x02, 0x00, 0x61, 0x1E, 0x33, 0x0E, 0x7E, 0x40, 0x13, 0x5E, 0x2E,
    0x00, 0x0A, 0x0E, 0xF2, 0x8F, 0x83, 0xAE, 0x02, 0x00, 0x23, 0x20, 0xDE, 0x01, 0x91, 0x02, 0x11, 0x0E, 0xE3, 0xCA,
    0x62, 0xFE, 0x33, 0x0E, 0x5E, 0x40, 0xFE, 0x82, 0x37, 0x0F, 0x02, 0x00, 0x13, 0x0F, 0x0F, 0x05, 0x23, 0x20, 0x5F,
    0x00, 0x97, 0x02, 0x00, 0x00, 0x93, 0x82, 0xA2, 0xF7, 0x03, 0xA3, 0x82, 0x01, 0xB7, 0xC3, 0x02, 0x00, 0x23, 0xAC,
    0x63, 0xFE, 0x03, 0xA3, 0xC2, 0x01, 0x23, 0xAE, 0x63, 0xFE, 0x37, 0x15, 0x02, 0x00, 0x13, 0x05, 0x45, 0xC0, 0xB7,
    0x15, 0x02, 0x00, 0x93, 0x85, 0x45, 0xC4, 0x23, 0x20, 0x05, 0x00, 0x11, 0x05, 0xE3, 0x4D, 0xB5, 0xFE, 0x37, 0x16,
    0x02, 0x00, 0x13, 0x06, 0x46, 0xC0, 0x97, 0x06, 0x00, 0x00, 0x93, 0x86, 0xC6, 0x2C, 0xF2, 0x96, 0x54, 0xC2, 0x97,
    0x06, 0x00, 0x00, 0x93, 0x86, 0xC6, 0x2C, 0xF2, 0x96, 0x14, 0xC6, 0x97, 0x06, 0x00, 0x00, 0x93, 0x86, 0xC6, 0x2C,
    0xF2, 0x96, 0x54, 0xCA, 0x97, 0x06, 0x00, 0x00, 0x93, 0x86, 0x46, 0x2E, 0xF2, 0x96, 0x14, 0xCE, 0x97, 0x06, 0x00,
    0x00, 0x93, 0x86, 0x06, 0x2F, 0xF2, 0x96, 0x54, 0xCE, 0x37, 0x15, 0x02, 0x00, 0x13, 0x05, 0x05, 0xC0, 0xB7, 0x85,
    0x65, 0x6B, 0x93, 0x85, 0x15, 0x92, 0x0C, 0xC1, 0x82, 0x80, 0xB7, 0x52, 0x02, 0x00, 0x93, 0x82, 0xC2, 0x24, 0x03,
    0xA3, 0x02, 0x02, 0x91, 0x43, 0x63, 0x19, 0x73, 0x00, 0x03, 0xAE, 0x82, 0x02, 0x8D, 0x4E, 0x63, 0x04, 0xDE, 0x01,
    0x23, 0xA4, 0xD2, 0x03, 0x82, 0x80, 0x95, 0x42, 0x63, 0x1F, 0x59, 0x00, 0x83, 0x02, 0x14, 0x00, 0x3D, 0x43, 0x63,
    0x9A, 0x62, 0x00, 0x83, 0xA2, 0x44, 0x00, 0x37, 0x53, 0x02, 0x00, 0x13, 0x03, 0x43, 0x12, 0x23, 0x28, 0x53, 0x00,
    0x82, 0x80, 0x41, 0x11, 0x06, 0xC4, 0x97, 0x02, 0x00, 0x00, 0x93, 0x82, 0xC2, 0x1E, 0x13, 0x03, 0xF0, 0x0F, 0xA3,
    0x83, 0x62, 0x00, 0x99, 0x62, 0x81, 0x12, 0x82, 0x92, 0x17, 0x03, 0x00, 0x00, 0x13, 0x03, 0x23, 0x1E, 0x23, 0x00,
    0xA3, 0x00, 0xB7, 0x03, 0x02, 0x00, 0x93, 0x83, 0x63, 0x02, 0x03, 0xDE, 0x03, 0x00, 0x97, 0x0E, 0x00, 0x00, 0x93,
    0x8E, 0x6E, 0x1C, 0x23, 0x90, 0xCE, 0x01, 0x05, 0x6F, 0xB1, 0x6F, 0xFD, 0x1F, 0x33, 0x6E, 0xEE, 0x01, 0x33, 0x7E,
    0xFE, 0x01, 0x23, 0x91, 0xCE, 0x01, 0xB7, 0xE2, 0x00, 0x48, 0xD1, 0x02, 0x05, 0x43, 0x23, 0xA0, 0x62, 0x00, 0xB7,
    0xE3, 0x00, 0x48, 0xE1, 0x03, 0x23, 0xA0, 0x03, 0x00, 0x93, 0x02, 0x00, 0x02, 0x37, 0x53, 0x00, 0x48, 0x23, 0x24,
    0x53, 0x00, 0x23, 0x2E, 0x53, 0x00, 0x37, 0xEE, 0x00, 0x48, 0x41, 0x0E, 0x83, 0x2E, 0x0E, 0x00, 0x93, 0xFE, 0xFE,
    0xF3, 0x23, 0x20, 0xDE, 0x01, 0x02, 0xC0, 0x02, 0xC2, 0x31, 0x4F, 0xA3, 0x03, 0xE1, 0x01, 0x23, 0x00, 0xE1, 0x01,
    0x0A, 0x85, 0x95, 0x6F, 0x93, 0x8F, 0xCF, 0x22, 0x82, 0x9F, 0xA2, 0x40, 0x41, 0x01, 0x82, 0x80, 0x41, 0x11, 0x06,
    0xC0, 0xB7, 0x52, 0x02, 0x00, 0x93, 0x82, 0x52, 0x39, 0x03, 0xC3, 0x02, 0x00, 0x63, 0x06, 0x03, 0x06, 0x97, 0x03,
    0x00, 0x00, 0x93, 0x83, 0x23, 0x14, 0x03, 0xC5, 0x03, 0x00, 0x31, 0xCD, 0x97, 0x05, 0x00, 0x00, 0x93, 0x85, 0x85,
    0x12, 0x03, 0xC3, 0x75, 0x00, 0x93, 0x03, 0xF0, 0x0F, 0x63, 0x0F, 0x73, 0x00, 0xC8, 0x08, 0x21, 0x46, 0x71, 0x4E,
    0x02, 0x9E, 0x97, 0x02, 0x00, 0x00, 0x93, 0x82, 0xC2, 0x10, 0x93, 0x03, 0xF0, 0x0F, 0xA3, 0x83, 0x72, 0x00, 0x2D,
    0xA0, 0xB7, 0xE2, 0x00, 0x48, 0xF1, 0x02, 0x03, 0xA3, 0x02, 0x00, 0x13, 0x73, 0x43, 0x00, 0x63, 0x0F, 0x03, 0x00,
    0xC8, 0x08, 0x95, 0x63, 0x93, 0x83, 0x63, 0x25, 0x82, 0x93, 0x83, 0x42, 0xB1, 0x05, 0x13, 0x03, 0xF0, 0x0F, 0xE3,
    0x8E, 0x62, 0xFC, 0x05, 0x45, 0x31, 0xA8, 0x01, 0x45, 0x97, 0x03, 0x00, 0x00, 0x93, 0x83, 0x63, 0x0D, 0x03, 0xDE,
    0x03, 0x00, 0xB7, 0x03, 0x02, 0x00, 0x93, 0x83, 0x63, 0x02, 0x23, 0x90, 0xC3, 0x01, 0x82, 0x40, 0x41, 0x01, 0x82,
    0x80, 0x01, 0x45, 0x85, 0x42, 0x13, 0x03, 0xD9, 0xFF, 0x63, 0xE4, 0x62, 0x08, 0x41, 0x11, 0x06, 0xC6, 0xB7, 0xE2,
    0x00, 0x48, 0xF1, 0x02, 0x03, 0xA3, 0x02, 0x00, 0x13, 0x73, 0x43, 0x00, 0x63, 0x0A, 0x03, 0x00, 0x17, 0x05, 0x00,
    0x00, 0x13, 0x05, 0xC5, 0x08, 0x95, 0x63, 0x93, 0x83, 0x63, 0x25, 0x82, 0x93, 0x63, 0x9D, 0x09, 0x04, 0x83, 0xC7,
    0x24, 0x00, 0x23, 0x03, 0xF1, 0x00, 0x3E, 0x83, 0x03, 0xC7, 0x34, 0x00, 0xA3, 0x02, 0xE1, 0x00, 0x3A, 0x93, 0x03,
    0xC7, 0x74, 0x00, 0x23, 0x02, 0xE1, 0x00, 0x3A, 0x93, 0x03, 0xC7, 0x64, 0x00, 0xA3, 0x01, 0xE1, 0x00, 0x3A, 0x93,
    0x03, 0xC7, 0x54, 0x00, 0x23, 0x01, 0xE1, 0x00, 0x3A, 0x93, 0x03, 0xC7, 0x44, 0x00, 0xA3, 0x00, 0xE1, 0x00, 0x3A,
    0x93, 0x13, 0x07, 0xF0, 0x0F, 0xA3, 0x03, 0xE1, 0x00, 0x3A, 0x93, 0x23, 0x00, 0x61, 0x00, 0x0A, 0x85, 0x15, 0x6F,
    0x13, 0x0F, 0xCF, 0x22, 0x02, 0x9F, 0x05, 0x45, 0xB2, 0x40, 0x41, 0x01, 0x82, 0x80, 0x97, 0x03, 0x00, 0x00, 0x93,
    0x83, 0x43, 0x02, 0x03, 0xDE, 0x03, 0x00, 0xB7, 0x03, 0x02, 0x00, 0x93, 0x83, 0x63, 0x02, 0x23, 0x90, 0xC3, 0x01,
    0x82, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x45, 0x83, 0x42, 0x14, 0x00, 0x13, 0x03, 0xD0, 0x09, 0x63, 0x96, 0x62, 0x02, 0x83, 0x43, 0x24, 0x00,
    0x93, 0x83, 0x43, 0xF3, 0x05, 0x4E, 0x63, 0x6F, 0x7E, 0x00, 0x8A, 0x03, 0xB7, 0xCE, 0x02, 0x00, 0x9E, 0x9E, 0x03,
    0xAF, 0x8E, 0xFF, 0x23, 0xA2, 0xE4, 0x01, 0x93, 0x0F, 0x40, 0x06, 0x23, 0x81, 0xF4, 0x01, 0x05, 0x45, 0x82, 0x80,
    0x41, 0x11, 0x06, 0xC0, 0xD1, 0x33, 0x82, 0x40, 0x41, 0x01, 0x82, 0x80, 0x41, 0x11, 0x06, 0xC0, 0x95, 0x3B, 0x82,
    0x40, 0x41, 0x01, 0x82, 0x80, 0x41, 0x11, 0x22, 0xC0, 0x26, 0xC2, 0x06, 0xC4, 0x2A, 0x84, 0xAE, 0x84, 0xA5, 0x3F,
    0x4D, 0x37, 0x19, 0xE1, 0xA2, 0x40, 0x21, 0xA0, 0xAD, 0x60, 0x93, 0x80, 0xA0, 0x94, 0x02, 0x44, 0x92, 0x44, 0x41,
    0x01, 0x82, 0x80, 0x41, 0x11, 0x06, 0xC0, 0x3D, 0x35, 0x19, 0xE1, 0x82, 0x40, 0x21, 0xA0, 0xA9, 0x60, 0x93, 0x80,
    0x40, 0x7B, 0x41, 0x01, 0x01, 0x45, 0x82, 0x80, 0x01, 0x11, 0x26, 0xC0, 0x4A, 0xC2, 0x4E, 0xC4, 0x06, 0xC6, 0x22,
    0xC8, 0x2A, 0x84, 0xAE, 0x84, 0x32, 0x89, 0xB6, 0x89, 0x3D, 0x3B, 0x45, 0x35, 0x19, 0xE1, 0xB2, 0x40, 0x21, 0xA0,
    0xAD, 0x60, 0x93, 0x80, 0xC0, 0x37, 0x82, 0x44, 0x12, 0x49, 0xA2, 0x49, 0x42, 0x44, 0x05, 0x61, 0x82, 0x80, 0x00,
    0x00, 0x00, 0x00,
};

// This function returns 0 on success, a negative error code on failure.
// The error code consists of the TMC-API error number in the upper 16 bits,
// and the step of the sequence that has failed in the lower 16 bits.
int32_t tmc9660_bl_installAddon_SPI_v0_6(uint16_t icID)
{
    int32_t err    = 0;
    uint16_t count = 0;

    // Install the addon
    if ((count++, err = tmc9660_bl_installAddon(icID, &tmc9660_addon_SPI_v0_6[0], sizeof(tmc9660_addon_SPI_v0_6))) < 0)
        return (err * (1 << 16)) + count;

    // Wait for the addon installation to complete
    tmc_delayMicroseconds(1000);

    // Verify the addon installation
    uint32_t id      = 0;
    uint32_t version = 0;
    if ((count++, err = tmc9660_bl_getAddonInfo(icID, &id, &version)) < 0)
        return (err * (1 << 16)) + count;

    count++;
    if (id != TMC9660_ADDON_ID)
        return (TMC9660_ERROR_INVALID_ADDON_INSTALL * (1 << 16)) + count;
    if (version != TMC9660_ADDON_VERSION)
        return (TMC9660_ERROR_INVALID_ADDON_INSTALL * (1 << 16)) + count;

    return 0;
}

static TMC9660BusType activeBus = TMC9660_BUS_SPI;
static TMC9660BusAddresses busAddresses;

int IC1_HOLDN_FLASH   = 41;
int IC2_HOLDN_FLASH   = 42;
int IC2_FAULTN_STATUS = 39;
int IC1_FAULTN_STATUS = 40;
int CS1               = 31;
int CS2               = 30;
int IC1_RESET_CTRL    = 49;
int IC2_RESET_CTRL    = 48;

void setChipAddresses(uint16_t icID, uint8_t device, uint8_t host)
{
    busAddresses.device = device;
    busAddresses.host   = host;
}

TMC9660BusType tmc9660_getBusType(uint16_t icID)
{
    return activeBus;
}

TMC9660BusAddresses tmc9660_getBusAddresses(uint16_t icID)
{
    return busAddresses;
}

uint32_t tmc_getMicrosecondTimestamp()
{
    uint32_t timestamp = micros();
    return timestamp;
}

bool tmc9660_isFaultPinAsserted(uint16_t icID)
{
    // Note: The FAULTN pin is active low.
    // It is asserted, when it is low!
    if (icID == IC_1)
        return digitalRead(IC1_FAULTN_STATUS) == LOW;
    else
        return digitalRead(IC2_FAULTN_STATUS) == LOW;
}

void tmc9660_readWriteSPI(uint16_t icID, uint8_t *data, size_t dataLength, bool keepCSNAsserted)
{
    if (icID == IC_1)
        digitalWrite(CS1, LOW);
    else
        digitalWrite(CS2, LOW);

    delayMicroseconds(10);
    for (uint32_t i = 0; i < dataLength; i++)
    {
        data[i] = SPI.transfer(data[i]);
    }

    delayMicroseconds(10);
    if (!keepCSNAsserted)
    {
        if (icID == IC_1)
            digitalWrite(CS1, HIGH);
        else
            digitalWrite(CS2, HIGH);
    }
    delayMicroseconds(10);
}

bool tmc9660_readWriteUART(uint16_t icID, uint8_t *data, size_t writeLength, size_t readLength)
{
    Serial3.write(data, writeLength);
    delay(2);

    unsigned long startTime = millis();

    // Wait for write echo
    while (Serial3.available() < writeLength)
    {
        if (millis() - startTime >= 1000)
        {
            return false;
        }
    }

    // Wait for the actual response
    startTime = millis();
    while (Serial3.available() < readLength)
    {
        if (millis() - startTime >= 1000)
        {
            return false; // Timeout
        }
    }

    // Read the actual response
    Serial3.readBytes(data, readLength);
    return true;
}

static void rotateMotorOpenLoop(uint16_t icID, uint32_t targetVelocity)
{

    tmc9660_param_setParameter(icID, TMC9660_PARAM_MOTOR_TYPE, 2); // STEPPER_MOTOR
    tmc9660_param_setParameter(icID, TMC9660_PARAM_OPENLOOP_VOLTAGE, 1000);
    tmc9660_param_setParameter(icID, TMC9660_PARAM_COMMUTATION_MODE, 3); // FOC_OPENLOOP_VOLTAGE_MODE
    tmc9660_param_setParameter(icID, TMC9660_PARAM_TARGET_VELOCITY, targetVelocity);
    Serial.println("Rotating motor in openloop mode for 5 secs");

    delay(5000);

    tmc9660_param_setParameter(icID, TMC9660_PARAM_TARGET_VELOCITY, 0);

    // tmc9660_param_setParameter(IC_1, TMC9660_PARAM_COMMUTATION_MODE, 0);  // SYSTEM_OFF
    Serial.println("Motor Stopped");
}

// This function returns 0 on success, a negative error code on failure.
// The error code consists of the TMC-API error number in the upper 16 bits,
// and the step of the sequence that has failed in the lower 16 bits.
int32_t tmc9660_loadBootConfig(uint16_t icID)
{
    uint32_t value = 0;
    uint16_t count = 0;
    int32_t err;

    // Initial commands
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_BANK, 0x00000005, NULL)) < 0)
        return (err * (1 << 16)) + count;

    // Modifying CONFIG.BOOT_04_BOOTSTRAP
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_ADDRESS, CONFIG_BOOT_04_BOOTSTRAP, NULL)) < 0)
        return (err * (1 << 16)) + count;
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_READ_16, 0, &value)) < 0)
        return (err * (1 << 16)) + count;
    value = field_update16(value, CONFIG_BOOT_04_BOOTSTRAP_BOOT_MODE_FIELD, 2);
    value = field_update16(value, CONFIG_BOOT_04_BOOTSTRAP_START_MOTOR_CONTROL_FIELD, 0);
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_WRITE_16, value, NULL)) < 0)
        return (err * (1 << 16)) + count;

    // Modifying CONFIG.BOOT_00_POWER
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_ADDRESS, CONFIG_BOOT_00_POWER, NULL)) < 0)
        return (err * (1 << 16)) + count;
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_READ_16, 0, &value)) < 0)
        return (err * (1 << 16)) + count;
    value = field_update16(value, CONFIG_BOOT_00_POWER_VEXT1_FIELD, 3);
    value = field_update16(value, CONFIG_BOOT_00_POWER_VEXT2_FIELD, 2);
    value = field_update16(value, CONFIG_BOOT_00_POWER_SS_VEXT1_FIELD, 0);
    value = field_update16(value, CONFIG_BOOT_00_POWER_SS_VEXT2_FIELD, 0);
    value = field_update16(value, CONFIG_BOOT_00_POWER_LDO_SHORT_FAULT_FIELD, 0);
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_WRITE_16, value, NULL)) < 0)
        return (err * (1 << 16)) + count;

    // Modifying CONFIG.BOOT_01_ADDRESS
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_ADDRESS, CONFIG_BOOT_01_ADDRESS, NULL)) < 0)
        return (err * (1 << 16)) + count;
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_READ_16, 0, &value)) < 0)
        return (err * (1 << 16)) + count;
    value = field_update16(value, CONFIG_BOOT_01_DEVICE_ADDRESS_FIELD, 1);
    value = field_update16(value, CONFIG_BOOT_01_MASTER_ADDRESS_FIELD, 255);
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_WRITE_16, value, NULL)) < 0)
        return (err * (1 << 16)) + count;
    tmc_delayMicroseconds(1000);

    // Updating communication addresses
    setChipAddresses(icID, 1, 255);

    // Modifying CONFIG.BOOT_10_APP_CONFIG_0
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_ADDRESS, CONFIG_BOOT_10_APP_CONFIG_0, NULL)) < 0)
        return (err * (1 << 16)) + count;
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_READ_16, 0, &value)) < 0)
        return (err * (1 << 16)) + count;
    value = field_update16(value, CONFIG_BOOT_10_APP_CONFIG_0_HALL_ENABLE_FIELD, 0);
    value = field_update16(value, CONFIG_BOOT_10_APP_CONFIG_0_ABN1_ENABLE_FIELD, 1);
    value = field_update16(value, CONFIG_BOOT_10_APP_CONFIG_0_ABN1_A_FIELD, 1);
    value = field_update16(value, CONFIG_BOOT_10_APP_CONFIG_0_ABN1_B_FIELD, 1);
    value = field_update16(value, CONFIG_BOOT_10_APP_CONFIG_0_ABN1_N_FIELD, 1);
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_WRITE_16, value, NULL)) < 0)
        return (err * (1 << 16)) + count;

    // Modifying CONFIG.BOOT_14_APP_CONFIG_4
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_ADDRESS, CONFIG_BOOT_14_APP_CONFIG_4, NULL)) < 0)
        return (err * (1 << 16)) + count;
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_READ_16, 0, &value)) < 0)
        return (err * (1 << 16)) + count;
    value = field_update16(value, CONFIG_BOOT_14_APP_CONFIG_4_MEM_TMCL_SCRIPT_FIELD, 1);
    value = field_update16(value, CONFIG_BOOT_14_APP_CONFIG_4_MEM_PARAMETERS_FIELD, 1);
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_WRITE_16, value, NULL)) < 0)
        return (err * (1 << 16)) + count;

    // Modifying CONFIG.BOOT_11_APP_CONFIG_1
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_ADDRESS, CONFIG_BOOT_11_APP_CONFIG_1, NULL)) < 0)
        return (err * (1 << 16)) + count;
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_READ_16, 0, &value)) < 0)
        return (err * (1 << 16)) + count;
    value = field_update16(value, CONFIG_BOOT_11_APP_CONFIG_1_REF_L_PIN_FIELD, 1);
    value = field_update16(value, CONFIG_BOOT_11_APP_CONFIG_1_REF_R_PIN_FIELD, 1);
    value = field_update16(value, CONFIG_BOOT_11_APP_CONFIG_1_REF_H_PIN_FIELD, 1);
    value = field_update16(value, CONFIG_BOOT_11_APP_CONFIG_1_ABN2_ENABLE_FIELD, 1);
    value = field_update16(value, CONFIG_BOOT_11_APP_CONFIG_1_ABN2_A_FIELD, 1);
    value = field_update16(value, CONFIG_BOOT_11_APP_CONFIG_1_ABN2_B_FIELD, 2);
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_WRITE_16, value, NULL)) < 0)
        return (err * (1 << 16)) + count;

    // Modifying CONFIG.BOOT_12_APP_CONFIG_2
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_ADDRESS, CONFIG_BOOT_12_APP_CONFIG_2, NULL)) < 0)
        return (err * (1 << 16)) + count;
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_READ_16, 0, &value)) < 0)
        return (err * (1 << 16)) + count;
    value = field_update16(value, CONFIG_BOOT_12_APP_CONFIG_2_WDG_DISABLE_FIELD, 0);
    value = field_update16(value, CONFIG_BOOT_12_APP_CONFIG_2_WDG_TIMEOUT_FIELD, 7);
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_WRITE_16, value, NULL)) < 0)
        return (err * (1 << 16)) + count;

    // Modifying CONFIG.BOOT_CLK_SEL_INIT
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_ADDRESS, CONFIG_BOOT_0C_CLK_SEL_INIT, NULL)) < 0)
        return (err * (1 << 16)) + count;
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_READ_32, 0, &value)) < 0)
        return (err * (1 << 16)) + count;
    value = field_update32(value, CONFIG_BOOT_0C_CLK_SEL_INIT_PLL_FB_DIV_FIELD, 99);
    value = field_update32(value, CONFIG_BOOT_0C_CLK_SEL_INIT_EXT_NOT_XTAL_FIELD, 0);
    value = field_update32(value, CONFIG_BOOT_0C_CLK_SEL_INIT_XTAL_CFG_FIELD, 3);
    value = field_update32(value, CONFIG_BOOT_0C_CLK_SEL_INIT_XTAL_BOOST_FIELD, 0);
    value = field_update32(value, CONFIG_BOOT_0C_CLK_SEL_INIT_EXT_NOT_INT_FIELD, 1);
    value = field_update32(value, CONFIG_BOOT_0C_CLK_SEL_INIT_PLL_OUT_SEL_FIELD, 1);
    value = field_update32(value, CONFIG_BOOT_0C_CLK_SEL_INIT_RDIV_FIELD, 15);
    value = field_update32(value, CONFIG_BOOT_0C_CLK_SEL_INIT_SYS_CLK_DIV_FIELD, 0);
    value = field_update32(value, CONFIG_BOOT_0C_CLK_SEL_INIT_PLL_CONFIG_BOOT_FIELD, 0);
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_WRITE_32, value, NULL)) < 0)
        return (err * (1 << 16)) + count;
    tmc_delayMicroseconds(20000);

    // Modifying CONFIG.BOOT_09_GPIO_PULLUP_0_15_INIT
    if ((count++,
         err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_ADDRESS, CONFIG_BOOT_09_GPIO_PULLUP_0_15_INIT, NULL)) < 0)
        return (err * (1 << 16)) + count;
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_READ_16, 0, &value)) < 0)
        return (err * (1 << 16)) + count;
    value = field_update16(value, CONFIG_BOOT_09_GPIO_PULLUP_0_15_INIT_GPIO5_PU_FIELD, 0);
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_WRITE_16, value, NULL)) < 0)
        return (err * (1 << 16)) + count;

    // Modifying CONFIG.BOOT_0A_GPIO_PULLDOWN_0_15_INIT
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_ADDRESS, CONFIG_BOOT_0A_GPIO_PULLDOWN_0_15_INIT,
                                               NULL)) < 0)
        return (err * (1 << 16)) + count;
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_READ_16, 0, &value)) < 0)
        return (err * (1 << 16)) + count;
    value = field_update16(value, CONFIG_BOOT_0A_GPIO_PULLDOWN_0_15_INIT_GPIO5_PD_FIELD, 0);
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_WRITE_16, value, NULL)) < 0)
        return (err * (1 << 16)) + count;

    // Modifying CONFIG.BOOT_0B_GPIO_16_18_INIT
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_ADDRESS, CONFIG_BOOT_0B_GPIO_16_18_INIT, NULL)) <
        0)
        return (err * (1 << 16)) + count;
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_READ_16, 0, &value)) < 0)
        return (err * (1 << 16)) + count;
    value = field_update16(value, CONFIG_BOOT_0B_GPIO_16_18_INIT_GPIO17_OUT_EN_FIELD, 0);
    value = field_update16(value, CONFIG_BOOT_0B_GPIO_16_18_INIT_GPIO18_OUT_EN_FIELD, 0);
    value = field_update16(value, CONFIG_BOOT_0B_GPIO_16_18_INIT_GPIO17_PD_FIELD, 1);
    value = field_update16(value, CONFIG_BOOT_0B_GPIO_16_18_INIT_GPIO18_PD_FIELD, 1);
    value = field_update16(value, CONFIG_BOOT_0B_GPIO_16_18_INIT_GPIO17_PU_FIELD, 0);
    value = field_update16(value, CONFIG_BOOT_0B_GPIO_16_18_INIT_GPIO18_PU_FIELD, 0);
    value = field_update16(value, CONFIG_BOOT_0B_GPIO_16_18_INIT_GPIO5_ANALOG_EN_FIELD, 1);
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_WRITE_16, value, NULL)) < 0)
        return (err * (1 << 16)) + count;

    // Workaround for Erratum 3: 'Bootloader GPIO Reconfiguration'
    // Modifying IOMATRIX.PIN_7_0
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_BANK, 0x00000000, NULL)) < 0)
        return (err * (1 << 16)) + count;
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_ADDRESS, IOMATRIX_PIN_7_0, NULL)) < 0)
        return (err * (1 << 16)) + count;
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_READ_32, 0, &value)) < 0)
        return (err * (1 << 16)) + count;
    value = field_update32(value, IOMATRIX_PIN_7_0_ALTF5_FIELD, 0);
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_WRITE_32, value, NULL)) < 0)
        return (err * (1 << 16)) + count;

    // Modifying IOMATRIX.PIN_18_16
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_ADDRESS, IOMATRIX_PIN_18_16, NULL)) < 0)
        return (err * (1 << 16)) + count;
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_READ_32, 0, &value)) < 0)
        return (err * (1 << 16)) + count;
    value = field_update32(value, IOMATRIX_PIN_18_16_ALTF17_FIELD, 0);
    value = field_update32(value, IOMATRIX_PIN_18_16_ALTF18_FIELD, 0);
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_WRITE_32, value, NULL)) < 0)
        return (err * (1 << 16)) + count;

    // Modifying CONFIG.BOOT_03_BOOT_INTERFACE
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_BANK, 0x00000005, NULL)) < 0)
        return (err * (1 << 16)) + count;
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_ADDRESS, CONFIG_BOOT_03_INTERFACE, NULL)) < 0)
        return (err * (1 << 16)) + count;
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_READ_16, 0, &value)) < 0)
        return (err * (1 << 16)) + count;
    value = field_update16(value, CONFIG_BOOT_03_INTERFACE_BL_DISABLE_UART_FIELD, 0);
    value = field_update16(value, CONFIG_BOOT_03_INTERFACE_BL_DISABLE_SPI_FIELD, 0);
    value = field_update16(value, CONFIG_BOOT_03_INTERFACE_BL_SPI_SELECT_FIELD, 0);
    value = field_update16(value, CONFIG_BOOT_03_INTERFACE_BL_UART_RX_FIELD, 0);
    value = field_update16(value, CONFIG_BOOT_03_INTERFACE_BL_UART_TX_FIELD, 0);
    value = field_update16(value, CONFIG_BOOT_03_INTERFACE_BL_UART_BAUDRATE_FIELD, 7);
    value = field_update16(value, CONFIG_BOOT_03_INTERFACE_BL_SPI0_SCK_FIELD, 1);
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_WRITE_16, value, NULL)) < 0)
        return (err * (1 << 16)) + count;
    tmc_delayMicroseconds(1000);

    return 0;
}
// -----------------------------------------------------------------------------
// End of autogenerated code
// ---------------------------------------------------------------------

void setup()
{
    Serial.begin(9600);

    pinMode(IC1_FAULTN_STATUS, INPUT);
    pinMode(IC2_FAULTN_STATUS, INPUT);

    if (activeBus == TMC9660_BUS_UART)
    {
        Serial3.begin(115200);
        delay(10);
        pinMode(IC1_HOLDN_FLASH, OUTPUT);
        digitalWrite(IC1_HOLDN_FLASH, HIGH);
    }
    else if (activeBus == TMC9660_BUS_SPI)
    {

        pinMode(CS1, OUTPUT);
        digitalWrite(CS1, HIGH);

        pinMode(CS2, OUTPUT);
        digitalWrite(CS2, HIGH);

        SPI.begin();
        SPI.beginTransaction(SPISettings(6000000, MSBFIRST, SPI_MODE3)); // 6 MHz

        // HACK
        SPI.transfer(0xAA);

        pinMode(IC1_HOLDN_FLASH, OUTPUT);
        digitalWrite(IC1_HOLDN_FLASH, LOW);

        pinMode(IC2_HOLDN_FLASH, OUTPUT);
        digitalWrite(IC2_HOLDN_FLASH, LOW);
    }

    pinMode(IC1_RESET_CTRL, OUTPUT);
    digitalWrite(IC1_RESET_CTRL, HIGH);
    delay(10);

    digitalWrite(IC1_RESET_CTRL, LOW);
    delay(500);
    pinMode(IC2_RESET_CTRL, OUTPUT);
    digitalWrite(IC2_RESET_CTRL, HIGH);
    delay(10);

    digitalWrite(IC2_RESET_CTRL, LOW);

    uint32_t id = 0, version = 0;
    int32_t err;
    uint32_t value = 0;

    if (IC1_ENABLE)
    {
        delay(500);
        Serial.println("\nStarted writing config...");
        tmc9660_loadBootConfig(IC_1);
        Serial.println("Finished writing config!");

        delay(100);
        Serial.println("Uploading Addon!");
        int32_t err1 = tmc9660_bl_installAddon_SPI_v0_6(IC_1);
        Serial.print("Addon uploaded on IC_1 with status: ");
        Serial.println(err1);
        delay(100);

        err = tmc9660_bl_getAddonInfo(IC_1, &id, &version);
        Serial.print("IC_1 Addon id: ");
        Serial.println(id);
        Serial.print("Addon version: ");
        Serial.println(version);

        // IC_1
        //Starting the App!
        tmc9660_bl_sendCommand(IC_1, TMC9660_BLCMD_SET_BANK, 0x00000005, NULL);
        tmc9660_bl_sendCommand(IC_1, TMC9660_BLCMD_SET_ADDRESS, 0x00020008, NULL);
        tmc9660_bl_sendCommand(IC_1, TMC9660_BLCMD_READ_16, 0, &value);
        value = field_update16(value, CONFIG_BOOT_04_BOOTSTRAP_LOAD_ROM_CODE_FIELD, 1);
        tmc9660_bl_sendCommand(IC_1, TMC9660_BLCMD_WRITE_16, value, NULL);
        tmc9660_waitForFaultDeassertion(IC_1, 1000);
    }

    if (IC2_ENABLE)
    {
        delay(500);
        Serial.println("\nStarted writing config...");
        tmc9660_loadBootConfig(IC_2);
        Serial.println("Finished writing config!");

        delay(100);
        Serial.println("Uploading Addon!");
        int32_t err2 = tmc9660_bl_installAddon_SPI_v0_6(IC_2);
        Serial.print("Addon uploaded on IC_2 with status: ");
        Serial.println(err2);
        delay(100);

        err = tmc9660_bl_getAddonInfo(IC_2, &id, &version);
        Serial.print("IC_2 Addon id: ");
        Serial.println(id);
        Serial.print("Addon version: ");
        Serial.println(version);

        // IC_2
        tmc9660_bl_sendCommand(IC_2, TMC9660_BLCMD_SET_BANK, 0x00000005, NULL);
        tmc9660_bl_sendCommand(IC_2, TMC9660_BLCMD_SET_ADDRESS, 0x00020008, NULL);
        tmc9660_bl_sendCommand(IC_2, TMC9660_BLCMD_READ_16, 0, &value);
        value = field_update16(value, CONFIG_BOOT_04_BOOTSTRAP_LOAD_ROM_CODE_FIELD, 1);
        tmc9660_bl_sendCommand(IC_2, TMC9660_BLCMD_WRITE_16, value, NULL);
        tmc9660_waitForFaultDeassertion(IC_2, 1000);
    }

    Serial.println("App Started!");
}
void loop()
{

    String inputString = "";

    // Open the serial monitor and enter a number from 1 to 5.
    // Case 1: Print the module ID of both slave ICs.
    // Case 2: Communicate with IC2 using basic SPI pipelining.
    // Case 3: Communicate with IC1 using simple access.
    // Case 4: Communicate with IC1 and IC2 alternately using pipeline access.
    // Case 5: Communicate with IC1 and IC2 using simple access.

    if (Serial.available())
    {
        inputString = Serial.readStringUntil('\n');
        if (inputString == "1")
        {
            Serial.println("read module id");
            uint32_t val   = 0;
            int32_t status = tmc9660_param_sendCommand(IC_1, 136, 1, 0, 0, &val);

            Serial.print("Module ID IC_1: ");
            Serial.println(val >> 16);

            Serial.print("Status: ");
            Serial.println(status);

            status = tmc9660_param_sendCommand(IC_2, 136, 1, 0, 0, &val);

            Serial.print("Module ID IC_2: ");
            Serial.println(val >> 16);

            Serial.print("Status: ");
            Serial.println(status);
        }
        else if (inputString == "2")
        {
            // Implementing basic pipelined access with the TMC-API (Error handling of each TMC-API call omitted for simplicity):
            uint32_t replyValue = 0;

            uint32_t startTime = micros();

            // Request 1: GET_INFO MODULE_ID
            tmc9660_param_sendPipelinedSPICommand(IC_2, 157, 0, 0, 0, &replyValue, false, 0);
            // Request 2: GET_INFO VERSION. Reply of Request 1 is received here.
            tmc9660_param_sendPipelinedSPICommand(IC_2, 157, 1, 0, 0, &replyValue, true, 1000);
            // Request 3: GET_VERSION HEX. Reply of Request 2 is received here.
            tmc9660_param_sendPipelinedSPICommand(IC_2, 136, 1, 0, 0, &replyValue, true, 1000);
            // Request 4: NOP command to retrieve the final reply. Reply of Request 3 is received here.
            tmc9660_param_sendPipelinedSPICommand(IC_2, 0xFF, 0, 0, 0, &replyValue, true, 1000);

            uint32_t endTime = micros();

            Serial.print("Basic Pipelined Access Execution time (us): ");
            Serial.println(endTime - startTime);
        }
        else if (inputString == "3")
        {
            // Implementing Simple access without pipelining (Error handling of each TMC-API call omitted for simplicity):
            uint32_t replyValue = 0;

            uint32_t startTime = micros();
            // Request 1: GET_INFO MODULE_ID.
            tmc9660_param_sendCommand(IC_1, 157, 0, 0, 0, &replyValue);
            // Request 2: GET_INFO VERSION
            tmc9660_param_sendCommand(IC_1, 157, 1, 0, 0, &replyValue);
            // Request 3: GET_VERSION HEX
            tmc9660_param_sendCommand(IC_1, 136, 1, 0, 0, &replyValue);

            uint32_t endTime = micros();

            Serial.print("Simple Access Execution time (us): ");
            Serial.println(endTime - startTime);
        }
        else if (inputString == "4")
        {
            // Implementing basic pipelined access for accessing multiple slave
            uint32_t replyValue = 0;

            uint32_t startTime = micros();

            // Request 1: GET_INFO MODULE_ID
            tmc9660_param_sendPipelinedSPICommand(IC_1, 157, 0, 0, 0, &replyValue, false, 0);
            tmc9660_param_sendPipelinedSPICommand(IC_2, 157, 0, 0, 0, &replyValue, false, 0);

            // Request 2: GET_INFO VERSION. Reply of Request 1 is received here.
            tmc9660_param_sendPipelinedSPICommand(IC_1, 157, 1, 0, 0, &replyValue, true, 1000);
            tmc9660_param_sendPipelinedSPICommand(IC_2, 157, 1, 0, 0, &replyValue, true, 1000);

            // Request 3: GET_VERSION HEX. Reply of Request 2 is received here.
            tmc9660_param_sendPipelinedSPICommand(IC_1, 136, 1, 0, 0, &replyValue, true, 1000);
            tmc9660_param_sendPipelinedSPICommand(IC_2, 136, 1, 0, 0, &replyValue, true, 1000);

            // Request 4: NOP command to retrieve the final reply. Reply of Request 3 is received here.
            tmc9660_param_sendPipelinedSPICommand(IC_1, 0xFF, 0, 0, 0, &replyValue, true, 1000);
            tmc9660_param_sendPipelinedSPICommand(IC_2, 0xFF, 0, 0, 0, &replyValue, true, 1000);

            uint32_t endTime = micros();

            Serial.print("Basic Pipelined Access Execution time(us) with multi slave: ");
            Serial.println(endTime - startTime);
        }
        else if (inputString == "5")
        {
            // Implementing Simple access without pipelining for accessing multiple slave
            uint32_t replyValue = 0;

            uint32_t startTime = micros();

            //-------------------------IC_1-----------------------

            // Request 1: GET_INFO MODULE_ID
            tmc9660_param_sendCommand(IC_1, 157, 0, 0, 0, &replyValue);

            // Request 2: GET_INFO VERSION
            tmc9660_param_sendCommand(IC_1, 157, 1, 0, 0, &replyValue);

            // Request 3: GET_VERSION HEX
            tmc9660_param_sendCommand(IC_1, 136, 1, 0, 0, &replyValue);

            //-------------------------IC_2-----------------------

            // Request 1: GET_INFO MODULE_ID
            tmc9660_param_sendCommand(IC_2, 157, 0, 0, 0, &replyValue);

            // Request 2: GET_INFO VERSION
            tmc9660_param_sendCommand(IC_2, 157, 1, 0, 0, &replyValue);

            // Request 3: GET_VERSION HEX
            tmc9660_param_sendCommand(IC_2, 136, 1, 0, 0, &replyValue);

            uint32_t endTime = micros();

            Serial.print("Simple Access Execution time (us) with multi slave: ");
            Serial.println(endTime - startTime);
        }
        inputString = "";
    }
}
