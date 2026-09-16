/*******************************************************************************
* Copyright © 2025 Analog Devices, Inc.
*******************************************************************************/

/*******************************************************************************
* This example demonstrates the prototyping of the OTP (One-Time Programmable) feature of the TMC5222. 
* Prototyping is reversible with a power cycle, allowing for experimentation with OTP settings without 
* permanently altering the OTP. The changes made during prototyping take effect immediately, but only 
* until the IC is reset.
*
* In this example, values are written to the USER_DATA registers, although any other register can also 
* be utilized. Some registers are written with write protection enabled, while others are modified with 
* the write protection bit disabled. The impact of overwriting the values in the registers is observed
* in each case.
*
* Arduino MEGA to TMC5222 Eval Board Wiring:
*   Arduino MEGA Pin | TMC5222 Pin | Signal
*   -----------------|-------------|------------------
*   20 (SDA)         | 13          | I2C_SDA
*   21 (SCL )        | 12          | I2C_SCL
*   06 (D6)          | 19          | NSLEEP
*   07 (D7)          | 08          | DRV_EN
*   GND              | 02, 03      | GND
*   GND              | 23 (CLK)    | GND
*   5V               | 05, 42      | +5V, +5V_VM
*
*******************************************************************************/

#include <Wire.h>

extern "C" {
#include "TMC5222_HW_Abstraction.h"
#include "TMC5222.h"
}

#define IC_ID 0
#define WRITEABLE 0
#define WRITE_PROTECT 1
#define DISABLE_IREF_FAULT 0
#define LOCK_OTP 1

static TMC5222BusType activeBus = IC_BUS_IIC;
static uint8_t deviceAddress = 0x60; // 0b110000(W/R)

int NSLEEP = 6;
int DRV_EN = 7;

TMC5222BusType tmc5222_getBusType(uint16_t icID) {
  return activeBus;
}

uint8_t tmc5222_getDeviceAddress(uint16_t icID)
{
  return deviceAddress;
}

bool tmc5222_readWriteIIC(uint16_t icID, uint8_t *data, size_t writeLength, size_t readLength) {

  // ---------- WRITE ----------
  Wire.beginTransmission(deviceAddress);

  if (writeLength > 0) {
    // Write starting at data[1]
    Wire.write(&data[1], writeLength);
  }

  // If we are going to read, keep the bus active
  uint8_t status = Wire.endTransmission(readLength > 0 ? false : true);
  if (status != 0) {
    Serial.println("I2C write error");
    return false;
  }

  // ---------- READ ----------
  if (readLength > 0) {
    size_t received = Wire.requestFrom(deviceAddress, readLength);

    if (received != readLength) {
      Serial.println("I2C read length mismatch");
      return false;
    }

    for (size_t i = 0; i < readLength; i++) {
      if (Wire.available()) {
        data[2 + i] = Wire.read();
      }
    }
  }

  return true;
}

/*OTP Prototyping*/
void prototype_register_start_address(uint16_t address, bool write_protect) {
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_PROT_ADDR, 0x00);
  uint8_t value = address | (write_protect << 7);
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_PROT_WDATA, value);
}

void prototype_register_value(uint32_t regValue) {
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_PROT_ADDR, 0x4);
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_PROT_WDATA, regValue);
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_PROT_ADDR, 0x5);
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_PROT_WDATA, regValue >> 8);
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_PROT_ADDR, 0x06);
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_PROT_WDATA, regValue >> 16);
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_PROT_ADDR, 0x07);
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_PROT_WDATA, regValue >> 24);
}

void prototype_register_lock_value(uint32_t regValue) {
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_PROT_ADDR, 0x8);
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_PROT_WDATA, regValue);
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_PROT_ADDR, 0x9);
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_PROT_WDATA, regValue >> 8);
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_PROT_ADDR, 0xA);
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_PROT_WDATA, regValue >> 16);
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_PROT_ADDR, 0xB);
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_PROT_WDATA, regValue >> 24);
}

void prototype_lock_otp(uint8_t iref_fault, uint8_t mtpblock) {
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_PROT_ADDR, 0xC);
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_PROT_WDATA, iref_fault << 3);
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_PROT_ADDR, 0xF);
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_PROT_WDATA, mtpblock << 7);
}

