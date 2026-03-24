/*******************************************************************************
* Copyright © 2026 Analog Devices Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

/*******************************************************************************
* Example for uploading the hex file to the SRAM of the TMC6460.
* The hex file is converted into a C array; the process is
* mentioned in the readme file. This example loops through
* the array and writes the entries to the SRAM.
*
* Arduino MEGA to TMC6460 Eval Board Wiring:
*   Arduino MEGA Pin | TMC6460 Pin | Signal
*   -----------------|-------------|------------------
*   51 (MOSI)        | 32          | SPI1_SDI
*   50 (MISO)        | 33          | SPI1_SDO
*   52 (SCK)         | 31          | SPI1_SCK
*   53 (SS)          | 30          | SPI1_CSN
*   14 (TX3)         | 21          | UART_RX
*   15 (RX3)         | 22          | UART_TX
*   06 (D6)          | 08          | DRV_EN
*   23 (D23)         | 19          | NSLEEP
*   GND              | 02          | GND
*   5V               | 05, 42      | +5V, +5V_VM
*
*******************************************************************************/

#include <SPI.h>

extern "C" {
#include "TMC6460_HW_Abstraction.h"
#include "TMC6460.h"
#include "firmware/ram_tmc6460.h"
}

// Uncomment the below line to use UART for communication
static TMC6460BusType activeBus = TMC6460_BUS_SPI;
//static TMC6460BusType activeBus = TMC6460_BUS_UART;

int DRV_EN = 6;
int NSLEEP = 23;

#define IC_ID 0

enum TMC6460BusType tmc6460_getBusType(uint16_t icID) {
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

bool is_unprogrammed(uint32_t prog_version){
    
    // Upper 3 bytes contain the program type
    // Ensure that we are currently not running a program
    
    if((prog_version & 0xFFFFFF00) == 0x544D1000)
      return true;
    if((prog_version & 0xFFFFFF00) == 0x64600000)
      return true;
    return false;
}

void ram_upload(){

  // Turn the IOController off
  tmc6460_writeRegister(IC_ID, TMC6460_IO_CONTROLLER_COMMAND, 0x00000000);
  tmc6460_writeRegister(IC_ID, TMC6460_IO_CONTROLLER_RESPONSE_0, 0x00000000);

  // Read out the running version
  tmc6460_writeRegister(IC_ID, TMC6460_IO_CONTROLLER_COMMAND, 0xAA000000);
  uint32_t bl_version;
  tmc6460_readRegister(IC_ID, TMC6460_IO_CONTROLLER_RESPONSE_1, &bl_version);
  Serial.print("bl_version: ");
  Serial.println(bl_version, HEX);

  if(!is_unprogrammed(bl_version)){
    Serial.print("Error: IO controller is already running a program");
    while(true){};
  }

  Serial.println("Downloading the program...");
  
  for(uint32_t i = 0; i < ram_tmc6460_bin_len; i += 2){

    uint8_t lsb = pgm_read_byte(&ram_tmc6460_bin[i]);
    uint8_t msb = pgm_read_byte(&ram_tmc6460_bin[i+1]);
    
    // Command format for writing SRAM: 0xByyyzzzz; where "yyy" represents the appointed address and "zzzz" is the 16-bit value.
    uint32_t command;
    command = 0xB0000000u;
    command |= 0x0FFF0000u & ((i / 2) << 16);
    command |= 0x0000FF00u & (msb << 8);
    command |= 0x000000FFu & lsb;
    
    tmc6460_writeRegister(IC_ID, TMC6460_IO_CONTROLLER_COMMAND, command);
  }

  Serial.println("Verifying upload...");

  for(uint32_t i = 0; i < ram_tmc6460_bin_len; i += 2){

    uint16_t lsb = pgm_read_byte(&ram_tmc6460_bin[i]);
    uint16_t msb = pgm_read_byte(&ram_tmc6460_bin[i+1]);

    // Command format for reading SRAM: 0xCyyy0000; where "yyy" represents the appointed address.
    uint32_t command;
    command = 0xC0000000u;
    command |= 0x0FFF0000u & ((i / 2u) << 16);

    tmc6460_writeRegister(IC_ID, TMC6460_IO_CONTROLLER_COMMAND, command);

    uint32_t reply;
    tmc6460_readRegister(IC_ID, TMC6460_IO_CONTROLLER_RESPONSE_1, &reply);
    
    uint32_t read_data = reply & 0xFFFF;
    uint32_t file_data = (msb << 8) | lsb;
    
    if(read_data != file_data){
      Serial.println("Error: Verification failed - content does not match!");
      while(true){};
    }
  }

  Serial.println("Verification successful: contents of program memory and hex-file are equal!");

  Serial.println("Starting program in TM10 program memory ...");

  // Start program in TM10 memory   
  tmc6460_writeRegister(IC_ID, TMC6460_IO_CONTROLLER_COMMAND, 0xFFFFFFFF);
  delay(500);

  // Turn the IOController off
  tmc6460_writeRegister(IC_ID, TMC6460_IO_CONTROLLER_COMMAND, 0x00000000);
  tmc6460_writeRegister(IC_ID, TMC6460_IO_CONTROLLER_RESPONSE_0, 0x00000000);

  // Get bootloader version
  tmc6460_writeRegister(IC_ID, TMC6460_IO_CONTROLLER_COMMAND, 0xAA000000);
  delay(500);
  tmc6460_readRegister(IC_ID, TMC6460_IO_CONTROLLER_RESPONSE_1, &bl_version);
  Serial.print("bl_versiona: ");
  Serial.println(bl_version, HEX);

}

void setup() {

  Serial.begin(9600);

  pinMode(NSLEEP, OUTPUT);
  pinMode(DRV_EN, OUTPUT);

  if (activeBus == TMC6460_BUS_SPI) {
    pinMode(PIN_SPI_SS, OUTPUT);
    digitalWrite(PIN_SPI_SS, HIGH);
    SPI.begin();
    SPI.beginTransaction(SPISettings(7500000, MSBFIRST, SPI_MODE1));
  }

  else if (activeBus == TMC6460_BUS_UART) {
    Serial3.begin(115200);
  }

  // Toggle the TMC6460 NSLEEP pin to reset it
  digitalWrite(NSLEEP, LOW);
  delayMicroseconds(10);
  digitalWrite(NSLEEP, HIGH);

  digitalWrite(DRV_EN, HIGH);
  delay(10);
  
  // Write the hex file data to SRAM
  ram_upload();
}


void loop() {
}
