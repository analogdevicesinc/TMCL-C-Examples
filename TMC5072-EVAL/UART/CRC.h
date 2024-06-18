/*******************************************************************************
* Copyright © 2011 TRINAMIC Motion Control GmbH & Co. KG
* (now owned by Analog Devices Inc.),
*
* Copyright © 2023 Analog Devices Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

/****************************************************
  Projekt:  TMC5130 and TMC5072 CRC calculation

  Modul:    CRC.h
            CRC-Calculation for UART interfacing

  Hinweise: Start with CRC-Register=0,
            then call NextCRC for each byte to be sent
            or each by received. Send CRC byte last or
            check received CRC

  Datum:    14.6.2011 OK
*****************************************************/
#ifndef CRC_H
#define CRC_H

  #include "TypeDefs.h"

  uint8 NextCRCSingle(uint8 Crc, uint8 Data, uint8 Gen, uint8 Bit);
  uint8 NextCRC(uint8 Crc, uint8 Data, uint8 Gen);

#endif
