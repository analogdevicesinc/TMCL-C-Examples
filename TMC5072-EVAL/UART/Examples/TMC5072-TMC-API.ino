/*******************************************************************************
* Copyright © 2017 TRINAMIC Motion Control GmbH & Co. KG
* (now owned by Analog Devices Inc.),
*
* Copyright © 2023 Analog Devices Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

extern "C" {
#include "TMC5072_HW_Abstraction.h"
#include "TMC5072.h"
#include "TMC5072_Simple_Rotation.h"
}

/* 
 * TMC5072-EVAL_UART.ino
 * 
 * Created on: 20.02.2017
 *     Author: MN
 *     
 *  The Trinamic TMC5072 motor controller and driver operates through Single 
 *  Wire UART. This is 64 bits datagram (8 sync bytes, 8 bit slave address,
 *  8 bit register address and 32 bit word).
 *  Each register is specified by a one byte register address: with MSB = 0
 *  for read, MSB = 1 for write.
 *  
 * Arduino Pins   Eval Board Pins
 * 18 TX1         40 SWIOP
 * 19 RX1         41 SWION
 *  2 DIO         38 DIO16 (SWSEL)
 *  4 DIO         33 SDO/RING
 *  3 DIO          8 DIO0 (DRV_ENN)
 * 10 DIO         23 CLK16
 *    GND          2 GND
 *    +5V          5 +5V
 */
 
#define IC_ID 0
  
int SWSEL = 2;
int DRV_ENN = 3;
int SDO_RING = 4;
const int CLOCKOUT = 10;

uint8_t tmc5072_getNodeAddress(uint16_t icID) {
  return (IC_ID);
}

bool tmc5072_readWriteUART(uint16_t icID, uint8_t *data, size_t writeLength, size_t readLength) {
    Serial1.write(data, writeLength);
    delay(2); // Ensure this delay is appropriate for your setup

    unsigned long startTime = millis();
    
    // Wait for write echo
    while (Serial1.available() < readLength) {
        if (millis() - startTime >= 1000) {
          Serial.println("Serial Write Timeout!");
          return false; // Timeout
        }
    }
    
    // Skip the echoed bytes
    Serial1.readBytes(data, writeLength);

    // Wait for the actual response
    startTime = millis();
    while (Serial1.available() < readLength) {
        if (millis() - startTime >= 1000) {
            Serial.println("Serial Read Timeout!");
            return false; // Timeout
        }
    }

    // Read the actual response
    Serial1.readBytes(data, readLength);
    return true;
}


void setup() {

  pinMode(SWSEL, OUTPUT);  
  pinMode(CLOCKOUT,OUTPUT);
  pinMode(DRV_ENN, OUTPUT);
  pinMode(SDO_RING, OUTPUT);

  digitalWrite(DRV_ENN, LOW);
  //HIGH = power stage disabled, LOW = power stage enabled  
  digitalWrite(SWSEL, HIGH);
  //HIGH = UART mode, LOW = SPI mode 
  digitalWrite(SDO_RING, HIGH);
  //HIGH = ring mode, LOW = normal mode
  
  //setup timer2
  TCCR2A = ((1 << WGM21) | (1 << COM2A0));
  TCCR2B = (1 << CS20);
  TIMSK2 = 0;
  OCR2A = 0;

  Serial.begin(115200);
  Serial1.begin(115200);
  
  delay(500);
  // tmc5072_writeRegister(IC_ID, TMC5072_SLAVECONF, 0x00000001); //SLAVEADDR to 1
  // delay(500);

  initAllMotors();

}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("X Actual, Motor 0: 0x");
  Serial.println(tmc5072_readRegister(IC_ID, TMC5072_XACTUAL(0)), HEX);  //read out XACTUAL of motor 1
  Serial.print("X Actual, Motor 1: 0x");
  Serial.println(tmc5072_readRegister(IC_ID, TMC5072_XACTUAL(1)), HEX);  //read out XACTUAL of motor 2
  
  tmc5072_writeRegister(IC_ID, TMC5072_XTARGET(0), 0x0007D000);
  //XTARGET=512000 | 10 revolutions with micro step = 256
  tmc5072_writeRegister(IC_ID, TMC5072_XTARGET(1), 0xFFF83000);
  //XTARGET=-512000 | 10 revolutions with micro step = 256
  
  delay(15000);

  Serial.print("X Actual, Motor 0: 0x");
  Serial.println(tmc5072_readRegister(IC_ID, TMC5072_XACTUAL(0)), HEX);  //read out XACTUAL of motor 1
  Serial.print("X Actual, Motor 1: 0x");
  Serial.println(tmc5072_readRegister(IC_ID, TMC5072_XACTUAL(1)), HEX);  //read out XACTUAL of motor 2

  tmc5072_writeRegister(IC_ID, TMC5072_XTARGET(0), 0x00000000); //XTARGET=0
  tmc5072_writeRegister(IC_ID, TMC5072_XTARGET(1), 0x00000000); //XTARGET=0
  
  delay(15000);
}
