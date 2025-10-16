/*******************************************************************************
* Copyright © 2025 Analog Devices Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/
#include <SPI.h>

extern "C" {
#include "TMC9660.h"
#include "TMC9660_PARAM_HW_Abstraction.h"
#include "TMC9660_BL_HW_Abstraction.h"
}

/* 
 * Arduino Pins       Eval Board Pins
 * 51 MOSI            32 SPI1_SDI
 * 50 MISO            33 SPI1_SDO
 * 52 SCK             31 SPI1_SCK
 * 53 SS              30 SPI1_CSN
 * 14 USART3_TX       22 UART_RX
 * 15 USART3_RX       21 UART_TX
 * GND                2 GND
 * +5V                5 +5V_USB
 * 41 HOLDN_FLASH     34 DIO12
 * 40 FAULTN_STATUS   18 DIO07
 * 49 RESET_CTRL      19 DIO8
 */

#define DEFAULT_IC 0

// TMC9660_addon_spi.hex, Addon "SPI" version 1
static const uint8_t tmc9660_addon[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0x45, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x61, 0x64, 0x64, 0x5F, 0x6F, 0x6E, 0x3A, 0x2D, 0x53, 0x50, 0x49, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x81, 0xBA, 0x00, 0x0D, 0x50, 0xC1, 0xB8, 0x54, 0x97, 0x02, 0x00, 0x00, 0x93, 0x82, 0x82, 0xFD,
        0x37, 0x13, 0x02, 0x00, 0x13, 0x03, 0x03, 0xC0, 0x17, 0x06, 0x00, 0x00, 0x13, 0x06, 0x86, 0x1D,
        0x97, 0x06, 0x00, 0x00, 0x93, 0x86, 0x66, 0x10, 0x17, 0x07, 0x00, 0x00, 0x13, 0x07, 0x87, 0x0A,
        0x97, 0x07, 0x00, 0x00, 0x93, 0x87, 0xA7, 0x14, 0x63, 0x92, 0x62, 0x04, 0x97, 0x02, 0x00, 0x00,
        0x93, 0x82, 0x42, 0x09, 0x17, 0x03, 0x00, 0x00, 0x13, 0x03, 0x63, 0x1E, 0xB3, 0x03, 0x53, 0x40,
        0x37, 0xCE, 0x02, 0x00, 0x61, 0x1E, 0x33, 0x0E, 0x7E, 0x40, 0x13, 0x5E, 0x2E, 0x00, 0x0A, 0x0E,
        0x83, 0xAE, 0x02, 0x00, 0x23, 0x20, 0xDE, 0x01, 0x91, 0x02, 0x11, 0x0E, 0xE3, 0xCA, 0x62, 0xFE,
        0x33, 0x0E, 0x5E, 0x40, 0x72, 0x96, 0xF2, 0x96, 0x72, 0x97, 0xF2, 0x97, 0x97, 0x02, 0x00, 0x00,
        0x93, 0x82, 0x42, 0xF6, 0x03, 0xA3, 0x82, 0x01, 0xB7, 0xC3, 0x02, 0x00, 0x23, 0xAC, 0x63, 0xFE,
        0x03, 0xA3, 0xC2, 0x01, 0x23, 0xAE, 0x63, 0xFE, 0x37, 0x15, 0x02, 0x00, 0x13, 0x05, 0x45, 0xC0,
        0xB7, 0x15, 0x02, 0x00, 0x93, 0x85, 0x45, 0xC4, 0x23, 0x20, 0x05, 0x00, 0x11, 0x05, 0xE3, 0x4D,
        0xB5, 0xFE, 0x37, 0x15, 0x02, 0x00, 0x13, 0x05, 0x05, 0xC0, 0x10, 0xCD, 0x54, 0xCD, 0x18, 0xC5,
        0x1C, 0xD1, 0xB7, 0x85, 0x65, 0x6B, 0x93, 0x85, 0x15, 0x92, 0x0C, 0xC1, 0x82, 0x80, 0x01, 0x00,
        0xB7, 0xE2, 0x00, 0x48, 0xD1, 0x02, 0x05, 0x43, 0x23, 0xA0, 0x62, 0x00, 0xB7, 0xE3, 0x00, 0x48,
        0xE1, 0x03, 0x23, 0xA0, 0x03, 0x00, 0x93, 0x02, 0x00, 0x02, 0x37, 0x53, 0x00, 0x48, 0x23, 0x24,
        0x53, 0x00, 0x23, 0x2E, 0x53, 0x00, 0x37, 0xEE, 0x00, 0x48, 0x41, 0x0E, 0x83, 0x2E, 0x0E, 0x00,
        0x93, 0xFE, 0xFE, 0xF3, 0x23, 0x20, 0xDE, 0x01, 0x41, 0x11, 0x06, 0xC4, 0x02, 0xC0, 0x02, 0xC2,
        0x31, 0x4F, 0xA3, 0x03, 0xE1, 0x01, 0x0A, 0x85, 0x95, 0x6F, 0x93, 0x8F, 0xCF, 0x22, 0x82, 0x9F,
        0xA2, 0x40, 0x41, 0x01, 0x82, 0x80, 0x41, 0x11, 0x06, 0xC0, 0xB7, 0x52, 0x02, 0x00, 0x93, 0x82,
        0x52, 0x39, 0x03, 0xC3, 0x02, 0x00, 0x63, 0x0E, 0x03, 0x02, 0x99, 0x63, 0x81, 0x13, 0x82, 0x93,
        0x0D, 0xC9, 0xB7, 0xE2, 0x00, 0x48, 0xF1, 0x02, 0x03, 0xA3, 0x02, 0x00, 0x13, 0x73, 0x43, 0x00,
        0x63, 0x01, 0x03, 0x02, 0xC8, 0x00, 0x95, 0x63, 0x93, 0x83, 0x63, 0x25, 0x82, 0x93, 0x83, 0x42,
        0xB1, 0x04, 0x13, 0x03, 0xF0, 0x0F, 0xE3, 0x8E, 0x62, 0xFC, 0xA9, 0x60, 0x93, 0x80, 0x40, 0x7B,
        0x11, 0xA0, 0x82, 0x40, 0x41, 0x01, 0x01, 0x45, 0x82, 0x80, 0x85, 0x42, 0x13, 0x03, 0xD6, 0xFF,
        0x63, 0xE4, 0x62, 0x0A, 0xD5, 0xE2, 0x61, 0x11, 0x83, 0xC7, 0x25, 0x00, 0x23, 0x03, 0xF1, 0x00,
        0x3E, 0x83, 0x03, 0xC7, 0x35, 0x00, 0xA3, 0x02, 0xE1, 0x00, 0x3E, 0x93, 0x03, 0xC7, 0x75, 0x00,
        0x23, 0x02, 0xE1, 0x00, 0x3E, 0x93, 0x03, 0xC7, 0x65, 0x00, 0xA3, 0x01, 0xE1, 0x00, 0x3E, 0x93,
        0x03, 0xC7, 0x55, 0x00, 0x23, 0x01, 0xE1, 0x00, 0x3E, 0x93, 0x03, 0xC7, 0x45, 0x00, 0xA3, 0x00,
        0xE1, 0x00, 0x3E, 0x93, 0xFD, 0x17, 0xB3, 0x37, 0xF0, 0x00, 0xB3, 0x07, 0xF0, 0x40, 0x13, 0xE7,
        0xA7, 0x0A, 0xA3, 0x03, 0xE1, 0x00, 0x3A, 0x93, 0x23, 0x00, 0x61, 0x00, 0x0A, 0x85, 0x15, 0x6F,
        0x13, 0x0F, 0xCF, 0x22, 0x02, 0x9F, 0x21, 0x01, 0xAD, 0x60, 0x93, 0x80, 0xC0, 0x37, 0x82, 0x80,
        0x83, 0x42, 0x15, 0x00, 0x13, 0x03, 0xD0, 0x09, 0x63, 0x98, 0x62, 0x02, 0x83, 0x43, 0x25, 0x00,
        0x93, 0x83, 0x43, 0xF3, 0x05, 0x4E, 0x63, 0x61, 0x7E, 0x02, 0x8A, 0x03, 0xB7, 0xCE, 0x02, 0x00,
        0x9E, 0x9E, 0x03, 0xAF, 0x8E, 0xFF, 0x23, 0xA2, 0xE5, 0x01, 0x93, 0x0F, 0x40, 0x06, 0x23, 0x81,
        0xF5, 0x01, 0xAD, 0x60, 0x93, 0x80, 0xA0, 0x94, 0x82, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
 
 
static TMC9660BusType activeBus = TMC9660_BUS_SPI;
static TMC9660BusAddresses busAddresses;

int HOLDN_FLASH = 41;
int FAULTN_STATUS = 40;
int CS = 31;
int RESET_CTRL = 49;
int LED = 13;

void setChipAddresses(uint16_t icID, uint8_t device, uint8_t host) {
  busAddresses.device = device;
  busAddresses.host = host;
}

TMC9660BusType tmc9660_getBusType(uint16_t icID) {
  return activeBus;
}

TMC9660BusAddresses tmc9660_getBusAddresses(uint16_t icID) {
  return busAddresses;
}

uint32_t tmc_getMicrosecondTimestamp() {
  uint32_t timestamp = micros();
  return timestamp;
}

bool tmc9660_isFaultPinAsserted(uint16_t icID) {
  // Note: The FAULTN pin is active low.
  // It is asserted, when it is low!
  return digitalRead(FAULTN_STATUS) == LOW;
}

void tmc9660_readWriteSPI(uint16_t icID, uint8_t *data, size_t dataLength, bool keepCSNAsserted) {
  digitalWrite(CS, LOW);
  delayMicroseconds(10);
  for (uint32_t i = 0; i < dataLength; i++) {
    data[i] = SPI.transfer(data[i]);
  }

  delayMicroseconds(10);
  if (!keepCSNAsserted) {
    digitalWrite(CS, HIGH);
  }
  delayMicroseconds(10);
}

bool tmc9660_readWriteUART(uint16_t icID, uint8_t *data, size_t writeLength, size_t readLength) {
  Serial3.write(data, writeLength);
  delay(2);

  unsigned long startTime = millis();

  // Wait for write echo
  while (Serial3.available() < writeLength) {
    if (millis() - startTime >= 1000) {
      return false;
    }
  }

  // Wait for the actual response
  startTime = millis();
  while (Serial3.available() < readLength) {
    if (millis() - startTime >= 1000) {
      return false;  // Timeout
    }
  }

  // Read the actual response
  Serial3.readBytes(data, readLength);
  return true;
}

static void rotateMotorOpenLoop(uint32_t targetVelocity) {

  tmc9660_param_setParameter(DEFAULT_IC, TMC9660_PARAM_MOTOR_TYPE, 2);  // STEPPER_MOTOR
  tmc9660_param_setParameter(DEFAULT_IC, TMC9660_PARAM_OPENLOOP_VOLTAGE, 1000);
  tmc9660_param_setParameter(DEFAULT_IC, TMC9660_PARAM_COMMUTATION_MODE, 3);  // FOC_OPENLOOP_VOLTAGE_MODE
  tmc9660_param_setParameter(DEFAULT_IC, TMC9660_PARAM_TARGET_VELOCITY, targetVelocity);
  Serial.println("Rotating motor in openloop mode for 5 secs");

  delay(5000);

  tmc9660_param_setParameter(DEFAULT_IC, TMC9660_PARAM_TARGET_VELOCITY, 0);

  // tmc9660_param_setParameter(DEFAULT_IC, TMC9660_PARAM_COMMUTATION_MODE, 0);  // SYSTEM_OFF
  Serial.println("Motor Stopped");
}

static void bootstrapping() {
  // Paste the autogenerated ubltools C code here:
  uint32_t value;

  // Initial commands
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_SET_BANK, 0x00000005, NULL);

  // Modifying CONFIG.BOOT_04_BOOTSTRAP
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_SET_ADDRESS, 0x00020008, NULL);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_READ_16, 0, &value);
  value = field_update16(value, CONFIG_BOOT_04_BOOTSTRAP_BOOT_APP_FIELD, 2);
  value = field_update16(value, CONFIG_BOOT_04_BOOTSTRAP_BL_ENTRY_FAULT_FIELD, 0);
  value = field_update16(value, CONFIG_BOOT_04_BOOTSTRAP_BL_EXIT_FAULT_FIELD, 0);
  value = field_update16(value, CONFIG_BOOT_04_BOOTSTRAP_BL_CONFIG_FAULT_FIELD, 1);
  value = field_update16(value, CONFIG_BOOT_04_BOOTSTRAP_LOAD_ROM_CODE_FIELD, 0);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_WRITE_16, value, NULL);
  tmc9660_waitForFaultDeassertion(DEFAULT_IC);

  // Modifying CONFIG.BOOT_00_POWER
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_SET_ADDRESS, 0x00020000, NULL);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_READ_16, 0, &value);
  value = field_update16(value, CONFIG_BOOT_00_POWER_VEXT1_FIELD, 3);
  value = field_update16(value, CONFIG_BOOT_00_POWER_VEXT2_FIELD, 2);
  value = field_update16(value, CONFIG_BOOT_00_POWER_LDO_SHORT_FAULT_FIELD, 0);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_WRITE_16, value, NULL);
  tmc9660_waitForFaultDeassertion(DEFAULT_IC);

  // Modifying CONFIG.BOOT_01_ADDRESS
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_SET_ADDRESS, 0x00020002, NULL);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_READ_16, 0, &value);
  value = field_update16(value, CONFIG_BOOT_01_DEVICE_ADDRESS_FIELD, 1);
  value = field_update16(value, CONFIG_BOOT_01_MASTER_ADDRESS_FIELD, 255);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_WRITE_16, value, NULL);
  tmc9660_waitForFaultDeassertion(DEFAULT_IC);

  // Updating communication addresses
  setChipAddresses(DEFAULT_IC, 1, 255);

  // Modifying CONFIG.BOOT_10_APP_CONFIG_0
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_SET_ADDRESS, 0x00020020, NULL);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_READ_16, 0, &value);
  value = field_update16(value, CONFIG_BOOT_10_APP_CONFIG_0_HALL_ENABLE_FIELD, 1);
  value = field_update16(value, CONFIG_BOOT_10_APP_CONFIG_0_ABN1_ENABLE_FIELD, 1);
  value = field_update16(value, CONFIG_BOOT_10_APP_CONFIG_0_HALL_UX_FIELD, 0);
  value = field_update16(value, CONFIG_BOOT_10_APP_CONFIG_0_HALL_V_FIELD, 0);
  value = field_update16(value, CONFIG_BOOT_10_APP_CONFIG_0_HALL_WY_FIELD, 0);
  value = field_update16(value, CONFIG_BOOT_10_APP_CONFIG_0_ABN1_A_FIELD, 1);
  value = field_update16(value, CONFIG_BOOT_10_APP_CONFIG_0_ABN1_B_FIELD, 1);
  value = field_update16(value, CONFIG_BOOT_10_APP_CONFIG_0_ABN1_N_FIELD, 1);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_WRITE_16, value, NULL);
  tmc9660_waitForFaultDeassertion(DEFAULT_IC);

  // Modifying CONFIG.BOOT_06_EEPROM
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_SET_ADDRESS, 0x0002000C, NULL);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_READ_16, 0, &value);
  value = field_update16(value, CONFIG_BOOT_06_I2C_EEPROM_EN_FIELD, 0);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_WRITE_16, value, NULL);
  tmc9660_waitForFaultDeassertion(DEFAULT_IC);

  // Modifying CONFIG.BOOT_14_APP_CONFIG_4
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_SET_ADDRESS, 0x00020028, NULL);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_READ_16, 0, &value);
  value = field_update16(value, CONFIG_BOOT_14_APP_CONFIG_4_MEM_TMCL_SCRIPT_FIELD, 1);
  value = field_update16(value, CONFIG_BOOT_14_APP_CONFIG_4_MEM_PARAMETERS_FIELD, 1);
  value = field_update16(value, CONFIG_BOOT_14_APP_CONFIG_4_MEM_STIMULUS_FIELD, 1);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_WRITE_16, value, NULL);
  tmc9660_waitForFaultDeassertion(DEFAULT_IC);

  // Modifying CONFIG.BOOT_11_APP_CONFIG_1
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_SET_ADDRESS, 0x00020022, NULL);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_READ_16, 0, &value);
  value = field_update16(value, CONFIG_BOOT_11_APP_CONFIG_1_REF_L_PIN_FIELD, 3);
  value = field_update16(value, CONFIG_BOOT_11_APP_CONFIG_1_REF_R_PIN_FIELD, 2);
  value = field_update16(value, CONFIG_BOOT_11_APP_CONFIG_1_REF_H_PIN_FIELD, 4);
  value = field_update16(value, CONFIG_BOOT_11_APP_CONFIG_1_STEPDIR_ENABLE_FIELD, 0);
  value = field_update16(value, CONFIG_BOOT_11_APP_CONFIG_1_ABN2_ENABLE_FIELD, 0);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_WRITE_16, value, NULL);
  tmc9660_waitForFaultDeassertion(DEFAULT_IC);

  // Modifying CONFIG.BOOT_12_APP_CONFIG_2
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_SET_ADDRESS, 0x00020024, NULL);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_READ_16, 0, &value);
  value = field_update16(value, CONFIG_BOOT_12_APP_CONFIG_2_WDG_DISABLE_FIELD, 0);
  value = field_update16(value, CONFIG_BOOT_12_APP_CONFIG_2_WDG_TIMEOUT_FIELD, 7);
  value = field_update16(value, CONFIG_BOOT_12_APP_CONFIG_2_BRAKECHOPPER_ENABLE_FIELD, 0);
  value = field_update16(value, CONFIG_BOOT_12_APP_CONFIG_2_MECH_BRAKE_ENABLE_FIELD, 0);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_WRITE_16, value, NULL);
  tmc9660_waitForFaultDeassertion(DEFAULT_IC);

  // Modifying CONFIG.BOOT_13_APP_CONFIG_3
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_SET_ADDRESS, 0x00020026, NULL);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_READ_16, 0, &value);
  value = field_update16(value, CONFIG_BOOT_13_APP_CONFIG_3_SPI_ENC_ENABLE_FIELD, 0);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_WRITE_16, value, NULL);
  tmc9660_waitForFaultDeassertion(DEFAULT_IC);

  // Modifying CONFIG.BOOT_CLK_SEL_INIT
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_SET_ADDRESS, 0x00020018, NULL);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_READ_32, 0, &value);
  value = field_update32(value, CONFIG_BOOT_0C_CLK_SEL_INIT_PLL_FB_DIV_FIELD, 99);
  value = field_update32(value, CONFIG_BOOT_0C_CLK_SEL_INIT_EXT_NOT_XTAL_FIELD, 0);
  value = field_update32(value, CONFIG_BOOT_0C_CLK_SEL_INIT_XTAL_CFG_FIELD, 3);
  value = field_update32(value, CONFIG_BOOT_0C_CLK_SEL_INIT_XTAL_BOOST_FIELD, 0);
  value = field_update32(value, CONFIG_BOOT_0C_CLK_SEL_INIT_EXT_NOT_INT_FIELD, 1);
  value = field_update32(value, CONFIG_BOOT_0C_CLK_SEL_INIT_PLL_OUT_SEL_FIELD, 1);
  value = field_update32(value, CONFIG_BOOT_0C_CLK_SEL_INIT_RDIV_FIELD, 15);
  value = field_update32(value, CONFIG_BOOT_0C_CLK_SEL_INIT_SYS_CLK_DIV_FIELD, 0);
  value = field_update32(value, CONFIG_BOOT_0C_CLK_SEL_INIT_PLL_CONFIG_BOOT_FIELD, 0);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_WRITE_32, value, NULL);
  tmc9660_waitForFaultDeassertion(DEFAULT_IC);

  // ---- WARNING - next write should be a RAM_WRITE!
  // Modifying CONFIG.BOOT_03_BOOT_INTERFACE
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_SET_ADDRESS, 0x00020006, NULL);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_READ_16, 0, &value);
  value = field_update16(value, CONFIG_BOOT_03_INTERFACE_BL_DISABLE_UART_FIELD, 0);
  value = field_update16(value, CONFIG_BOOT_03_INTERFACE_BL_DISABLE_SPI_FIELD, 0);  //done
  value = field_update16(value, CONFIG_BOOT_03_INTERFACE_BL_SPI_SELECT_FIELD, 0);   //done
  value = field_update16(value, CONFIG_BOOT_03_INTERFACE_BL_UART_RX_FIELD, 0);
  value = field_update16(value, CONFIG_BOOT_03_INTERFACE_BL_UART_TX_FIELD, 0);
  value = field_update16(value, CONFIG_BOOT_03_INTERFACE_BL_UART_BAUDRATE_FIELD, 7);  //
  value = field_update16(value, CONFIG_BOOT_03_INTERFACE_BL_SPI0_SCK_FIELD, 1);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_WRITE_16, value, NULL);
  tmc9660_waitForFaultDeassertion(DEFAULT_IC);

  //Modifying CONFIG.BOOT_05_FLASH
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_SET_ADDRESS, 0x0002000A, NULL);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_READ_16, 0, &value);
  value = field_update16(value, CONFIG_BOOT_05_SPI_FLASH_EN_FIELD, 0);  //
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_WRITE_16, value, NULL);
  tmc9660_waitForFaultDeassertion(DEFAULT_IC);
}

