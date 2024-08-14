/*******************************************************************************
* Copyright © 2024 Analog Devices Inc. All Rights Reserved. This software is
* proprietary & confidential to Analog Devices, Inc. and its licensors.
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
 * -----------------------------------
 * +5V                42 +5V_VM
 * 14 UART_TX3        21 UART_TX (DI010)
 * 15 UART_RX3        22 UART_RX (DIO11)
 * 10 PIN_8           08 ENN (DIO0)
 * 11 PIN_11(OC1A)    17 INT_STEP (DIO6)
 * 12 PIN_12          18 INT_DIR (DIO7)
 * GND                23 CLK
 * GND                GND
 *
 ** For using multi-node configurations, please look at the pins in the code and make connections accordingly
 */

#define MOTOR0_ID 0
#define MOTOR1_ID 1
#define MOTOR2_ID 2
#define MOTOR3_ID 3

enum motors {
  motor0,
  motor1,
  motor2,
  motor3
};

int en0_pin = 10;
int dir0_pin = 12;
int step0_pin = 11;

int en1_pin = 2;
int dir1_pin = 4;
int step1_pin = 3;

uint8_t tmc2209_getNodeAddress(uint16_t icID) {
  switch (icID){
    case 0:
      return ( motor0);
    case 1:
      return ( motor1);
    case 2:
      return ( motor2);
    case 3:
      return ( motor3);
    default:
      return ( motor0);
  }
}


bool tmc2209_readWriteUART(uint16_t icID, uint8_t *data, size_t writeLength, size_t readLength) {
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
  // Configuring Timer 1 - CTC Mode
  TCCR1B |= (1 << WGM13) | (1 << CS10);   // Setting CTC Mode for the timer adn prescalar=1

  OCR1A = 0x0001;                         // Setting output at 100kHz

  TIMSK1 |= (1 << ICIE1) | (1 << OCIE1A);  // Enabling respective ISR for respective Timer 1 interrupt
  // TIMSK1 &= ~(1 << ICIE1) & ~(1 << OCIE1A);  // Disabeling respective ISR for respective Timer 1 interrupt

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
  Serial1.begin(115200);
  delay(10);
  
  Serial.begin(9600);
  delay(10);

  // Inititalize motors
  initAllMotors(MOTOR0_ID);
  initAllMotors(MOTOR1_ID);

  // Enabling Motor Outputs
  digitalWrite(en0_pin, LOW);
  digitalWrite(dir0_pin, HIGH);

  digitalWrite(en1_pin, LOW);
  digitalWrite(dir1_pin, HIGH);

}

void loop() {
}

ISR(TIMER1_COMPA_vect){
  PORTB ^= (1 << PB5);
  PORTE ^= (1 << PE5);
}
