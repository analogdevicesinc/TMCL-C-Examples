/*******************************************************************************
* Copyright © 2025 Analog Devices, Inc.
*******************************************************************************/


#include "TMC9660.h"

#define SPI_DEFAULT_DELAY 100 // [us]

#define TMC9660_ADDON_MIN_SIZE 48

// An addon installation has this address set to this key
#define TMC9660_ADDON_KEY_ADDR 0x00020C00
#define TMC9660_ADDON_KEY      0x6B657921
// An installed addon has its ID and version located at this address:
#define TMC9660_ADDON_METADATA_ADDR 0x0002BFF8

// ToDo: Make the timing function & callback usable with multiple TMC-API chips in use.
void tmc_delayMicroseconds(uint32_t microseconds)
{
    uint32_t timestamp = tmc_getMicrosecondTimestamp();

    while (tmc_getMicrosecondTimestamp() - timestamp < microseconds);
}

#if TMC_API_EXTERNAL_CRC_TABLE
extern const uint8_t tmcCRCTable_Poly7Reflected[256];
extern const uint32_t tmcCRCTable_Poly104C11DB7Reflected[256];
#else
const uint8_t tmcCRCTable_Poly7Reflected[256] = {
            0x00, 0x91, 0xE3, 0x72, 0x07, 0x96, 0xE4, 0x75, 0x0E, 0x9F, 0xED, 0x7C, 0x09, 0x98, 0xEA, 0x7B,
            0x1C, 0x8D, 0xFF, 0x6E, 0x1B, 0x8A, 0xF8, 0x69, 0x12, 0x83, 0xF1, 0x60, 0x15, 0x84, 0xF6, 0x67,
            0x38, 0xA9, 0xDB, 0x4A, 0x3F, 0xAE, 0xDC, 0x4D, 0x36, 0xA7, 0xD5, 0x44, 0x31, 0xA0, 0xD2, 0x43,
            0x24, 0xB5, 0xC7, 0x56, 0x23, 0xB2, 0xC0, 0x51, 0x2A, 0xBB, 0xC9, 0x58, 0x2D, 0xBC, 0xCE, 0x5F,
            0x70, 0xE1, 0x93, 0x02, 0x77, 0xE6, 0x94, 0x05, 0x7E, 0xEF, 0x9D, 0x0C, 0x79, 0xE8, 0x9A, 0x0B,
            0x6C, 0xFD, 0x8F, 0x1E, 0x6B, 0xFA, 0x88, 0x19, 0x62, 0xF3, 0x81, 0x10, 0x65, 0xF4, 0x86, 0x17,
            0x48, 0xD9, 0xAB, 0x3A, 0x4F, 0xDE, 0xAC, 0x3D, 0x46, 0xD7, 0xA5, 0x34, 0x41, 0xD0, 0xA2, 0x33,
            0x54, 0xC5, 0xB7, 0x26, 0x53, 0xC2, 0xB0, 0x21, 0x5A, 0xCB, 0xB9, 0x28, 0x5D, 0xCC, 0xBE, 0x2F,
            0xE0, 0x71, 0x03, 0x92, 0xE7, 0x76, 0x04, 0x95, 0xEE, 0x7F, 0x0D, 0x9C, 0xE9, 0x78, 0x0A, 0x9B,
            0xFC, 0x6D, 0x1F, 0x8E, 0xFB, 0x6A, 0x18, 0x89, 0xF2, 0x63, 0x11, 0x80, 0xF5, 0x64, 0x16, 0x87,
            0xD8, 0x49, 0x3B, 0xAA, 0xDF, 0x4E, 0x3C, 0xAD, 0xD6, 0x47, 0x35, 0xA4, 0xD1, 0x40, 0x32, 0xA3,
            0xC4, 0x55, 0x27, 0xB6, 0xC3, 0x52, 0x20, 0xB1, 0xCA, 0x5B, 0x29, 0xB8, 0xCD, 0x5C, 0x2E, 0xBF,
            0x90, 0x01, 0x73, 0xE2, 0x97, 0x06, 0x74, 0xE5, 0x9E, 0x0F, 0x7D, 0xEC, 0x99, 0x08, 0x7A, 0xEB,
            0x8C, 0x1D, 0x6F, 0xFE, 0x8B, 0x1A, 0x68, 0xF9, 0x82, 0x13, 0x61, 0xF0, 0x85, 0x14, 0x66, 0xF7,
            0xA8, 0x39, 0x4B, 0xDA, 0xAF, 0x3E, 0x4C, 0xDD, 0xA6, 0x37, 0x45, 0xD4, 0xA1, 0x30, 0x42, 0xD3,
            0xB4, 0x25, 0x57, 0xC6, 0xB3, 0x22, 0x50, 0xC1, 0xBA, 0x2B, 0x59, 0xC8, 0xBD, 0x2C, 0x5E, 0xCF,
};