void setup() {

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  Serial.begin(9600);

  setChipAddresses(DEFAULT_IC, 1, 255);
  pinMode(FAULTN_STATUS, INPUT);

  if (activeBus == TMC9660_BUS_UART) {
    Serial3.begin(115200);
    delay(10);
    pinMode(HOLDN_FLASH, OUTPUT);
    digitalWrite(HOLDN_FLASH, HIGH);
  } else if (activeBus == TMC9660_BUS_SPI) {

    pinMode(CS, OUTPUT);
    digitalWrite(CS, HIGH);
    SPI.begin();
    SPI.beginTransaction(SPISettings(1875000, MSBFIRST, SPI_MODE3));
    pinMode(HOLDN_FLASH, OUTPUT);
    digitalWrite(HOLDN_FLASH, LOW);
  }

  pinMode(RESET_CTRL, OUTPUT);
  digitalWrite(RESET_CTRL, HIGH);
  delay(10);

  digitalWrite(RESET_CTRL, LOW);

  delay(1000);

  Serial.println("\nStarted writing config...");
  bootstrapping();
  Serial.println("Finished writing config!");

  Serial.println("Uploading Addon!");
  int32_t err = tmc9660_bl_installAddon(DEFAULT_IC, &tmc9660_addon[0], sizeof(tmc9660_addon));
  Serial.print("Addon uploaded with status: ");
  Serial.println(err);
  delay(100);
  uint32_t id = 0, version = 0;
  err = tmc9660_bl_getAddonInfo(DEFAULT_IC, &id, &version);
  Serial.print("Addon id: ");
  Serial.println(id);
  Serial.print("Addon version: ");
  Serial.println(version);
  //Starting the App!
  uint32_t value = 0;
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_SET_BANK, 0x00000005, NULL);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_SET_ADDRESS, 0x00020008, NULL);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_READ_16, 0, &value);
  value = field_update16(value, CONFIG_BOOT_04_BOOTSTRAP_LOAD_ROM_CODE_FIELD, 1);
  tmc9660_bl_sendCommand(DEFAULT_IC, TMC9660_BLCMD_WRITE_16, value, NULL);
  tmc9660_waitForFaultDeassertion(DEFAULT_IC);

  Serial.println("App Started!");
}

void loop() {
  String inputString = "";
  if (Serial.available()) {
    inputString = Serial.readStringUntil('\n');
    if (inputString == "1") {
      Serial.println("read module id");
      uint32_t val = 0;
      int32_t status = tmc9660_param_sendCommand(DEFAULT_IC, 136, 1, 0, 0, &val);

      Serial.print("Module ID: ");
      Serial.println(val >> 16);

      Serial.print("Status: ");
      Serial.println(status);

    } else if (inputString == "2") {
      rotateMotorOpenLoop(90000);
    }
    inputString = "";
  }
}
