/*******************************************************************************
* Copyright © 2025 Analog Devices Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/
#include <SPI.h>

extern "C" {
#include "TMC9660.h"
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

#define IC_ID 0

typedef enum {
  BOOTSTRAPPING,
  GET_MODULE_ID,
  RUN_EXAMPLE,
  IDLE,
} Commands;
static Commands cmd = IDLE;

static TMC9660BusType activeBus = TMC9660_BUS_SPI;
static TMC9660BusAddresses busAddresses;
const int BUFFER_SIZE = 5;
uint8_t buffer[BUFFER_SIZE] = { 0 };
int HOLDN_FLASH = 41;
int RESET_CTRL = 49;
int LED = 13;


TMC9660BusType tmc9660_getBusType(uint16_t icID) {
  return activeBus;
}

TMC9660BusAddresses tmc9660_getBusAddresses(uint16_t icID) {
  return busAddresses;
}

void readWriteSPI(uint8_t *data, size_t dataLength) {
  digitalWrite(PIN_SPI_SS, LOW);
  delayMicroseconds(10);
  for (uint32_t i = 0; i < dataLength; i++) {
    data[i] = SPI.transfer(data[i]);
  }

  delayMicroseconds(10);
  digitalWrite(PIN_SPI_SS, HIGH);
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

static uint8_t calcChecksum(uint8_t *data, uint32_t bytes) {
  uint8_t checksum = 0;

  for (uint32_t i = 0; i < bytes; i++) {
    checksum += data[i];
  }

  return checksum;
}

static int processTunnelBL(uint8_t *buffer, int size) {
  if (activeBus == TMC9660_BUS_SPI) {
    // Send the data
    readWriteSPI(buffer, 5);
    Serial.write(&buffer[0], 5);
    Serial.flush();
  }
  return 0;
}

static int processTunnelApp(uint8_t operation, uint8_t type, uint8_t motor, uint32_t *value) {
  if (activeBus == TMC9660_BUS_SPI) {
    uint8_t data[8] = { 0 };
    data[0] = operation;  //Operation
    data[1] = type;       //type
    data[2] = motor;      //motor
    data[3] = (*value >> 24) & 0xFF;
    data[4] = (*value >> 16) & 0xFF;
    data[5] = (*value >> 8) & 0xFF;
    data[6] = (*value) & 0xFF;
    data[7] = calcChecksum(data, 7);

    readWriteSPI(&data[0], 8);
    delayMicroseconds(100);

    do {
      // Send another datagram to receive the previous response
      memset(&data[0], 0, 8);

      // Send the data
      readWriteSPI(&data[0], 8);
    } while (data[0] == 0xF0);

    Serial.write(&data[0], 8);
    Serial.flush();
  }
  return 0;
}

static void writeParameter(uint16_t type, uint8_t motor, uint32_t value) {
  uint32_t *val = &value;
  uint8_t tmclMotor = motor | ((type & 0xF00) >> 4);
  uint8_t tmclType = type & 0xFF;
  processTunnelApp(5, tmclType, tmclMotor, val);
}

static void rotateMotorOpenLoop(uint32_t targetVelocity) {
  if (activeBus == TMC9660_BUS_SPI) {
    writeParameter(TMC9660_STEPPER_PARAM_EVAL_MOTOR_TYPE, 0, 2);  // STEPPER_MOTOR
    writeParameter(TMC9660_STEPPER_PARAM_EVAL_OPENLOOP_VOLTAGE, 0, 1000);
    writeParameter(TMC9660_STEPPER_PARAM_EVAL_COMMUTATION_MODE, 0, 3);  // FOC_OPENLOOP_VOLTAGE_MODE
    writeParameter(TMC9660_STEPPER_PARAM_EVAL_TARGET_VELOCITY, 0, targetVelocity);

    delay(5000);

    writeParameter(TMC9660_STEPPER_PARAM_EVAL_TARGET_VELOCITY, 0, 0);
    writeParameter(TMC9660_STEPPER_PARAM_EVAL_COMMUTATION_MODE, 0, 0);  // SYSTEM_OFF
  } else {
    tmc9660_param_setParameter(IC_ID, TMC9660_STEPPER_PARAM_EVAL_MOTOR_TYPE, 2);  // STEPPER_MOTOR
    tmc9660_param_setParameter(IC_ID, TMC9660_STEPPER_PARAM_EVAL_OPENLOOP_VOLTAGE, 1000);
    tmc9660_param_setParameter(IC_ID, TMC9660_STEPPER_PARAM_EVAL_COMMUTATION_MODE, 3);  // FOC_OPENLOOP_VOLTAGE_MODE
    tmc9660_param_setParameter(IC_ID, TMC9660_STEPPER_PARAM_EVAL_TARGET_VELOCITY, targetVelocity);

    delay(5000);

    tmc9660_param_setParameter(IC_ID, TMC9660_STEPPER_PARAM_EVAL_TARGET_VELOCITY, 0);
    tmc9660_param_setParameter(IC_ID, TMC9660_STEPPER_PARAM_EVAL_COMMUTATION_MODE, 0);  // SYSTEM_OFF
  }
}

void setup() {

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  Serial.begin(9600);

  busAddresses.device = 0x01;
  busAddresses.host = 0xFF;

  if (activeBus == TMC9660_BUS_UART) {
    Serial3.begin(115200);
    delay(10);

    pinMode(PIN_SPI_MOSI, OUTPUT);
    pinMode(PIN_SPI_SCK, OUTPUT);
    pinMode(PIN_SPI_SS, OUTPUT);
    digitalWrite(PIN_SPI_MOSI, LOW);
    digitalWrite(PIN_SPI_SS, LOW);
    digitalWrite(PIN_SPI_SCK, LOW);
    pinMode(HOLDN_FLASH, OUTPUT);
    digitalWrite(HOLDN_FLASH, HIGH);
  } else if (activeBus == TMC9660_BUS_SPI) {

    pinMode(PIN_SPI_SS, OUTPUT);
    digitalWrite(PIN_SPI_SS, HIGH);
    SPI.begin();
    SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE3));
    // SPI.beginTransaction(SPISettings(1875000, MSBFIRST, SPI_MODE3));
    pinMode(HOLDN_FLASH, OUTPUT);
    digitalWrite(HOLDN_FLASH, LOW);
  }

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
        cmd = IDLE;
        uint32_t val = 0;
        int32_t status = 0;
        if (activeBus == TMC9660_BUS_SPI) {
          status = processTunnelApp(136, 1, 0, &val);
        } else {
          status = tmc9660_param_sendCommand(IC_ID, 136, 1, 0, &val);
          uint8_t data[4] = { (val >> 24) & 0xFF, (val >> 16) & 0xFF, (val >> 8) & 0xFF, (val >> 0) & 0xFF };
          if (status != -2) {
            Serial.write(&data[0], 4);
            Serial.flush();
          }else{
          digitalWrite(LED, HIGH);
        }
        }
        continue;
      } else if (buffer[0] == 0xCC) {
        cmd = IDLE;
        rotateMotorOpenLoop(50000);
        continue;
      }

      switch (cmd) {
        case BOOTSTRAPPING:
          if (activeBus == TMC9660_BUS_SPI) {
            if (processTunnelBL(buffer, BUFFER_SIZE) == -1) {
              digitalWrite(LED, HIGH);
            }
          } else {
            uint32_t val = 0;
            int8_t status = 0;
            val = ((uint32_t)buffer[1] << 24) | ((uint32_t)buffer[2] << 16) | ((uint32_t)buffer[3] << 8) | (buffer[4] << 0);
            status = tmc9660_boot_sendCommand(IC_ID, buffer[0], &val);
            uint8_t data[5] = { status, (val >> 24) & 0xFF, (val >> 16) & 0xFF, (val >> 8) & 0xFF, (val >> 0) & 0xFF };
            if (status == 0) {
              Serial.write(&data[0], 5);
              Serial.flush();
            } else {
              digitalWrite(LED, HIGH);
            }
          }
          break;
      }
    }
  }
}