const uint32_t tmcCRCTable_Poly104C11DB7Reflected[256] = {

        0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
        0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
        0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
        0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
        0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
        0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
        0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
        0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
        0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
        0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
        0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
        0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
        0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
        0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
        0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
        0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
        0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
        0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
        0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
        0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
        0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
        0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
        0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
        0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
        0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
        0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
        0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
        0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
        0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
        0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
        0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
        0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D,
};

#endif

// Helper functions
static int32_t tmc9660_bl_sendCommand_SPI(uint16_t icID, uint8_t cmd, uint32_t writeValue, uint32_t *readValue, uint32_t delay);
static int32_t tmc9660_bl_sendCommand_UART(uint16_t icID, uint8_t cmd, uint32_t writeValue, uint32_t *readValue);
static int32_t tmc9660_param_sendCommand_SPI(uint16_t icID, uint8_t cmd, uint16_t type, uint8_t index, uint32_t writeValue, uint32_t *readValue);
static int32_t tmc9660_param_spiSingleRequest(uint16_t icID, uint8_t cmd, uint16_t type, uint8_t index, uint32_t writeValue, uint32_t *readValue, bool poll, uint32_t timeout_us);
static int32_t tmc9660_param_getVersionASCII_SPI(uint16_t icID, uint8_t *versionString);
static int32_t tmc9660_param_returnToBootloader_SPI(uint16_t icID);
static int32_t tmc9660_param_sendCommand_UART(uint16_t icID, uint8_t cmd, uint16_t type, uint8_t index, uint32_t writeValue, uint32_t *readValue);
static int32_t tmc9660_param_getVersionASCII_UART(uint16_t icID, uint8_t *versionString);
static int32_t tmc9660_param_readTMCLMemory_UART(uint16_t icID, uint32_t cmdIndex, uint8_t *command);
static int32_t tmc9660_param_returnToBootloader_UART(uint16_t icID);
static int32_t tmc9660_reg_sendCommand_UART(uint16_t icID, uint8_t cmd, uint16_t registerOffset, uint8_t registerBlock, uint32_t writeValue, uint32_t *readValue);

static uint8_t calcParamChecksum(uint8_t *data, uint32_t bytes);
static uint8_t CRC8(uint8_t *data, uint32_t bytes);

/*** General functions implementation ********************************************/
#if TMC_API_TMC9660_FAULT_PIN_SUPPORTED != 0
bool tmc9660_waitForFaultDeassertion(uint16_t icID, uint32_t timeout_us)
{
    uint32_t timestamp = 0;
    if (timeout_us != 0)
    {
        timestamp = tmc_getMicrosecondTimestamp();
    }

    while (tmc9660_isFaultPinAsserted(icID))
    {
        if (timeout_us == 0)
            continue;

        // Timeout reached? If yes, return failure
        if ((tmc_getMicrosecondTimestamp() - timestamp) > timeout_us)
            return false;
    }

    // Fault deassertion completed, return success
    return true;
}
#endif

/*** Bootstrapping code implementation *******************************************/

