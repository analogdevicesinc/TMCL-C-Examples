/*******************************************************************************
* Copyright © 2024 Analog Devices Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#include <SPI.h>

extern "C" {
#include "TMC2209_HW_Abstraction.h"
#include "TMC2209.h"
#include "TMC2209_Simple_Rotation.h"
}

/* 
 * In this example the step pulses for Motor are produced by using a Timer-1
 *  
 * Arduino Pins       Eval Board Pins
 * ---------------------------------------
 * 14 UART_TX3        21 UART_TX (DI010)
 * 15 UART_RX3        22 UART_RX (DIO11)
 * 08 PIN_8           08 ENN (DIO0)
 * 11 PIN_11          (J303) STEP
 * 12 PIN_12          (J303) DIR
 * GND                GND
 * 
 * # This pinout mentioned for arduino are with reference to Arduino Mega
 */

#define MOTOR0_ID 0
#define MOTOR1_ID 1
#define MOTOR2_ID 2
#define MOTOR3_ID 3

static uint8_t nodeAddress0 = 0;
static uint8_t nodeAddress1 = 1;
static uint8_t nodeAddress2 = 2;
static uint8_t nodeAddress3 = 3;

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

// Pin for Motor-0
int en0_pin = 10;
int dir0_pin = 12;
int step0_pin = 11;

// Pins for Motor-1
int en1_pin = 2;
int dir1_pin = 3;
int step1_pin = 4;

uint8_t tmc2209_getNodeAddress(uint16_t icID) {

  switch (icID)
    case MOTOR0_ID;
      return nodeAddress0;
    case MOTOR1_ID;
      return nodeAddress1;
    case MOTOR2_ID;
      return nodeAddress2;
    case MOTOR3_ID;
      return nodeAddress3;
      
}



bool tmc2209_readWriteUART(uint16_t icID, uint8_t *data, size_t writeLength, size_t readLength) {
    Serial3.write(data, writeLength);
    
    delay(2);                             // Appropriate for your setup

    unsigned long startTime = millis();
    // Wait for write echo
    while (Serial3.available() < readLength) {
        // One second timeout
        if (millis() - startTime >= 1000) {
          Serial.println("Serial Write Timeout!");
          return false; // Timeout
        }
    }

    Serial3.readBytes(data, readLength);
    
    return true;
}

void setup() {
  // Configuring Timer 1 - CTC Mode
  TCCR1B |= (1 << WGM13) | (1 << CS10);   // Setting CTC Mode for the timer adn prescalar=1

  OCR1A = 0x0007;                         // Setting output at 100kHz

  TIMSK1 = (1 << ICIE1) | (1 << OCIE1A);  // Enabling respective ISR for respective Timer 1 interrupt

  // Initialising Pinouts
  pinMode(en0_pin, OUTPUT);
  pinMode(step0_pin, OUTPUT);
  pinMode(dir0_pin, OUTPUT);
  
  pinMode(en1_pin, OUTPUT);
  pinMode(step1_pin, OUTPUT);
  pinMode(dir1_pin, OUTPUT);

  digitalWrite(en0_pin, HIGH);
  digitalWrite(step0_pin, LOW);
  digitalWrite(dir0_pin, LOW);

  digitalWrite(en1_pin, HIGH);
  digitalWrite(step1_pin, LOW);
  digitalWrite(dir1_pin, LOW);

  // Configuring UART for communication
  Serial3.begin(115200);

  delay(10);
   
  // Initialising Motors
  initAllMotors(MOTOR0_ID);
  initAllMotors(MOTOR1_ID);

  // Enabling Motor Outputs
  digitalWrite(en0_pin, LOW);
  digitalWrite(dir0_pin, HIGH);

  digitalWrite(en1_pin, LOW);
  digitalWrite(dir1_pin, HIGH);
}

void loop() {
  // Debugging Loop
  int32_t value0 = tmc2209_readRegister(MOTOR0_ID, TMC2209_GCONF);
  if (value0 == -2) {
    Serial.println("Error reading GCONF register!");
  } else {
    Serial.print("Received Data 0: ");
    Serial.println(value0);
  }

  int32_t value1 = tmc2209_readRegister(MOTOR0_ID, TMC2209_IHOLD_IRUN);
  if (value1 == -2) {
    Serial.println("Error reading IHOLD_IRUN register!");
  } else {
    Serial.print("Received Data 1: ");
    Serial.println(value1);
  }
  
  Serial.println("----------------------");
  
  delay(1000);
}

ISR(TIMER1_COMPA_vect){
  digitalWrite(step0_pin, digitalRead(step0_pin) ^ 1);
  digitalWrite(step1_pin, digitalRead(step1_pin) ^ 1);
}
