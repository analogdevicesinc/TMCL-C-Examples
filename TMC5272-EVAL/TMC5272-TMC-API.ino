/*******************************************************************************
* Copyright © 2023 Analog Devices Inc. All Rights Reserved. This software is
* proprietary & confidential to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#include <SPI.h>

extern "C" {
#include "TMC5272_HW_Abstraction.h"
#include "TMC5272.h"
#include "TMC5272_Simple_Rotation.h"
}

/* 
 *  
 * Arduino Pins   Eval Board Pins
 * 51 MOSI        32 SPI1_SDI
 * 50 MISO        33 SPI1_SDO
 * 52 SCK         31 SPI1_SCK
 * 53 SS          30 SPI1_CSN
 * 25 DIO         8 DIO0 (DRV_ENN)
 * GND            23 CLK16 -> use internal 
 * 23 DIO         19 nSleep 
 * GND            2 GND
 * +5V            5 +5V_USB
 * 27 iRefR2     35 IREF_R2
 * 29 iRefR3     36 IRREF_R3
 * 31 uartMode    20 USEL
 */

#define IC_ID 0
static TMC5272BusType activeBus = IC_BUS_SPI;
static uint32_t nodeAddress = 0;

int enable = 25;
int nSleep = 23;
int iRefR2 = 27;
int iRefR3 = 29;
int uartMode = 31;

uint32_t tmc5272_getNodeAddress(void) {
  return nodeAddress;
}

TMC5272BusType tmc5272_getBusType(uint16_t icID) {
  return activeBus;
}

void tmc5272_readWriteSPI(uint16_t icID, uint8_t *data, size_t dataLength) {
  digitalWrite(PIN_SPI_SS, LOW);
  delayMicroseconds(10);

  for (uint32_t i = 0; i < dataLength; i++) {
    data[i] = SPI.transfer(data[i]);
    Serial.println(data[i]);
  }

  delayMicroseconds(10);
  digitalWrite(PIN_SPI_SS, HIGH);
}

void tmc5272_readWriteUART(uint16_t icID, uint8_t *data, size_t writeLength, size_t readLength) {
  Serial3.write(data, writeLength);

  delay(10);

  if (Serial3.available() >= readLength) {
    Serial3.readBytes(data, readLength);
  }
}

void setup() {
  Serial.begin(115200);

  // put your setup code here, to run once:
  pinMode(PIN_SPI_SS, OUTPUT);
  pinMode(enable, OUTPUT);
  pinMode(nSleep, OUTPUT);
  pinMode(iRefR2, OUTPUT);
  pinMode(iRefR3, OUTPUT);
  pinMode(uartMode, OUTPUT);

  digitalWrite(PIN_SPI_SS, HIGH);
  digitalWrite(nSleep, LOW);
  digitalWrite(enable, HIGH);
  digitalWrite(iRefR2, LOW);
  digitalWrite(iRefR3, LOW);

  if (activeBus == IC_BUS_SPI) {
    digitalWrite(uartMode, LOW);

    SPI.begin();
    SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE3));
  } else if (activeBus == IC_BUS_UART) {
    digitalWrite(uartMode, HIGH);
    Serial3.begin(115200);
  }

  delayMicroseconds(10);
  digitalWrite(enable, LOW);

  digitalWrite(iRefR2, HIGH);
  digitalWrite(iRefR3, HIGH);

  initAllMotors(IC_ID);
  tmc5272_rotateMotor(IC_ID, 0, 0x00002710);
  tmc5272_rotateMotor(IC_ID, 1, 0x00002710);
}

void loop() {
  int32_t value = tmc5272_readRegister(IC_ID, TMC5272_VMAX(0));
  Serial.print("Received Data: ");
  Serial.println(value);
  Serial.print(" from register: ");
  Serial.println(TMC5272_VMAX(0), HEX);
  delay(1000);
}