int32_t tmc9660_bl_sendCommand(uint16_t icID, uint8_t cmd, uint32_t writeValue, uint32_t *readValue)
{
    TMC9660BusType bus = tmc9660_getBusType(icID);

    if(bus == TMC9660_BUS_SPI)
    {
        return tmc9660_bl_sendCommand_SPI(icID, cmd, writeValue, readValue, SPI_DEFAULT_DELAY);
    }
    else if(bus == TMC9660_BUS_UART)
    {
        return tmc9660_bl_sendCommand_UART(icID, cmd, writeValue, readValue);
    }

    return TMC9660_ERROR_INVALID_BUS;
}

static int32_t tmc9660_bl_sendCommand_SPI(uint16_t icID, uint8_t cmd, uint32_t writeValue, uint32_t *readValue, uint32_t delay)
{
    uint8_t data[5] = { 0 };

    data[0] = cmd;  // Command
    data[1] = (writeValue >> 24) & 0xFF;
    data[2] = (writeValue >> 16) & 0xFF;
    data[3] = (writeValue >> 8 ) & 0xFF;
    data[4] = (writeValue      ) & 0xFF;

    // Send the request
    tmc9660_readWriteSPI(icID, &data[0], 5, false);
    tmc_delayMicroseconds(delay);

    if (readValue)
    {
        data[0] = TMC9660_BLCMD_NO_OP;
        tmc9660_readWriteSPI(icID, &data[0], 5, false);
        tmc_delayMicroseconds(SPI_DEFAULT_DELAY);

        *readValue = ((uint32_t) data[1] << 24)
                   | ((uint32_t) data[2] << 16)
                   | ((uint32_t) data[3] << 8)
                   | ((uint32_t) data[4]);

        return data[0];
    }

    return data[0];
}

static int32_t tmc9660_bl_sendCommand_UART(uint16_t icID, uint8_t cmd, uint32_t writeValue, uint32_t *readValue)
{
    uint8_t data[8] = { 0 };
    TMC9660BusAddresses addresses = tmc9660_getBusAddresses(icID);

    data[0] = 0x55;       // Sync byte
    data[1] = 0x01 | (addresses.device); // Device Address
    data[2] = cmd;  // Command
    data[3] = (writeValue >> 24) & 0xFF;
    data[4] = (writeValue >> 16) & 0xFF;
    data[5] = (writeValue >> 8 ) & 0xFF;
    data[6] = (writeValue      ) & 0xFF;
    data[7] = CRC8(data, 7);

    if (!tmc9660_readWriteUART(icID, &data[0], 8, 8)) {
      return TMC9660_ERROR_INVALID_BUS;
    }

    if (readValue)
    {
        *readValue = ((uint32_t) data[3] << 24)
                   | ((uint32_t) data[4] << 16)
                   | ((uint32_t) data[5] << 8)
                   | ((uint32_t) data[6]);
    }

    // Workaround: Wait a short moment before proceeding
    tmc_delayMicroseconds(10);

    return data[2];
}

static uint8_t CRC8(uint8_t *data, uint32_t bytes)
{
    uint8_t result = 0;
    while(bytes--)
        result = tmcCRCTable_Poly7Reflected[result ^ *data++];

    // Flip the result around
    // swap odd and even bits
    result = ((result >> 1) & 0x55) | ((result & 0x55) << 1);
    // swap consecutive pairs
    result = ((result >> 2) & 0x33) | ((result & 0x33) << 2);
    // swap nibbles ...
    result = ((result >> 4) & 0x0F) | ((result & 0x0F) << 4);

    return result;
}

static uint32_t CRC32(const uint8_t *data, uint32_t bytes)
{
    uint32_t result;

    result = 0xFFFFFFFF;
    while (bytes--)
    {
        result = tmcCRCTable_Poly104C11DB7Reflected[(result ^ *data++) & 0xFF] ^ (result >> 8);
    }

    return result ^ 0xFFFFFFFF;
}

