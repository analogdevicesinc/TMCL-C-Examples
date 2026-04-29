/*******************************************************************************
* Copyright © 2025 Analog Devices, Inc.
*******************************************************************************/

/*******************************************************************************
* This example demonstrates the prototyping of the OTP (One-Time Programmable) feature of the TMC5221. 
* Prototyping is reversible with a power cycle, allowing for experimentation with OTP settings without 
* permanently altering the OTP. The changes made during prototyping take effect immediately, but only 
* until the IC is reset.
*
* In this example, values are written to the USER_DATA registers, although any other register can also 
* be utilized. Some registers are written with write protection enabled, while others are modified with 
* the write protection bit disabled. The impact of overwriting the values in the registers is observed
* in each case.
*
* Arduino MEGA to TMC5221 Eval Board Wiring:
*   Arduino MEGA Pin | TMC5221 Pin | Signal
*   -----------------|-------------|------------------
*   51 (MOSI)        | 32          | SPI1_SDI
*   50 (MISO)        | 33          | SPI1_SDO
*   52 (SCK)         | 31          | SPI1_SCK
*   53 (SS)          | 30          | SPI1_CSN
*   23 (D23)         | 19          | NSLEEP
*   GND              | 02, 03      | GND
*   GND              | 23 (CLK)    | GND
*   5V               | 05, 42      | +5V, +5V_VM
*
*******************************************************************************/

#include <SPI.h>

extern "C" {
#include "TMC5221_HW_Abstraction.h"
#include "TMC5221.h"
}

#define IC_ID 0
#define WRITEABLE 0
#define WRITE_PROTECT 1
#define DISABLE_IREF_FAULT 0
#define LOCK_OTP 1

static TMC5221BusType activeBus = IC_BUS_SPI;

int NSLEEP = 23;
int DRV_EN = 7;
int nCS = 22;

TMC5221BusType tmc5221_getBusType(uint16_t icID) {
  return activeBus;
}

void tmc5221_readWriteSPI(uint16_t icID, uint8_t *data, size_t dataLength) {
  digitalWrite(nCS, LOW);
  delayMicroseconds(5);

  for (uint32_t i = 0; i < dataLength; i++) {
    data[i] = SPI.transfer(data[i]);
  }

  delayMicroseconds(5);
  digitalWrite(nCS, HIGH);
}

/*OTP Prototyping*/
void prototype_register_start_address(uint16_t address, bool write_protect) {
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_PROT_ADDR, 0x00);
  uint8_t value = address | (write_protect << 7);
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_PROT_WDATA, value);
}

void prototype_register_value(uint32_t regValue) {
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_PROT_ADDR, 0x4);
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_PROT_WDATA, regValue);
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_PROT_ADDR, 0x5);
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_PROT_WDATA, regValue >> 8);
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_PROT_ADDR, 0x06);
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_PROT_WDATA, regValue >> 16);
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_PROT_ADDR, 0x07);
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_PROT_WDATA, regValue >> 24);
}

void prototype_register_lock_value(uint32_t regValue) {
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_PROT_ADDR, 0x8);
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_PROT_WDATA, regValue);
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_PROT_ADDR, 0x9);
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_PROT_WDATA, regValue >> 8);
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_PROT_ADDR, 0xA);
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_PROT_WDATA, regValue >> 16);
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_PROT_ADDR, 0xB);
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_PROT_WDATA, regValue >> 24);
}

void prototype_lock_otp(uint8_t iref_fault, uint8_t mtpblock) {
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_PROT_ADDR, 0xC);
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_PROT_WDATA, iref_fault << 3);
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_PROT_ADDR, 0xF);
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_PROT_WDATA, mtpblock << 7);
}

