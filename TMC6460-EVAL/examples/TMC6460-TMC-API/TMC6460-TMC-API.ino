/*******************************************************************************
* Copyright © 2026 Analog Devices Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

/* 
 * --------Pinout--------------
 * Arduino MEGA  |  Eval Board 
 ------------------------------
 * 51 MOSI       32 SPI1_SDI
 * 50 MISO       33 SPI1_SDO
 * 52 SCK        31 SPI1_SCK
 * 53 SS         30 SPI1_CSN
 * 14 TX3        21 UART_RX
 * 15 RX3        22 UART_TX   
 * 23 D23        19 NSLEEP
 * GND           02 GND
 * 5V            05 +5V
 */

#include <SPI.h>

extern "C" {
    #include "TMC6460_HW_Abstraction.h"
    #include "TMC6460.h"
}

#define IC_ID 0

// Uncomment the below line to use UART for communication 
//static ICBusType activeBus = IC_BUS_SPI;
static ICBusType activeBus = IC_BUS_UART;

int DRV_EN = 6;
int nSLEEP = 23;

enum ICBusType tmc6460_getBusType(uint16_t icID)
{
    return activeBus;
}

void tmc6460_readWriteSPI(uint16_t icID, uint8_t *data, size_t dataLength) {
    digitalWrite(PIN_SPI_SS, LOW);
    delayMicroseconds(10);

    for (uint32_t i = 0; i < dataLength; i++) {
        data[i] = SPI.transfer(data[i]);
    }

    delayMicroseconds(10);
    digitalWrite(PIN_SPI_SS, HIGH);
}


bool tmc6460_readWriteUART(uint16_t icID, uint8_t *data, size_t writeLength, size_t readLength)
{
    Serial3.write(data, writeLength);
    delay(2); // Ensure this delay is appropriate for your setup

    unsigned long startTime = millis();
    
    // Wait for write echo
    while (Serial3.available() < readLength) {
        if (millis() - startTime >= 1000) {
          Serial.println("Serial Write Timeout!");
          return false; // Timeout
        }
    }

    // Read the actual response
    Serial3.readBytes(data, readLength);
    return true;
}

void setup()
{
    Serial.begin(9600);
  
    pinMode(nSLEEP, OUTPUT);
    pinMode(DRV_EN, OUTPUT);

    // Toggle the TMC6460 nSLEEP pin to reset it
    digitalWrite(nSLEEP, LOW);
    delayMicroseconds(10);
    digitalWrite(nSLEEP, HIGH);

    digitalWrite(DRV_EN, HIGH);

    
    if (activeBus == IC_BUS_SPI) {
        pinMode(PIN_SPI_SS, OUTPUT);
        digitalWrite(PIN_SPI_SS, HIGH);
        SPI.begin();
        SPI.beginTransaction(SPISettings(7500000, MSBFIRST, SPI_MODE1));
    }

    else if (activeBus == IC_BUS_UART) {
      Serial3.begin(115200);
    }

    delay(10);

    // Write to CHIP.IO_MATRIX register
    tmc6460_writeRegister(IC_ID, 0x06, 0x42);
}

void loop() {
  uint32_t data;
  tmc6460_readRegister(IC_ID, 0x06, &data);
  Serial.print("Received Data: ");
  Serial.println(data, HEX);
  delay(10);
  Serial.print(" from register: ");
  Serial.println(0x06, HEX);
  delay(1000);
}
