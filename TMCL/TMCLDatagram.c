/*******************************************************************************
* Copyright © 2024 Analog Devices Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

// Example for sending TMCL commands as datagrams to a module.
// The functions for handling the serial port must be added by the user.
// Data types used here: UCHAR: 1 byte (unsigned char)
//                       INT:   4 byte integer ("long" on most MCUs)

// Opcodes of all TMCL commands that can be used in direct mode
#define TMCL_ROR     1
#define TMCL_ROL     2
#define TMCL_MST     3
#define TMCL_MVP     4
#define TMCL_SAP     5
#define TMCL_GAP     6
#define TMCL_STAP    7
#define TMCL_RSAP    8
#define TMCL_SGP     9
#define TMCL_GGP     10
#define TMCL_STGP    11
#define TMCL_RSGP    12
#define TMCL_RFS     13
#define TMCL_SIO     14
#define TMCL_GIO     15
#define TMCL_SCO     30
#define TMCL_GCO     31
#define TMCL_CCO     32

// Options for MVP command
#define MVP_ABS      0
#define MVP_REL      1
#define MVP_COORD    2

// Options for RFS command
#define RFS_START    0
#define RFS_STOP     1
#define RFS_STATUS   2

#define FALSE   0
#define TRUE    1

// Send a command
void SendCmd(UCHAR Address, UCHAR Command, UCHAR Type, UCHAR Motor, INT Value)
{
    UCHAR TxBuffer[9];
    UCHAR i;

    TxBuffer[0]=Address;
    TxBuffer[1]=Command;
    TxBuffer[2]=Type;
    TxBuffer[3]=Motor;
    TxBuffer[4]=Value >> 24;
    TxBuffer[5]=Value >> 16;
    TxBuffer[6]=Value >> 8;
    TxBuffer[7]=Value & 0xff;
    TxBuffer[8]=0;
    for(i=0; i<8; i++)
    TxBuffer[8]+=TxBuffer[i];

    // Now, send the 9 bytes stored in TxBuffer to the module
    // (this is MCU specific)
}

// Get the result
// Return TRUE when checksum of the result if okay, else return FALSE
// The follwing values are returned:
//      *Address: Host address
//      *Status: Status of the module (100 means okay)
//      *Value: Value read back by the command
UCHAR GetResult(HANDLE Handle, UCHAR *Address, UCHAR *Status, INT *Value)
{
    UCHAR RxBuffer[9], Checksum;
    DWORD Errors, BytesRead;
    COMSTAT ComStat;
    int i;

    // First, get 9 bytes from the module and store them in RxBuffer[0..8]
    // (this is MCU specific)

    // Check the checksum
    Checksum=0;
    for(i=0; i<8; i++)
        Checksum+=RxBuffer[i];

    if(Checksum!=RxBuffer[8])
        return FALSE;

    // Decode the datagram
    *Address=RxBuffer[0];
    *Status=RxBuffer[2];
    *Value=(RxBuffer[4] << 24) | (RxBuffer[5] << 16) | (RxBuffer[6] << 8) | RxBuffer[7];

    return TRUE;
}