int32_t tmc9660_bl_installAddon(uint16_t icID, const uint8_t *addon, uint32_t addonSize)
{
    uint32_t count = 0;

    if (addonSize < TMC9660_ADDON_MIN_SIZE)
        return (TMC9660_ERROR_INVALID_ADDON * (1<<16)) + count;
    if (addonSize % 8 != 0)
        return (TMC9660_ERROR_INVALID_ADDON * (1<<16)) + count;

    TMC9660BusType bus = tmc9660_getBusType(icID);

    uint32_t value = 0;
    int32_t err = 0;

    // Check that the addon fits into SRAM
    // Note: First command - we got to allow SESSION_START and BOOTLOADER_RESUMED status here
    (count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_GET_INFO, 6, &value));
    if (err != 0 && err != 19 && err != 21)
        return (err * (1<<16)) + count;

    if (value < addonSize)
        return (TMC9660_ERROR_NO_ADDON_SPACE * (1<<16)) + count;

    // Grab the start of the SRAM region
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_GET_INFO, 5, &value)) != 0)
        return (err * (1<<16)) + count;

    uint32_t startAddr = value;

    // Select the SRAM
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_BANK, 0, NULL)) != 0)
        return (err * (1<<16)) + count;

    // Set the upload start address (skipping the addon's length and checksum bytes)
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_ADDRESS, startAddr + 8, NULL)) != 0)
        return (err * (1<<16)) + count;

    for (size_t i = 8; i < addonSize; i+= 4)
    {
        value = ((uint32_t) addon[i])
              | ((uint32_t) addon[i+1] << 8)
              | ((uint32_t) addon[i+2] << 16)
              | ((uint32_t) addon[i+3] << 24);

        if (bus == TMC9660_BUS_SPI)
        {
            // The RAM WRITE_32_INC commands don't take that long, 15µs delay is sufficient.
            if ((count++, err = tmc9660_bl_sendCommand_SPI(icID, TMC9660_BLCMD_WRITE_32_INC, value, NULL, 15)) != 0)
                return (err * (1<<16)) + count;
        }
        else
        {
            if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_WRITE_32_INC, value, NULL)) != 0)
                return (err * (1<<16)) + count;
        }
    }

    // Set the addon start address
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_ADDRESS, startAddr, NULL)) != 0)
        return (err * (1<<16)) + count;

    // Write the addon length
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_WRITE_32, addonSize, NULL)) != 0)
        return (err * (1<<16)) + count;

    // Write the addon checksum - for SPI we need extra delay here
    uint32_t checksum = CRC32(&addon[8], addonSize-8);

    // Both the checksum writing and the start command take slightly longer than
    // other commands. For UART this has no special impact, as the protocol
    // naturally waits for the response. For SPI we have to inject a small
    // delay to ensure we don't act too fast.

    if (bus == TMC9660_BUS_SPI)
    {
        // We need more delay here - roughly 0.375µs / byte
        uint32_t delay = SPI_DEFAULT_DELAY + (addonSize / 8)*3;

        // Even though we don't check the value here, we still request it.
        // causes the underlying SPI function to actually read back the status of the command.
        if ((count++, err = tmc9660_bl_sendCommand_SPI(icID, TMC9660_BLCMD_WRITE_CHECKSUM, checksum, &value, delay)) != 0)
            return (err * (1<<16)) + count;

        // Start the addon installation on the TMC9660
        if ((count++, err = tmc9660_bl_sendCommand_SPI(icID, TMC9660_BLCMD_START_APP, 0, NULL, delay)) != 0)
            return (err * (1<<16)) + count;
    }
    else
    {
        // Write the checksum
        if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_WRITE_CHECKSUM, checksum, NULL)) != 0)
            return (err * (1<<16)) + count;

        // Start the addon installation on the TMC9660
        if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_START_APP, 0, NULL)) != 0)
            return (err * (1<<16)) + count;
    }

    return 0;
}