void prototype_otp(void){
  /*Configuration of OTP*/
  
  // Disable the driver before entering the OTP mode
  tmc5222_fieldWrite(IC_ID, TMC5222_DRV_EN_SW_FIELD, 0x00);

  // Enter OTP mode
  tmc5222_writeRegister(IC_ID, TMC5222_OTP_MODE_OTP_MODE, 0x12A7);

  // Set prototyping mode
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_CONTROL, 0x10);
  
  // Set the address and write the value to the OTP
  prototype_register_start_address(TMC5222_USER_DATA_USER_DATA_3, WRITEABLE);  // Set address to USER_DATA_3(0x49)
  prototype_register_value(0x76543211);                                       // Write data to USER_DATA_3
  prototype_register_value(0xFEDCBA98);                                       // Write data to USER_DATA_4 (0x4A), no need to write the register address again if it comes next in the sequence 0x49, 0x4A,..

  prototype_register_start_address(TMC5222_USER_DATA_USER_DATA_5, WRITE_PROTECT); // Set address to USER_DATA_5 (0x4B). Although 0x4B comes after 0x4A, we write the address to enable write_protect this time
  prototype_register_value(0x01234567);                                           // Write data to USER_DATA_5 
  prototype_register_value(0x89ABCDEF);                                           // Write data to USER_DATA_6 (0x4C)

  prototype_lock_otp(DISABLE_IREF_FAULT, LOCK_OTP); // Lock the whole otp --> after this no new entry will be usable

  // As the OTP was locked before this will not have any effect anymore.
  prototype_register_start_address(TMC5222_USER_DATA_USER_DATA_5, WRITEABLE); // This does nothing now
  prototype_register_value(0xCAFEBEEF); // This does nothing now
  prototype_register_value(0xBABEBABE); // This does nothing now

  // Leave OTP mode
  tmc5222_writeRegister(IC_ID, TMC5222_OTP_MODE_OTP_MODE, 0x12A0);

  // Reading registers
  Serial.println("Reading registers...");
  Serial.print("USER_DATA_3: ");
  Serial.println(tmc5222_readRegister(IC_ID, TMC5222_USER_DATA_USER_DATA_3), HEX);
  Serial.print("USER_DATA_4: ");
  Serial.println(tmc5222_readRegister(IC_ID, TMC5222_USER_DATA_USER_DATA_4), HEX);
  Serial.print("USER_DATA_5: ");
  Serial.println(tmc5222_readRegister(IC_ID, TMC5222_USER_DATA_USER_DATA_5), HEX);
  Serial.print("USER_DATA_6: ");
  Serial.println(tmc5222_readRegister(IC_ID, TMC5222_USER_DATA_USER_DATA_6), HEX);

  // Overwriting registers to 0
  Serial.println("Overwriting registers to 0...");
  tmc5222_writeRegister(IC_ID, TMC5222_USER_DATA_USER_DATA_3, 0x00);
  tmc5222_writeRegister(IC_ID, TMC5222_USER_DATA_USER_DATA_4, 0x00);
  tmc5222_writeRegister(IC_ID, TMC5222_USER_DATA_USER_DATA_5, 0x00);
  tmc5222_writeRegister(IC_ID, TMC5222_USER_DATA_USER_DATA_6, 0x00);

  // Reading registers again
  Serial.println("Reading registers again...");
  Serial.print("USER_DATA_3: ");
  Serial.println(tmc5222_readRegister(IC_ID, TMC5222_USER_DATA_USER_DATA_3), HEX);
  Serial.print("USER_DATA_4: ");
  Serial.println(tmc5222_readRegister(IC_ID, TMC5222_USER_DATA_USER_DATA_4), HEX);
  Serial.print("USER_DATA_5: ");
  Serial.println(tmc5222_readRegister(IC_ID, TMC5222_USER_DATA_USER_DATA_5), HEX);
  Serial.print("USER_DATA_6: ");
  Serial.println(tmc5222_readRegister(IC_ID, TMC5222_USER_DATA_USER_DATA_6), HEX);
}

void setup() {

  Serial.begin(9600);

  pinMode(NSLEEP, OUTPUT);
  pinMode(DRV_EN, OUTPUT);

  // Join I2C bus as master
  Wire.begin(); 
 
  // Toggle the TMC5222 NSLEEP pin to reset it
  digitalWrite(NSLEEP, LOW);
  delayMicroseconds(10);
  digitalWrite(NSLEEP, HIGH);

  digitalWrite(DRV_EN, HIGH);

  delay(1000);
}

void loop() {
  prototype_otp();
  while(1){};
}
