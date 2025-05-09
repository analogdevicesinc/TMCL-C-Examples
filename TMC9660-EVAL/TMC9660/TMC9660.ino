/*******************************************************************************
* Copyright © 2025 Analog Devices Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

extern "C" {
#include "TMC9660_STEPPER_PARAM_HW_Abstraction.h"
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
 * 49 RESET_CTRL       19 DIO8
 */

typedef enum{
  BOOTSTRAPPING,
  GET_MODULE_ID,
  RUN_EXAMPLE,
  IDLE,
}Commands;
static Commands cmd = IDLE;

typedef enum {
  IC_BUS_UART,
  IC_BUS_SPI,
} TMC9660BusType;

static TMC9660BusType activeBus = IC_BUS_UART;
const int BUFFER_SIZE = 5;
uint8_t buffer[BUFFER_SIZE] = { 0 };
int HOLDN_FLASH = 41;
int RESET_CTRL = 49;
int LED = 13;

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


bool readWriteUART(uint8_t *data, size_t writeLength, size_t readLength) {
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

static uint8_t CRC8(uint8_t *data, uint32_t bytes) {
  uint8_t result = 0;

  while (bytes--) result = tmcCRCTable_Poly7Reflected[result ^ *data++];

  // Flip the result around
  // swap odd and even bits
  result = ((result >> 1) & 0x55) | ((result & 0x55) << 1);
  // swap consecutive pairs
  result = ((result >> 2) & 0x33) | ((result & 0x33) << 2);
  // swap nibbles ...
  result = ((result >> 4) & 0x0F) | ((result & 0x0F) << 4);

  return result;
}

static int processTunnelBL(uint8_t* buffer, int size) {
  if (activeBus == IC_BUS_UART) {
    uint8_t data[8] = { 0 };

    data[0] = 0x55;       // Sync byte
    data[1] = 0x01;       // Device Address
    data[2] = buffer[0];  // Command
    data[3] = buffer[1];
    data[4] = buffer[2];
    data[5] = buffer[3];
    data[6] = buffer[4];
    data[7] = CRC8(data, 7);

    if (!readWriteUART(&data[0], 8, 8)) {
      return -1;
    }

    Serial.write(&data[2], 5);
    Serial.flush();
  }
  return 0;
}

static uint8_t calcCheckSum(uint8_t *data, uint32_t bytes)
{
    uint8_t checkSum = 0;

    for (uint32_t i = 0; i < bytes; i++) { checkSum += data[i]; }
    return checkSum;
}

static int processTunnelApp(uint8_t operation, uint8_t type, uint8_t motor, uint32_t *value) {
  uint8_t data[9] = { 0 };

  data[0] = 0x01;       // Module Address
  data[1] = operation;  //Operation
  data[2] = type;       //type
  data[3] = motor;      //motor
  data[4] = (*value >> 24) & 0xFF;
  data[5] = (*value >> 16) & 0xFF;
  data[6] = (*value >> 8) & 0xFF;
  data[7] = (*value) & 0xFF;
  data[8] = calcCheckSum(data, 8);

  if (!readWriteUART(&data[0], 9, 9)) {
    return -1;
  }

  // Byte 8: CRC correct?
  if (data[8] != calcCheckSum(data, 8)) {
    return -2;
  }

  *value = ((uint32_t) data[4] << 24) | ((uint32_t) data[5] << 16) | ((uint32_t) data[6] << 8) | data[7];
  
  Serial.write(&data[2], 6);
  Serial.flush();

  return 0;
}

static void writeParameter(uint16_t type, uint8_t motor, uint32_t value) {
  uint32_t *val = &value;
  uint8_t tmclMotor = motor | ((type & 0xF00) >> 4);
  uint8_t tmclType = type & 0xFF;
  processTunnelApp(5, tmclType, tmclMotor, val);
}

static void rotateMotorOpenLoop(uint32_t targetVelocity) {
  writeParameter(TMC9660_STEPPER_PARAM_EVAL_MOTOR_TYPE, 0, 2); // STEPPER_MOTOR
  writeParameter(TMC9660_STEPPER_PARAM_EVAL_OPENLOOP_VOLTAGE, 0, 1000);
  writeParameter(TMC9660_STEPPER_PARAM_EVAL_COMMUTATION_MODE, 0, 3);  // FOC_OPENLOOP_VOLTAGE_MODE
  writeParameter(TMC9660_STEPPER_PARAM_EVAL_TARGET_VELOCITY, 0, targetVelocity);

  delay(5000);

  writeParameter(TMC9660_STEPPER_PARAM_EVAL_TARGET_VELOCITY, 0, 0);
  writeParameter(TMC9660_STEPPER_PARAM_EVAL_COMMUTATION_MODE, 0, 0);  // SYSTEM_OFF
}

void setup() {

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  Serial.begin(9600);

  if (activeBus == IC_BUS_UART) {
    Serial3.begin(115200);
    delay(10);

    pinMode(PIN_SPI_MOSI, OUTPUT);
    pinMode(PIN_SPI_SCK, OUTPUT);
    pinMode(PIN_SPI_SS, OUTPUT);
    digitalWrite(PIN_SPI_MOSI, LOW);
    digitalWrite(PIN_SPI_SS, LOW);
    digitalWrite(PIN_SPI_SCK, LOW);
  }

  pinMode(HOLDN_FLASH, OUTPUT);
  digitalWrite(HOLDN_FLASH, HIGH);
  pinMode(RESET_CTRL, OUTPUT);
  digitalWrite(RESET_CTRL, HIGH);
  delay(10);

  digitalWrite(RESET_CTRL, LOW);

  delay(1000);
}

void loop() {
  while (1) {
    if (Serial.available() > 0) {
      Serial.readBytes(buffer, BUFFER_SIZE);

      if (buffer[0] == 0xAA) {
        cmd = BOOTSTRAPPING;
        digitalWrite(LED, LOW);
        continue;
        
      } else if (buffer[0] == 0xBB) {
        uint32_t val = 0;
        int status = processTunnelApp(136, 1, 0, &val);
        if (status == -1 || status == -2) {
          digitalWrite(LED, HIGH);
        }
      } else if (buffer[0] == 0xCC) {
        rotateMotorOpenLoop(50000);
      }

      switch (cmd) {
        case BOOTSTRAPPING:
          if (processTunnelBL(buffer, BUFFER_SIZE) == -1) {
            digitalWrite(LED, HIGH);
          }
          break;
      }
    }
  }
}