int32_t tmc9660_bl_getAddonInfo(uint16_t icID, uint32_t *id, uint32_t *version)
{
    uint32_t count = 0;
    int32_t err;
    uint32_t value;

    // Select the SRAM
    // Note: First command - we got to allow SESSION_START and BOOTLOADER_RESUMED status here
    (count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_BANK, 0, NULL));
    if (err != 0 && err != 19 && err != 21)
        return (err * (1<<16)) + count;

    // Check for the presence of an addon by checking the addon key
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_ADDRESS, TMC9660_ADDON_KEY_ADDR, NULL)) != 0)
        return (err * (1<<16)) + count;

    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_READ_32, 0, &value)) != 0)
        return (err * (1<<16)) + count;

    if (value != TMC9660_ADDON_KEY)
    {
        // Addon not present
        return 0;
    }

    // Addon is present, read out the metadata
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_SET_ADDRESS, TMC9660_ADDON_METADATA_ADDR, NULL)) != 0)
        return (err * (1<<16)) + count;

    // Readout the ID
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_READ_32_INC, 0, id)) != 0)
        return (err * (1<<16)) + count;

    // Readout the version
    if ((count++, err = tmc9660_bl_sendCommand(icID, TMC9660_BLCMD_READ_32, 0, version)) != 0)
        return (err * (1<<16)) + count;

    return 1;
}


/*** Command sending implementation *******************************************/


int32_t tmc9660_param_sendCommand(uint16_t icID, uint8_t cmd, uint16_t type, uint8_t index, uint32_t writeValue, uint32_t *readValue)
{
    TMC9660BusType bus = tmc9660_getBusType(icID);

    if(bus == TMC9660_BUS_SPI)
    {
        return tmc9660_param_sendCommand_SPI(icID, cmd, type, index, writeValue, readValue);
    }
    else if(bus == TMC9660_BUS_UART)
    {
        return tmc9660_param_sendCommand_UART(icID, cmd, type, index, writeValue, readValue);
    }

    return TMC9660_ERROR_INVALID_BUS;
}

int32_t tmc9660_param_getVersionASCII(uint16_t icID, uint8_t *versionString)
{
    TMC9660BusType bus = tmc9660_getBusType(icID);

    if(bus == TMC9660_BUS_SPI)
    {
        return tmc9660_param_getVersionASCII_SPI(icID, versionString);
    }
    else if(bus == TMC9660_BUS_UART)
    {
        return tmc9660_param_getVersionASCII_UART(icID, versionString);
    }

    return TMC9660_ERROR_INVALID_BUS;
}

int32_t tmc9660_param_readTMCLMemory(uint16_t icID, uint32_t cmdIndex, uint8_t *command)
{
    TMC9660BusType bus = tmc9660_getBusType(icID);

    if(bus == TMC9660_BUS_SPI)
    {
        // This special-case command does not work over SPI
        return TMC9660_ERROR_INVALID_BUS;
    }
    else if(bus == TMC9660_BUS_UART)
    {
        return tmc9660_param_readTMCLMemory_UART(icID, cmdIndex, command);
    }

    return TMC9660_ERROR_INVALID_BUS;
}

int32_t tmc9660_param_returnToBootloader(uint16_t icID)
{
    TMC9660BusType bus = tmc9660_getBusType(icID);

    if(bus == TMC9660_BUS_SPI)
    {
        return tmc9660_param_returnToBootloader_SPI(icID);
    }
    else if(bus == TMC9660_BUS_UART)
    {
        return tmc9660_param_returnToBootloader_UART(icID);
    }

    return TMC9660_ERROR_INVALID_BUS;
}

static bool sendRequestUART(uint16_t icID, uint8_t cmd, uint16_t type, uint8_t index, uint32_t writeValue, uint8_t *data, TMC9660BusAddresses addresses, bool expectReply)
{
    // Create the request datagram
    uint8_t syncByte = 0x01 | (addresses.device);
    data[0] = syncByte; // Module Address & sync bit
    data[1] = cmd;
    data[2] = type & 0xFF;
    data[3] = (type >> 8) << 4 | (index & 0x0F); // ToDo: Do we wanna do the 4/12 split at this function's arguments?
    data[4] = (writeValue >> 24) & 0xFF;
    data[5] = (writeValue >> 16) & 0xFF;
    data[6] = (writeValue >> 8) & 0xFF;
    data[7] = (writeValue) & 0xFF;
    data[8] = calcParamChecksum(&data[0], 8);

    return tmc9660_readWriteUART(icID, &data[0], 9, (expectReply)? 9:0);
}