void prototype_otp(void){
  /*Configuration of OTP*/
  
  // Disable the driver before entering the OTP mode
  tmc5221_fieldWrite(IC_ID, TMC5221_DRV_EN_SW_FIELD, 0x00);

  // Enter OTP mode
  tmc5221_writeRegister(IC_ID, TMC5221_OTP_MODE_OTP_MODE, 0x12A7);

  // Set prototyping mode
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_CONTROL, 0x10);
  
  // Set the address and write the value to the OTP
  prototype_register_start_address(TMC5221_USER_DATA_USER_DATA_3, WRITEABLE);  // Set address to USER_DATA_3(0x49)
  prototype_register_value(0x76543211);                                       // Write data to USER_DATA_3
  prototype_register_value(0xFEDCBA98);                                       // Write data to USER_DATA_4 (0x4A), no need to write the register address again if it comes next in the sequence 0x49, 0x4A,..

  prototype_register_start_address(TMC5221_USER_DATA_USER_DATA_5, WRITE_PROTECT); // Set address to USER_DATA_5 (0x4B). Although 0x4B comes after 0x4A, we write the address to enable write_protect this time
  prototype_register_value(0x01234567);                                           // Write data to USER_DATA_5 
  prototype_register_value(0x89ABCDEF);                                           // Write data to USER_DATA_6 (0x4C)

  prototype_lock_otp(DISABLE_IREF_FAULT, LOCK_OTP); // Lock the whole otp --> after this no new entry will be usable

  // As the OTP was locked before this will not have any effect anymore.
  prototype_register_start_address(TMC5221_USER_DATA_USER_DATA_5, WRITEABLE); // This does nothing now
  prototype_register_value(0xCAFEBEEF); // This does nothing now
  prototype_register_value(0xBABEBABE); // This does nothing now

  // Leave OTP mode
  tmc5221_writeRegister(IC_ID, TMC5221_OTP_MODE_OTP_MODE, 0x12A0);

  // Reading registers
  Serial.println("Reading registers...");
  Serial.print("USER_DATA_3: ");
  Serial.println(tmc5221_readRegister(IC_ID, TMC5221_USER_DATA_USER_DATA_3), HEX);
  Serial.print("USER_DATA_4: ");
  Serial.println(tmc5221_readRegister(IC_ID, TMC5221_USER_DATA_USER_DATA_4), HEX);
  Serial.print("USER_DATA_5: ");
  Serial.println(tmc5221_readRegister(IC_ID, TMC5221_USER_DATA_USER_DATA_5), HEX);
  Serial.print("USER_DATA_6: ");
  Serial.println(tmc5221_readRegister(IC_ID, TMC5221_USER_DATA_USER_DATA_6), HEX);

  // Overwriting registers to 0
  Serial.println("Overwriting registers to 0...");
  tmc5221_writeRegister(IC_ID, TMC5221_USER_DATA_USER_DATA_3, 0x00);
  tmc5221_writeRegister(IC_ID, TMC5221_USER_DATA_USER_DATA_4, 0x00);
  tmc5221_writeRegister(IC_ID, TMC5221_USER_DATA_USER_DATA_5, 0x00);
  tmc5221_writeRegister(IC_ID, TMC5221_USER_DATA_USER_DATA_6, 0x00);

  // Reading registers again
  Serial.println("Reading registers again...");
  Serial.print("USER_DATA_3: ");
  Serial.println(tmc5221_readRegister(IC_ID, TMC5221_USER_DATA_USER_DATA_3), HEX);
  Serial.print("USER_DATA_4: ");
  Serial.println(tmc5221_readRegister(IC_ID, TMC5221_USER_DATA_USER_DATA_4), HEX);
  Serial.print("USER_DATA_5: ");
  Serial.println(tmc5221_readRegister(IC_ID, TMC5221_USER_DATA_USER_DATA_5), HEX);
  Serial.print("USER_DATA_6: ");
  Serial.println(tmc5221_readRegister(IC_ID, TMC5221_USER_DATA_USER_DATA_6), HEX);
}

void setup() {

  Serial.begin(9600);

  pinMode(NSLEEP, OUTPUT);
  pinMode(DRV_EN, OUTPUT);

  if (activeBus == IC_BUS_SPI) {
    pinMode(nCS, OUTPUT);
    digitalWrite(nCS, HIGH);
    SPI.begin();
    SPI.beginTransaction(SPISettings(3000000, MSBFIRST, SPI_MODE3)); // 3 MHz
  }

  // Toggle the TMC5221 NSLEEP pin to reset it
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
