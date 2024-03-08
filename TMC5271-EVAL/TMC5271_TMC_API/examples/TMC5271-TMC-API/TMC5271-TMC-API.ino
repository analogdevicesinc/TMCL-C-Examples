/*******************************************************************************
* Copyright © 2023 Analog Devices Inc. All Rights Reserved. This software is
* proprietary & confidential to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#include <SPI.h>

#include "../TMC5271_HW_Abstraction.h"
#include "../TMC5271.h" 
#include "../TMC5271_Simple_Rotation.h"

extern "C" {
    #include "TMC5271_HW_Abstraction.h"
    #include "TMC5271.h"
    #include "TMC5271_Simple_Rotation.h"
}

/* 
 * Arduino Pins   Eval Board Pins
 * 51 MOSI        32 SPI1_SDI
 * 50 MISO        33 SPI1_SDO
 * 52 SCK         31 SPI1_SCK
 * 53 SS          30 SPI1_CSN
 * 14 USART3_TX   22 UART_RX
 * 15 USART3_RX   21 UART_TX
 * GND            23 CLK16 -> use internal 
 * 23 DIO         19 nSleep 
 * GND            2 GND
 * +5V            5 +5V_USB
 * 27 iRefR2      35 IREF_R2
 * 29 iRefR3      36 IRREF_R3
 * 31 uartMode    20 USEL
 */

 #define IC_ID 0
static TMC5271BusType activeBus = IC_BUS_SPI;
static uint8_t nodeAddress = 0;

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

int nSleep = 23;
int iRefR2 = 27;
int iRefR3 = 29;
int uartMode = 31;

uint8_t tmc5271_getNodeAddress(uint16_t icID) {
    return nodeAddress;
}

TMC5271BusType tmc5271_getBusType(uint16_t icID) {
    return activeBus;
}

void tmc5271_readWriteSPI(uint16_t icID, uint8_t *data, size_t dataLength) {
    digitalWrite(PIN_SPI_SS, LOW);
    delayMicroseconds(10);

    for (uint32_t i = 0; i < dataLength; i++) {
        data[i] = SPI.transfer(data[i]);
        Serial.println(data[i]);
    }

    delayMicroseconds(10);
    digitalWrite(PIN_SPI_SS, HIGH);
}

bool tmc5271_readWriteUART(uint16_t icID, uint8_t *data, size_t writeLength, size_t readLength) {
    Serial3.write(data, writeLength);

    //delay(10); //better wait for data
    for (int i = 0;i<writeLength;i++){
    }
    unsigned long startTime = millis();
    // Read back the echo from the write operation
    while (Serial3.available() < writeLength ){
        //Timeout of 1sec
        if (millis() - startTime >= 1000){
        return false;
        }    
    } 
    Serial3.readBytes(data, writeLength);
    for (int i = 0;i<writeLength;i++){
    }

    // Read the reply
    if (readLength > 0)
    {
        while (Serial3.available() < readLength ){
        //add timeout, if timeout return false, if not return true
        } 
        Serial3.readBytes(data, readLength);
        for (int i = 0;i<readLength;i++){
        }
    }

    return true;
}

void setup()
{
    Serial.begin(115200);

    // put your setup code here, to run once:
    pinMode(PIN_SPI_SS, OUTPUT);
    pinMode(nSleep, OUTPUT);
    pinMode(iRefR2, OUTPUT);
    pinMode(iRefR3, OUTPUT);
    pinMode(uartMode, OUTPUT);

    digitalWrite(PIN_SPI_SS, HIGH);
    digitalWrite(nSleep, LOW); // Disabling standby
    digitalWrite(iRefR2, LOW);
    digitalWrite(iRefR3, LOW);
    
    if (activeBus == IC_BUS_SPI) {
        digitalWrite(uartMode, LOW);

        SPI.begin();
        SPI.beginTransaction(SPISettings(115200, MSBFIRST, SPI_MODE3));
    }
    else if (activeBus == IC_BUS_UART) {
        digitalWrite(uartMode, HIGH);
        Serial3.begin(115200);
        pinMode(PIN_SPI_MOSI, OUTPUT);
        pinMode(PIN_SPI_SCK, OUTPUT);
        digitalWrite(PIN_SPI_MOSI, LOW);
        digitalWrite(PIN_SPI_SS, LOW);
        digitalWrite(PIN_SPI_SCK, LOW);
    } 

    delay(10);

    digitalWrite(iRefR2, HIGH);
    digitalWrite(iRefR3, HIGH);

    initAllMotors(IC_ID);
    tmc5271_rotateMotor(IC_ID, 0, 0x00002710);
}

void loop() {
  int32_t value = tmc5271_readRegister(IC_ID, TMC5271_VMAX);
  Serial.print("Received Data: ");
  Serial.println(value);
  Serial.print(" from register: ");
  Serial.println(TMC5271_VMAX, HEX);
  delay(1000);
}