static int32_t tmc9660_param_sendCommand_SPI(uint16_t icID, uint8_t cmd, uint16_t type, uint8_t index, uint32_t writeValue, uint32_t *readValue)
{
    // Send the command request
    tmc9660_param_spiSingleRequest(icID, cmd, type, index, writeValue, readValue, false, 0);

    // Get the reply with a NOOP request with a 10ms (10000µs) timeout
    return tmc9660_param_spiSingleRequest(icID, 0xFF, 0, 0, 0, readValue, true, 10*1000);
}

static bool sendRequestSPI(uint16_t icID, uint8_t cmd, uint16_t type, uint8_t index, uint32_t writeValue, uint8_t *data, bool poll, uint32_t timeout_us)
{
    uint32_t timestamp;
    if (timeout_us)
    {
        timestamp = tmc_getMicrosecondTimestamp();
    }

    if (poll)
    {
        do {
            // Fast polling with single byte requests

            if (TMC_API_TMC9660_ENABLE_SPI_POLL_RESUMING)
            {
                // Zero-sized request to de-assert chip select
                tmc9660_readWriteSPI(icID, &data[0], 0, false);

                // Do a request for just one byte and keep the SPI transaction going
                data[0] = cmd;
                tmc9660_readWriteSPI(icID, &data[0], 1, true);
            }
            else
            {
                // Do a full request of just one byte
                data[0] = cmd;
                tmc9660_readWriteSPI(icID, &data[0], 1, false);
            }

            if (timeout_us && (tmc_getMicrosecondTimestamp() - timestamp) > timeout_us)
            {
                if (TMC_API_TMC9660_ENABLE_SPI_POLL_RESUMING)
                {
                    // Zero-sized request to de-assert chip select
                    tmc9660_readWriteSPI(icID, &data[0], 0, false);
                }

                // Report timeout error
                return false;
            }
        } while (data[0] == TMC9660_PARAMSPISTATUS_NOT_READY);
    }

    // Construct the request datagram
    // data[0] is managed separately for fast poll support
    data[1] = type & 0xFF;
    data[2] = (type >> 8) << 4 | (index & 0x0F);
    data[3] = (writeValue >> 24) & 0xFF;
    data[4] = (writeValue >> 16) & 0xFF;
    data[5] = (writeValue >> 8)  & 0xFF;
    data[6] = (writeValue)       & 0xFF;
    // The checksum calculation must manually incorporate cmd
    // because we're skipping data[0] here.
    data[7] = calcParamChecksum(&data[1], 6) + cmd;

    // Complete the SPI transaction
    if (TMC_API_TMC9660_ENABLE_SPI_POLL_RESUMING && poll)
    {
        // The first byte is already transmitted, send the rest
        tmc9660_readWriteSPI(icID, &data[1], 7, false);
    }
    else
    {
        // Send the full request
        data[0] = cmd;
        tmc9660_readWriteSPI(icID, &data[0], 8, false);
    }

    return true;
}

static int32_t tmc9660_param_spiSingleRequest(uint16_t icID, uint8_t cmd, uint16_t type, uint8_t index, uint32_t writeValue, uint32_t *readValue, bool poll, uint32_t timeout_us)
{
    uint8_t data[8] = { 0 };

    if (!sendRequestSPI(icID, cmd, type, index, writeValue, &data[0], poll, timeout_us))
        return TMC9660_ERROR_TIMEOUT;

    if (calcParamChecksum(&data[0], 7) != data[7])
        return TMC9660_ERROR_INVALID_CHECKSUM;

    if (readValue)
    {
        *readValue = ((uint32_t) data[3] << 24)
                   | ((uint32_t) data[4] << 16)
                   | ((uint32_t) data[5] << 8)
                   | ((uint32_t) data[6]);
    }

    return data[1];
}

