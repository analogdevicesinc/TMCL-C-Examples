/*******************************************************************************
* Copyright © 2017 TRINAMIC Motion Control GmbH & Co. KG
* (now owned by Analog Devices Inc.),
*
* Copyright © 2023 Analog Devices Inc. All Rights Reserved. This software is
* proprietary & confidential to Analog Devices, Inc. and its licensors.
*******************************************************************************/

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
 
#include "TMC5072_register.h"
#include "CRC.h"

#define CRC8_GEN 0x07
  
int SWSEL = 2;
int DRV_ENN = 3;
int SDO_RING = 4;
const int CLOCKOUT = 10;

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
  uartWriteDatagram(0x00, TMC5072_SLAVECONF, 0x00000001); //SLAVEADDR to 1
  delay(500);

  tmc5072_initMotorDrivers();

}

void loop() {
  // put your main code here, to run repeatedly:
  
  uartRead(0x01, TMC5072_XACTUAL_1);  //read out XACTUAL of motor 1
  uartRead(0x01, TMC5072_XACTUAL_2);  //read out XACTUAL of motor 2
  
  uartWriteDatagram(0x01, TMC5072_XTARGET_1, 0x0007D000);
  //XTARGET=512000 | 10 revolutions with micro step = 256
  uartWriteDatagram(0x01, TMC5072_XTARGET_2, 0xFFF83000);
  //XTARGET=-512000 | 10 revolutions with micro step = 256
  
  delay(15000);

  uartRead(0x01, TMC5072_XACTUAL_1);  //read out XACTUAL of motor 1
  uartRead(0x01, TMC5072_XACTUAL_2);  //read out XACTUAL of motor 2

  uartWriteDatagram(0x01, TMC5072_XTARGET_1, 0x00000000); //XTARGET=0
  uartWriteDatagram(0x01, TMC5072_XTARGET_2, 0x00000000); //XTARGET=0
  
  delay(15000);
}

void uartWriteDatagram(uint8_t SLAVEADDR, uint8_t registerAddress, 
      unsigned long datagram) {
  //TMC5072 takes 64 bit data: 8 sync + reserved, 8 chip address, 
  //8 register address, 32 data, 8 CRC

  uint8_t CRC = 0;
  int temp;
  unsigned char buf[8];
  CRC = NextCRC(CRC, 0x05, CRC8_GEN);
  CRC = NextCRC(CRC, SLAVEADDR, CRC8_GEN);
  CRC = NextCRC(CRC, registerAddress|0x80, CRC8_GEN);  
  CRC = NextCRC(CRC, (datagram >> 24) & 0xff, CRC8_GEN);
  CRC = NextCRC(CRC, (datagram >> 16) & 0xff, CRC8_GEN);
  CRC = NextCRC(CRC, (datagram >> 8) & 0xff, CRC8_GEN);
  CRC = NextCRC(CRC, datagram & 0xff, CRC8_GEN);
  
  buf[0] = 0x05;
  buf[1] = SLAVEADDR;
  buf[2] = registerAddress|0x80;
  buf[3] = (datagram >> 24) & 0xff;
  buf[4] = (datagram >> 16) & 0xff;
  buf[5] = (datagram >> 8) & 0xff;
  buf[6] = datagram & 0xff; 
  buf[7] = CRC;
  
  temp = Serial1.write(buf, 8); //write datagram
  Serial1.flush();  //wait until all datas are written
  Serial1.readBytes(buf, 8); //clear input buffer
}

unsigned long uartRead(uint8_t SALVEADDR, uint8_t registerAddress) {

  uint8_t CRC = 0, temp;
  unsigned char buf[8];
  unsigned long dataBytes;

  CRC = NextCRC(CRC, 0x05, CRC8_GEN);
  CRC = NextCRC(CRC, SALVEADDR, CRC8_GEN);
  CRC = NextCRC(CRC, registerAddress, CRC8_GEN);
  buf[0] = 0x05;
  buf[1] = SALVEADDR;
  buf[2] = registerAddress;
  buf[3] = CRC;
  Serial1.write(buf, 4); //write datagram
  Serial1.flush();  //wait until all datas are written
  Serial1.readBytes(buf, 4); //clear input buffer
  
  Serial1.readBytes(buf, 8);
  temp = buf[2];
  dataBytes = buf[3]; //bit 32...24
  dataBytes <<= 8;
  dataBytes |= buf[4]; //bit 23...16
  dataBytes <<= 8;
  dataBytes |= buf[5]; //bit 15...8
  dataBytes <<= 8;
  dataBytes |= buf[6]; //bit 7...0
  
  CRC = 0;
  for(int i=0;i<7;i++)
  {
    CRC = NextCRC(CRC, buf[i], CRC8_GEN);
  }

  //show received bytes
  Serial.print("Received: 0x");
  for(int i=0;i<8;i++)
  {
    char tmp[16];
    sprintf(tmp, "%.2X", buf[i]);
    Serial.print(tmp);
  }
  Serial.print("\n");
  Serial.print("CRC: "); Serial.print(CRC,HEX);Serial.print(" <-> BUFFER: "); 
  Serial.println(buf[7],HEX);

  return dataBytes;
}

void tmc5072_initMotorDrivers()
{
  //2-phase configuration Motor 1
  uartWriteDatagram(0x01, TMC5072_CHOPCONF_1, 0x00010135);
  uartWriteDatagram(0x01, TMC5072_IHOLD_IRUN_1, 0x00071400);

  //2-phase configuration Motor 2
  uartWriteDatagram(0x01, TMC5072_CHOPCONF_2, 0x00010135);
  uartWriteDatagram(0x01, TMC5072_IHOLD_IRUN_2, 0x00071400);


  //Reset positions
  uartWriteDatagram(0x01, TMC5072_RAMPMODE_1, TMC5072_MODE_POSITION);
  uartWriteDatagram(0x01, TMC5072_XTARGET_1, 0);
  uartWriteDatagram(0x01, TMC5072_XACTUAL_1, 0);
  uartWriteDatagram(0x01, TMC5072_RAMPMODE_2, TMC5072_MODE_POSITION);
  uartWriteDatagram(0x01, TMC5072_XTARGET_2, 0);
  uartWriteDatagram(0x01, TMC5072_XACTUAL_2, 0);

  //Standard values for speed and acceleration
  uartWriteDatagram(0x01, TMC5072_VSTART_1, 1);
  uartWriteDatagram(0x01, TMC5072_A1_1, 5000);
  uartWriteDatagram(0x01, TMC5072_V1_1, 26843);
  uartWriteDatagram(0x01, TMC5072_AMAX_1, 5000);   
  uartWriteDatagram(0x01, TMC5072_VMAX_1, 100000);
  uartWriteDatagram(0x01, TMC5072_DMAX_1, 5000);
  uartWriteDatagram(0x01, TMC5072_D1_1, 5000);
  uartWriteDatagram(0x01, TMC5072_VSTOP_1, 10);

  uartWriteDatagram(0x01, TMC5072_VSTART_2, 1);
  uartWriteDatagram(0x01, TMC5072_A1_2, 5000);
  uartWriteDatagram(0x01, TMC5072_V1_2, 26843);
  uartWriteDatagram(0x01, TMC5072_AMAX_2, 5000);
  uartWriteDatagram(0x01, TMC5072_VMAX_2, 100000);
  uartWriteDatagram(0x01, TMC5072_DMAX_2, 5000);
  uartWriteDatagram(0x01, TMC5072_D1_2, 5000);
  uartWriteDatagram(0x01, TMC5072_VSTOP_2, 10);
}