static int32_t tmc9660_param_sendCommand_UART(uint16_t icID, uint8_t cmd, uint16_t type, uint8_t index, uint32_t writeValue, uint32_t *readValue)
{
    uint8_t data[9] = { 0 };
    TMC9660BusAddresses addresses = tmc9660_getBusAddresses(icID);

    if (!sendRequestUART(icID, cmd, type, index, writeValue, data, addresses, true))
        return TMC9660_ERROR_TIMEOUT;

    uint8_t syncByte = 0x01 | (addresses.device);

    // Unpack the reply
    if (data[0] != addresses.host)
        return TMC9660_ERROR_WRONG_ADDR;
    if (data[1] != syncByte)
        return TMC9660_ERROR_INVALID_REPLY;
    if (data[8] != calcParamChecksum(&data[0], 8))
        return TMC9660_ERROR_INVALID_CHECKSUM;

    if (readValue)
    {
        *readValue = ((uint32_t) data[4] << 24)
                   | ((uint32_t) data[5] << 16)
                   | ((uint32_t) data[6] << 8)
                   | ((uint32_t) data[7]);
    }

    return data[2];
}

int32_t tmc9660_param_sendPipelinedSPICommand(uint16_t icID, uint8_t cmd, uint16_t type, uint8_t index, uint32_t requestValue, uint32_t *replyValue, bool requireReply, uint32_t timeout_us)
{
    return tmc9660_param_spiSingleRequest(icID, cmd, type, index, requestValue, replyValue, requireReply, timeout_us);
}

static int32_t tmc9660_param_getVersionASCII_UART(uint16_t icID, uint8_t *versionString)
{
    uint8_t data[9] = { 0 };
    TMC9660BusAddresses addresses = tmc9660_getBusAddresses(icID);

    if (!sendRequestUART(icID, TMC9660_CMD_GET_VERSION, 0, 0, 0, data, addresses, true))
        return TMC9660_ERROR_TIMEOUT;

    versionString[0] = data[1];
    versionString[1] = data[2];
    versionString[2] = data[3];
    versionString[3] = data[4];
    versionString[4] = data[5];
    versionString[5] = data[6];
    versionString[6] = data[7];
    versionString[7] = data[8];

    return 0;
}

static int32_t tmc9660_param_readTMCLMemory_UART(uint16_t icID, uint32_t cmdIndex, uint8_t *command)
{
    uint8_t data[9] = { 0 };
    TMC9660BusAddresses addresses = tmc9660_getBusAddresses(icID);

    if (!sendRequestUART(icID, TMC9660_CMD_READ_MEM, 0, 0, cmdIndex, data, addresses, true))
        return TMC9660_ERROR_TIMEOUT;

    command[0] = data[1];
    command[1] = data[2];
    command[2] = data[3];
    command[3] = data[4];
    command[4] = data[5];
    command[5] = data[6];
    command[6] = data[7];

    return 0;
}

static int32_t tmc9660_param_returnToBootloader_UART(uint16_t icID)
{
    uint8_t data[9] = { 0 };
    TMC9660BusAddresses addresses = tmc9660_getBusAddresses(icID);

    if (!sendRequestUART(icID, TMC9660_CMD_BOOT, 0x981, 0x2, 0xA3B4C5D6, data, addresses, false))
        return TMC9660_ERROR_TIMEOUT;

    return 0;
}

static int32_t tmc9660_param_getVersionASCII_SPI(uint16_t icID, uint8_t *versionString)
{
    // Send the command request
    tmc9660_param_spiSingleRequest(icID, TMC9660_CMD_GET_VERSION, 0, 0, 0, NULL, false, 0);

    // Get the reply with a NOOP request
    uint8_t data[8] = { 0 };
    if (!sendRequestSPI(icID, 0xFF, 0, 0, 0, data, true, 1000))
        return TMC9660_ERROR_TIMEOUT;

    // Unpack the special data format
    versionString[0] = data[0];
    versionString[1] = data[1];
    versionString[2] = data[2];
    versionString[3] = data[3];
    versionString[4] = data[4];
    versionString[5] = data[5];
    versionString[6] = data[6];
    versionString[7] = data[7];

    return 0;
}

static int32_t tmc9660_param_returnToBootloader_SPI(uint16_t icID)
{
    return tmc9660_param_spiSingleRequest(icID, TMC9660_CMD_BOOT, 0x981, 0x2, 0xA3B4C5D6, NULL, false, 0);
}

int32_t tmc9660_reg_sendCommand(uint16_t icID, uint8_t cmd, uint16_t registerOffset, uint8_t registerBlock, uint32_t writeValue, uint32_t *readValue)
{
    TMC9660BusType bus = tmc9660_getBusType(icID);

    if(bus == TMC9660_BUS_SPI)
    {
        // ToDo: SPI support
    }
    else if(bus == TMC9660_BUS_UART)
    {
        return tmc9660_reg_sendCommand_UART(icID, cmd, registerOffset, registerBlock, writeValue, readValue);
    }

    return TMC9660_ERROR_INVALID_BUS;
}

int32_t tmc9660_reg_getVersionASCII(uint16_t icID, uint8_t *versionString)
{
    // In the underlying protocol, register and parameter mode work identically
    // for this special command
    return tmc9660_param_getVersionASCII(icID, versionString);
}

int32_t tmc9660_reg_returnToBootloader(uint16_t icID)
{
    // In the underlying protocol, register and parameter mode work identically
    // for this special command
    return tmc9660_param_returnToBootloader(icID);
}

static int32_t tmc9660_reg_sendCommand_UART(uint16_t icID, uint8_t cmd, uint16_t registerOffset, uint8_t registerBlock, uint32_t writeValue, uint32_t *readValue)
{
    uint8_t data[9] = { 0 };
    TMC9660BusAddresses addresses = tmc9660_getBusAddresses(icID);

    // Create the request datagram
    uint8_t syncByte = 0x01 | (addresses.device);
    data[0] = syncByte; // Module Address & sync bit
    data[1] = cmd;
    data[2] = registerOffset & 0xFF;
    data[3] = (registerOffset >> 8) << 5 | (registerBlock & 0x1F);
    data[4] = (writeValue >> 24) & 0xFF;
    data[5] = (writeValue >> 16) & 0xFF;
    data[6] = (writeValue >> 8) & 0xFF;
    data[7] = (writeValue) & 0xFF;
    data[8] = calcParamChecksum(&data[0], 8);

    if (!tmc9660_readWriteUART(icID, &data[0], 9, 9))
        return TMC9660_ERROR_TIMEOUT;

    // Unpack the reply
    if (data[0] != addresses.host)
        return TMC9660_ERROR_WRONG_ADDR;
    if (data[1] != syncByte)
        return TMC9660_ERROR_INVALID_REPLY;
    if (data[8] != calcParamChecksum(&data[0], 8))
        return TMC9660_ERROR_INVALID_CHECKSUM;

    if (readValue)
    {
        *readValue = ((uint32_t) data[4] << 24)
                   | ((uint32_t) data[5] << 16)
                   | ((uint32_t) data[6] << 8)
                   | ((uint32_t) data[7]);
    }

    return data[2];
}

static uint8_t calcParamChecksum(uint8_t *data, uint32_t bytes)
{
    uint8_t checksum = 0;

    for (uint32_t i = 0; i < bytes; i++)
    {
        checksum += data[i];
    }

    return checksum;
}

/*******************************************************************************************************************************************************************/

uint32_t tmc9660_param_getParameter(uint16_t icID, uint16_t type)
{
    uint32_t value = 0;

    tmc9660_param_sendCommand(icID, TMC9660_CMD_GAP, type, 0, value, &value);

    return value;
}

bool tmc9660_param_setParameter(uint16_t icID, uint16_t type, uint32_t value)
{
    int32_t result = tmc9660_param_sendCommand(icID, TMC9660_CMD_SAP, type, 0, value, &value);

    return result == TMC9660_PARAMSTATUS_OK;
}

uint32_t tmc9660_param_getGlobalParameter(uint16_t icID, uint16_t index)
{
    uint32_t value = 0;

    tmc9660_param_sendCommand(icID, TMC9660_CMD_GGP, index, 0, value, &value);

    return value;
}

bool tmc9660_param_setGlobalParameter(uint16_t icID, uint16_t index, uint32_t value)
{
    int32_t result = tmc9660_param_sendCommand(icID, TMC9660_CMD_SGP, index, 0, value, &value);

    return result == TMC9660_PARAMSTATUS_OK;
}
