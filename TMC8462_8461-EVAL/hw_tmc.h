/*******************************************************************************
* Copyright © 2026 Analog Devices, Inc.
*******************************************************************************/

#ifndef HW_TMC_H_
#define HW_TMC_H_

#include "esc.h"
#include "ecat_def.h"

#define TMC846x_FPGA
//#define TEST_MEMORY_MAPPED

//TMC846x-spezifische Register
#if !defined(TMC846x_FPGA)
  #define TMC846x_ENC_MODE        0x0000
  #define TMC846x_ENC_STATUS      0x0010
  #define TMC846x_ENC_X_W         0x0020
  #define TMC846x_ENC_X_R         0x0030
  #define TMC846x_ENC_CONST       0x0040
  #define TMC846x_ENC_LATCH       0x0050

  #define TMC846x_SPI_RX_DATA     0x0060
  #define TMC846x_SPI_TX_DATA     0x0070
  #define TMC846x_SPI_CONF        0x0080
  #define TMC846x_SPI_STATUS      0x0090
  #define TMC846x_SPI_LENGTH      0x00A0
  #define TMC846x_SPI_TIME        0x00B0

  #define TMC846x_IIC_TIMEBASE    0x00C0
  #define TMC846x_IIC_CONTROL     0x00D0
  #define TMC846x_IIC_STATUS      0x00E0
  #define TMC846x_IIC_ADDRESS     0x00F0
  #define TMC846x_IIC_DATA_R      0x0100
  #define TMC846x_IIC_DATA_W      0x0110

  #define TMC846x_SD_SR0          0x0120
  #define TMC846x_SD_SR1          0x0130
  #define TMC846x_SD_SR2          0x0140
  #define TMC846x_SD_SC0          0x0150
  #define TMC846x_SD_SC1          0x0160
  #define TMC846x_SD_SC2          0x0170
  #define TMC846x_SD_ST0          0x0180
  #define TMC846x_SD_ST1          0x0190
  #define TMC846x_SD_ST2          0x01A0
  #define TMC846x_SD_SL0          0x01B0
  #define TMC846x_SD_SL1          0x01C0
  #define TMC846x_SD_SL2          0x01D0
  #define TMC846x_SD_DLY0         0x01E0
  #define TMC846x_SD_DLY1         0x01F0
  #define TMC846x_SD_DLY2         0x0200
  #define TMC846x_SD_CFG0         0x0210
  #define TMC846x_SD_CFG1         0x0220
  #define TMC846x_SD_CFG2         0x0230

  #define TMC846x_PWM_MAXCNT      0x0240
  #define TMC846x_PWM_CHOPMODE    0x0250
  #define TMC846x_PWM_ALIGNMENT   0x0260
  #define TMC846X_PWM_POLARITIES  0x0270
  #define TMC846x_PWM1            0x0280
  #define TMC846x_PWM2            0x0290
  #define TMC846x_PWM3            0x02A0
  #define TMC846x_PWM4            0x02B0
  #define TMC846x_PWM1_CNTRSHFT   0x02C0
  #define TMC846x_PWM2_CNTRSHFT   0x02D0
  #define TMC846x_PWM3_CNTRSHFT   0x02E0
  #define TMC846x_PWM4_CNTRSHFT   0x02F0
  #define TMC846x_PULSE_A         0x0300
  #define TMC846x_PULSE_B         0x0310
  #define TMC846x_PULSE_LENGTH    0x0320
  #define TMC846x_BBM_H           0x0330
  #define TMC846x_BBM_L           0x0340

  #define TMC846x_GPO             0x0350
  #define TMC846x_GPI             0x0360
  #define TMC846x_GPIO_CONFIG     0x0370

  #define TMC846x_DAC0_VAL        0x03A0
  #define TMC846x_MFCIO_IRQ_CFG   0x03B0
  #define TMC846x_MFCIO_IRQ_FLAGS 0x03C0

  //RAM-Adressen
  #define TMC846x_ESC_CFG_MFCIO_0     0x0580
  #define TMC846x_ESC_CFG_MFCIO_1     0x0581
  #define TMC846x_ESC_CFG_MFCIO_2     0x0582
  #define TMC846x_ESC_CFG_MFCIO_3     0x0583
  #define TMC846x_ESC_CFG_MFCIO_4     0x0584
  #define TMC846x_ESC_CFG_MFCIO_5     0x0585
  #define TMC846x_ESC_CFG_MFCIO_6     0x0586
  #define TMC846x_ESC_CFG_MFCIO_7     0x0587
  #define TMC846x_ESC_CFG_MFCIO_8     0x0588
  #define TMC846x_ESC_CFG_MFCIO_9     0x0589
  #define TMC846x_ESC_CFG_MFCIO_10    0x058A
  #define TMC846x_ESC_CFG_MFCIO_11    0x058B
  #define TMC846x_ESC_CFG_MFCIO_12    0x058C
  #define TMC846x_ESC_CFG_MFCIO_13    0x058D
  #define TMC846x_ESC_CFG_MFCIO_14    0x058E
  #define TMC846x_ESC_CFG_MFCIO_15    0x058F

  #define TMC846x_ESC_CFG_MFCIO_HV_0  0x0590
  #define TMC846x_ESC_CFG_MFCIO_HV_1  0x0591
  #define TMC846x_ESC_CFG_MFCIO_HV_2  0x0592
  #define TMC846x_ESC_CFG_MFCIO_HV_3  0x0593
  #define TMC846x_ESC_CFG_MFCIO_HV_4  0x0594
  #define TMC846x_ESC_CFG_MFCIO_HV_5  0x0595
  #define TMC846x_ESC_CFG_MFCIO_HV_6  0x0596
  #define TMC846x_ESC_CFG_MFCIO_HV_7  0x0597

  #define TMC846x_ESC_ENC_MODE        0x05A8
  #define TMC846x_ESC_ENC_STATUS      0x05A9
  #define TMC846x_ESC_ENC_X_W         0x05AA
  #define TMC846x_ESC_ENC_X_R         0x05AB
  #define TMC846x_ESC_ENC_CONST       0x05AC
  #define TMC846x_ESC_ENC_LATCH       0x05AD

  #define TMC846x_ESC_SPI_RX_DATA     0x05AE
  #define TMC846x_ESC_SPI_TX_DATA     0x05AF
  #define TMC846x_ESC_SPI_CONF        0x05B0
  #define TMC846x_ESC_SPI_STATUS      0x05B1
  #define TMC846x_ESC_SPI_LENGTH      0x05B2
  #define TMC846x_ESC_SPI_TIME        0x05B3

  #define TMC846x_ESC_IIC_TIMEBASE    0x05B4
  #define TMC846x_ESC_IIC_CONTROL     0x05B5
  #define TMC846x_ESC_IIC_STATUS      0x05B6
  #define TMC846x_ESC_IIC_ADDRESS     0x05B7
  #define TMC846x_ESC_IIC_DATA_R      0x05B8
  #define TMC846x_ESC_IIC_DATA_W      0x05B9

  #define TMC846x_ESC_SD_SR0          0x05BA
  #define TMC846x_ESC_SD_SR1          0x05BB
  #define TMC846x_ESC_SD_SR2          0x05BC
  #define TMC846x_ESC_SD_SC0          0x05BD
  #define TMC846x_ESC_SD_SC1          0x05BE
  #define TMC846x_ESC_SD_SC2          0x05BF
  #define TMC846x_ESC_SD_ST0          0x05C0
  #define TMC846x_ESC_SD_ST1          0x05C1
  #define TMC846x_ESC_SD_ST2          0x05C2
  #define TMC846x_ESC_SD_SL0          0x05C3
  #define TMC846x_ESC_SD_SL1          0x05C4
  #define TMC846x_ESC_SD_SL2          0x05C5
  #define TMC846x_ESC_SD_DLY0         0x05C6
  #define TMC846x_ESC_SD_DLY1         0x05C7
  #define TMC846x_ESC_SD_DLY2         0x05C8
  #define TMC846x_ESC_SD_CFG0         0x05C9
  #define TMC846x_ESC_SD_CFG1         0x05CA
  #define TMC846x_ESC_SD_CFG2         0x05CB

  #define TMC846x_ESC_PWM_MAXCNT      0x05CC
  #define TMC846x_ESC_PWM_CHOPMODE    0x05CD
  #define TMC846x_ESC_PWM_ALIGNMENT   0x05CE
  #define TMC846X_ESC_PWM_POLARITIES  0x05CF
  #define TMC846x_ESC_PWM1            0x05D0
  #define TMC846x_ESC_PWM2            0x05D1
  #define TMC846x_ESC_PWM3            0x05D2
  #define TMC846x_ESC_PWM4            0x05D3
  #define TMC846x_ESC_PWM1_CNTRSHFT   0x05D4
  #define TMC846x_ESC_PWM2_CNTRSHFT   0x05D5
  #define TMC846x_ESC_PWM3_CNTRSHFT   0x05D6
  #define TMC846x_ESC_PWM4_CNTRSHFT   0x05D7
  #define TMC846x_ESC_PULSE_A         0x05D8
  #define TMC846x_ESC_PULSE_B         0x05D9
  #define TMC846x_ESC_PULSE_LENGTH    0x05DA
  #define TMC846x_ESC_BBM_H           0x05DB
  #define TMC846x_ESC_BBM_L           0x05DC

  #define TMC846x_ESC_GPO             0x05DD
  #define TMC846x_ESC_GPI             0x05DE

  //#define TMC846x_WD_CFG          0x0EA2
  //#define TMC846x_WD_STATUS       0x0EA3
  //#define TMC846x_WD_INPUT_T      0x0EA3
  //#define TMC846x_WD_OUTPUT_T     0x0EA4

  //#define TMC846x_CLKOUT_CFG      0x0E9E
  #define TMC846x_DAC0            0x05E2

  #define TMC846x_IRQ_MASK        0x05E3
  #define TMC846x_IRQ_FLAGS       0x05E4

  #define TMC846x_MFCFG_INPUT       0
  #define TMC846x_MFCFG_LOW         1
  #define TMC846x_MFCFG_HIGH        2
  #define TMC846x_MFCFG_TRI         3
  #define TMC846x_MFCFG_ENCA_P      4
  #define TMC846x_MFCFG_ENCA_N      5
  #define TMC846x_MFCFG_ENCB_P      6
  #define TMC846x_MFCFG_ENCB_N      7
  #define TMC846x_MFCFG_ENCN_P      8
  #define TMC846x_MFCFG_ENCN_N      9
  #define TMC846x_MFCFG_SPI_SCK    10
  #define TMC846x_MFCFG_SPI_SDI    11
  #define TMC846x_MFCFG_SPI_SDO    12
  #define TMC846x_MFCFG_SPI_CSN0   13
  #define TMC846x_MFCFG_SPI_CSN1   14
  #define TMC846x_MFCFG_SPI_CSN2   15
  #define TMC846x_MFCFG_SPI_CSN3   16
  #define TMC846x_MFCFG_IIC_SCL    17
  #define TMC846x_MFCFG_IIC_SDA    18
  #define TMC846x_MFCFG_SD_STEP0   19
  #define TMC846x_MFCFG_SD_DIR0    20
  #define TMC846x_MFCFG_SD_STEP1   21
  #define TMC846x_MFCFG_SD_DIR1    22
  #define TMC846x_MFCFG_SD_STEP2   23
  #define TMC846x_MFCFG_SD_DIR2    24
  #define TMC846x_MFCFG_SD_STEP0_N 25
  #define TMC846x_MFCFG_SD_DIR0_N  26
  #define TMC846x_MFCFG_SD_STEP1_N 27
  #define TMC846x_MFCFG_SD_DIR1_N  28
  #define TMC846x_MFCFG_SD_STEP2_N 29
  #define TMC846x_MFCFG_SD_DIR2_N  30
  #define TMC846x_MFCFG_PWM_HS0    31
  #define TMC846x_MFCFG_PWM_LS0    32
  #define TMC846x_MFCFG_PWM_HS1    33
  #define TMC846x_MFCFG_PWM_LS1    34
  #define TMC846x_MFCFG_PWM_HS2    35
  #define TMC846x_MFCFG_PWM_LS2    36
  #define TMC846x_MFCFG_PWM_HS3    37
  #define TMC846x_MFCFG_PWM_LS4    38
  #define TMC846x_MFCFG_GPI0       39
  #define TMC846x_MFCFG_GPI1       40
  #define TMC846x_MFCFG_GPI2       41
  #define TMC846x_MFCFG_GPI3       42
  #define TMC846x_MFCFG_GPI4       43
  #define TMC846x_MFCFG_GPI5       44
  #define TMC846x_MFCFG_GPI6       45
  #define TMC846x_MFCFG_GPI7       46
  #define TMC846x_MFCFG_GPI8       47
  #define TMC846x_MFCFG_GPI9       48
  #define TMC846x_MFCFG_GPI10      49
  #define TMC846x_MFCFG_GPI11      50
  #define TMC846x_MFCFG_GPI12      51
  #define TMC846x_MFCFG_GPI13      52
  #define TMC846x_MFCFG_GPI14      53
  #define TMC846x_MFCFG_GPI15      54
  #define TMC846x_MFCFG_GPO0       55
  #define TMC846x_MFCFG_GPO1       56
  #define TMC846x_MFCFG_GPO2       57
  #define TMC846x_MFCFG_GPO3       58
  #define TMC846x_MFCFG_GPO4       59
  #define TMC846x_MFCFG_GPO5       60
  #define TMC846x_MFCFG_GPO6       61
  #define TMC846x_MFCFG_GPO7       62
  #define TMC846x_MFCFG_GPO8       63
  #define TMC846x_MFCFG_GPO9       64
  #define TMC846x_MFCFG_GPO10      65
  #define TMC846x_MFCFG_GPO11      66
  #define TMC846x_MFCFG_GPO12      67
  #define TMC846x_MFCFG_GPO13      68
  #define TMC846x_MFCFG_GPO14      69
  #define TMC846x_MFCFG_GPO15      70
  #define TMC846x_MFCFG_DAC0       71
  #define TMC846x_MFCFG_DAC1       72
  #define TMC846x_MFCFG_DAC2       73
  #define TMC846x_MFCFG_PWM_OVC_O        74
  #define TMC846x_MFCFG_PWM_PULSE_A      75
  #define TMC846x_MFCFG_PWM_PULSE_CENTER 76
  #define TMC846x_MFCFG_PWM_PULSE_B      77
  #define TMC846x_MFCFG_PWM_PULSE_AB     78
  #define TMC846x_MFCFG_PWM_PULSE_ZERO   79

#else

  #define TMC846x_ENC_MODE        0x0000
  #define TMC846x_ENC_STATUS      0x0010
  #define TMC846x_ENC_X_W         0x0020
  #define TMC846x_ENC_X_R         0x0030
  #define TMC846x_ENC_CONST       0x0040
  #define TMC846x_ENC_LATCH       0x0050

  #define TMC846x_SPI_RX_DATA     0x0060
  #define TMC846x_SPI_TX_DATA     0x0070
  #define TMC846x_SPI_CONF        0x0080
  #define TMC846x_SPI_STATUS      0x0090
  #define TMC846x_SPI_LENGTH      0x00A0
  #define TMC846x_SPI_TIME        0x00B0

  #define TMC846x_SD_SR0          0x00C0
  #define TMC846x_SD_SC0          0x00D0
  #define TMC846x_SD_ST0          0x00E0
  #define TMC846x_SD_SL0          0x00F0
  #define TMC846x_SD_DLY0         0x0100
  #define TMC846x_SD_CFG0         0x0110

  #define TMC846x_PWM_MAXCNT      0x0120
  #define TMC846x_PWM_CHOPMODE    0x0130
  #define TMC846x_PWM_ALIGNMENT   0x0140
  #define TMC846X_PWM_POLARITIES  0x0150
  #define TMC846x_PWM1            0x0160
  #define TMC846x_PWM2            0x0170
  #define TMC846x_PWM3            0x0180
  #define TMC846x_PWM1_CNTRSHFT   0x0190
  #define TMC846x_PWM2_CNTRSHFT   0x01A0
  #define TMC846x_PWM3_CNTRSHFT   0x01B0
  #define TMC846x_PULSE_A         0x01C0
  #define TMC846x_PULSE_B         0x01D0
  #define TMC846x_PULSE_LENGTH    0x01E0
  #define TMC846x_BBM_H           0x01F0
  #define TMC846x_BBM_L           0x0200

  #define TMC846x_GPO_OUT_VAL     0x0210
  #define TMC846x_GPI_IN_VAL      0x0220
  #define TMC846x_GPIO_CONFIG     0x0230

  #define TMC846x_MFCIO_IRQ_CFG   0x0240
  #define TMC846x_MFCIO_IRQ_FLAGS 0x0250

  #define TMC846x_WD_TIME         0x0260
  #define TMC846x_WD_CFG          0x0270
  #define TMC846x_WD_OUT_MASK_POL 0x0280
  #define TMC846x_WD_OE_POL       0x0290
  #define TMC846x_WD_IN_MASK_POL  0x02A0
  #define TMC846x_WD_MAX          0x02B0

  #define TMC846x_AL_STATE_OVERRIDE 0x02C0

  #define TMC846x_MAPCFG_ENC_MODE           0x0580
  #define TMC846x_MAPCFG_ENC_STATUS         0x0581
  #define TMC846x_MAPCFG_ENC_X_W            0x0582
  #define TMC846x_MAPCFG_ENC_X_R            0x0583
  #define TMC846x_MAPCFG_ENC_CONST          0x0584
  #define TMC846x_MAPCFG_ENC_LATCH          0x0585
  #define TMC846x_MAPCFG_SPI_RX_DATA        0x0586
  #define TMC846x_MAPCFG_SPI_TX_DATA        0x0587
  #define TMC846x_MAPCFG_SPI_CONF           0x0588
  #define TMC846x_MAPCFG_SPI_STATUS         0x0589
  #define TMC846x_MAPCFG_SPI_LENGTH         0x058A
  #define TMC846x_MAPCFG_SPI_TIME           0x058B
  #define TMC846x_MAPCFG_SD_SR              0x058C
  #define TMC846x_MAPCFG_SD_SC              0x058D
  #define TMC846x_MAPCFG_SD_ST              0x058E
  #define TMC846x_MAPCFG_SD_SL              0x058F
  #define TMC846x_MAPCFG_SD_DLY             0x0590
  #define TMC846x_MAPCFG_SD_CFG             0x0591
  #define TMC846x_MAPCFG_PWM_MAXCNT         0x0592
  #define TMC846x_MAPCFG_PWM_CHOPMODE       0x0593
  #define TMC846x_MAPCFG_PWM_ALIGNMENT      0x0594
  #define TMC846x_MAPCFG_PWM_POLARITIES     0x0595
  #define TMC846x_MAPCFG_PWM_VALUE_1        0x0596
  #define TMC846x_MAPCFG_PWM_VALUE_2        0x0597
  #define TMC846x_MAPCFG_PWM_VALUE_3        0x0598
  #define TMC846x_MAPCFG_PWM_CNTRSHIFT_1    0x0599
  #define TMC846x_MAPCFG_PWM_CNTRSHIFT_2    0x059A
  #define TMC846x_MAPCFG_PWM_CNTRSHIFT_3    0x059B
  #define TMC846x_MAPCFG_PWM_PULSE_A        0x059C
  #define TMC846x_MAPCFG_PWM_PULSE_B        0x059D
  #define TMC846x_MAPCFG_PWM_PULSE_LENGTH   0x059E
  #define TMC846x_MAPCFG_PWM_BBM_H          0x059F
  #define TMC846x_MAPCFG_PWM_BBM_L          0x05A0
  #define TMC846x_MAPCFG_GPO_OUT_VAL        0x05A1
  #define TMC846x_MAPCFG_GPI_IN_VAL         0x05A2
  #define TMC846x_MAPCFG_GPIO_CONFIG        0x05A3
  #define TMC846x_MAPCFG_IRQ_CFG            0x05A4
  #define TMC846x_MAPCFG_IRQ_FLAGS          0x05A5
  #define TMC846x_MAPCFG_WD_TIME            0x05A6
  #define TMC846x_MAPCFG_WD_CFG             0x05A7
  #define TMC846x_MAPCFG_WD_OUT_MASK_POL    0x05A8
  #define TMC846x_MAPCFG_WD_OE_POL          0x05A9
  #define TMC846x_MAPCFG_WD_IN_MASK_POL     0x05AA
  #define TMC846x_MAPCFG_WD_MAX             0x05AB

  #define TMC846x_MAPCFG_ECAT_WRITE           0x80
  #define TMC846x_MAPCFG_TRIGGER_SYNC0        0x01
  #define TMC846x_MAPCFG_TRIGGER_SYNC1        0x02
  #define TMC846x_MAPCFG_TRIGGER_SOF          0x03
  #define TMC846x_MAPCFG_TRIGGER_EOF          0x04
  #define TMC846x_MAPCFG_TRIGGER_CHIPSEL      0x05
  #define TMC846x_MAPCFG_TRIGGER_CHIPDESEL    0x06
  #define TMC846x_MAPCFG_TRIGGER_LATCH0       0x07
  #define TMC846x_MAPCFG_TRIGGER_LATCH1       0x08
  #define TMC846x_MAPCFG_TRIGGER_UNUSED0      0x09
  #define TMC846x_MAPCFG_TRIGGER_UNUSED1      0x0A
  #define TMC846x_MAPCFG_TRIGGER_BEFORE       0x0B
  #define TMC846x_MAPCFG_TRIGGER_AFTER        0x0C
  #define TMC846x_MAPCFG_TRIGGER_UNUSED2      0x0D
  #define TMC846x_MAPCFG_TRIGGER_2ND_CLOCK    0x0E
  #define TMC846x_MAPCFG_TRIGGER_IMMEDIATELY  0x0F

  #define TMC846x_MEMADDR_ENC_MODE           0x4000
  #define TMC846x_MEMADDR_ENC_X_W            0x4004
  #define TMC846x_MEMADDR_ENC_CONST          0x4008
  #define TMC846x_MEMADDR_SPI_TX_DATA        0x400C
  #define TMC846x_MEMADDR_SPI_CONF           0x4014
  #define TMC846x_MEMADDR_SPI_LENGTH         0x4016
  #define TMC846x_MEMADDR_SPI_TIME           0x4017
  #define TMC846x_MEMADDR_SD_SR              0x4018
  #define TMC846x_MEMADDR_SD_ST              0x401C
  #define TMC846x_MEMADDR_SD_SL              0x4020
  #define TMC846x_MEMADDR_SD_DLY             0x4022
  #define TMC846x_MEMADDR_SD_CFG             0x4024
  #define TMC846x_MEMADDR_PWM_MAXCNT         0x4026
  #define TMC846x_MEMADDR_PWM_CHOPMODE       0x4028
  #define TMC846x_MEMADDR_PWM_ALIGNMENT      0x402A
  #define TMC846x_MEMADDR_PWM_POLARITIES     0x402B
  #define TMC846x_MEMADDR_PWM_VALUE_1        0x402C
  #define TMC846x_MEMADDR_PWM_VALUE_2        0x402E
  #define TMC846x_MEMADDR_PWM_VALUE_3        0x4030
  #define TMC846x_MEMADDR_PWM_CNTRSHIFT_1    0x4032
  #define TMC846x_MEMADDR_PWM_CNTRSHIFT_2    0x4034
  #define TMC846x_MEMADDR_PWM_CNTRSHIFT_3    0x4036
  #define TMC846x_MEMADDR_PWM_PULSE_A        0x4038
  #define TMC846x_MEMADDR_PWM_PULSE_B        0x403A
  #define TMC846x_MEMADDR_PWM_PULSE_LENGTH   0x403C
  #define TMC846x_MEMADDR_PWM_BBM_H          0x403D
  #define TMC846x_MEMADDR_PWM_BBM_L          0x403E
  #define TMC846x_MEMADDR_GPO_OUT_VAL        0x4040
  #define TMC846x_MEMADDR_GPIO_CONFIG        0x4042
  #define TMC846x_MEMADDR_IRQ_CFG            0x4044
  #define TMC846x_MEMADDR_WD_TIME            0x4048
  #define TMC846x_MEMADDR_WD_CFG             0x404C
  #define TMC846x_MEMADDR_WD_OUT_MASK_POL    0x4050
  #define TMC846x_MEMADDR_WD_OE_POL          0x4058
  #define TMC846x_MEMADDR_WD_IN_MASK_POL     0x405C

  #define TMC846x_MEMADDR_ENC_STATUS         0x4800
  #define TMC846x_MEMADDR_ENC_X_R            0x4804
  #define TMC846x_MEMADDR_ENC_LATCH          0x4808
  #define TMC846x_MEMADDR_SPI_RX_DATA        0x480C
  #define TMC846x_MEMADDR_SPI_STATUS         0x4814
  #define TMC846x_MEMADDR_SD_SC              0x4818
  #define TMC846x_MEMADDR_GPI_IN_VAL         0x481C
  #define TMC846x_MEMADDR_IRQ_FLAGS          0x481E
  #define TMC846x_MEMADDR_WD_MAX             0x4820
#endif

#define TMC846x_


#define TMC846x_SDCFG_DISABLE      1
#define TMC846x_SDCFG_CONTINOUS   2
#define TMC846x_SDCFG_STEP_POL    4
#define TMC846x_SDCFG_DIR_POL     8
#define TMC846x_SDCFG_COUNT_CLEAR 16
#define TMC846x_SDCFG_TOGGLE      32

#define TMC846x_AL_OVR_SPI        1
#define TMC846x_AL_OVR_SD         2
#define TMC846x_AL_OVR_PWM        4
#define TMC846x_AL_OVR_GPIO       8

#define DISABLE_ESC_INT()   MSS_GPIO_disable_irq(MSS_GPIO_4)
#define ENABLE_ESC_INT()    MSS_GPIO_enable_irq(MSS_GPIO_4)
#define DISABLE_SYNC0_INT() MSS_GPIO_disable_irq(MSS_GPIO_2)
#define ENABLE_SYNC0_INT()  MSS_GPIO_enable_irq(MSS_GPIO_2)
#define DISABLE_SYNC1_INT() MSS_GPIO_disable_irq(MSS_GPIO_1)
#define ENABLE_SYNC1_INT()  MSS_GPIO_enable_irq(MSS_GPIO_1)

#define HW_EscReadByte(WordValue, Address) HW_EscRead(((MEM_ADDR *)&(WordValue)),((UINT16)(Address)),1) /**< \brief 8Bit ESC read access*/
#define HW_EscReadWord(WordValue, Address) HW_EscRead(((MEM_ADDR *)&(WordValue)),((UINT16)(Address)),2) /**< \brief 16Bit ESC read access*/
#define HW_EscReadDWord(DWordValue, Address) HW_EscRead(((MEM_ADDR *)&(DWordValue)),((UINT16)(Address)),4) /**< \brief 32Bit ESC read access*/
#define HW_EscReadMbxMem(pData,Address,Len) HW_EscRead(((MEM_ADDR *)(pData)),((UINT16)(Address)),(Len)) /**< \brief The mailbox data is stored in the local uC memory therefore the default read function is used.*/

#define HW_EscReadByteIsr(WordValue, Address) HW_EscReadIsr(((MEM_ADDR *)&(WordValue)),((UINT16)(Address)),1) /**< \brief Interrupt specific 8Bit ESC read access*/
#define HW_EscReadWordIsr(WordValue, Address) HW_EscReadIsr(((MEM_ADDR *)&(WordValue)),((UINT16)(Address)),2) /**< \brief Interrupt specific 16Bit ESC read access*/
#define HW_EscReadDWordIsr(DWordValue, Address) HW_EscReadIsr(((MEM_ADDR *)&(DWordValue)),((UINT16)(Address)),4) /**< \brief Interrupt specific 32Bit ESC read access*/

#define HW_EscWriteByte(WordValue, Address) HW_EscWrite(((MEM_ADDR *)&(WordValue)),((UINT16)(Address)),1) /**< \brief 8Bit ESC write access*/
#define HW_EscWriteWord(WordValue, Address) HW_EscWrite(((MEM_ADDR *)&(WordValue)),((UINT16)(Address)),2) /**< \brief 16Bit ESC write access*/
#define HW_EscWriteDWord(DWordValue, Address) HW_EscWrite(((MEM_ADDR *)&(DWordValue)),((UINT16)(Address)),4) /**< \brief 32Bit ESC write access*/
#define HW_EscWriteMbxMem(pData,Address,Len) HW_EscWrite(((MEM_ADDR *)(pData)),((UINT16)(Address)),(Len)) /**< \brief The mailbox data is stored in the local uC memory therefore the default write function is used.*/

#define HW_EscWriteByteIsr(WordValue, Address) HW_EscWriteIsr(((MEM_ADDR *)&(WordValue)),((UINT16)(Address)),1) /**< \brief Interrupt specific 8Bit ESC write access*/
#define HW_EscWriteWordIsr(WordValue, Address) HW_EscWriteIsr(((MEM_ADDR *)&(WordValue)),((UINT16)(Address)),2) /**< \brief Interrupt specific 16Bit ESC write access*/
#define HW_EscWriteDWordIsr(DWordValue, Address) HW_EscWriteIsr(((MEM_ADDR *)&(DWordValue)),((UINT16)(Address)),4) /**< \brief Interrupt specific 32Bit ESC write access*/

UINT16 HW_Init(void);
void HW_Release(void);
void HW_EscRead(MEM_ADDR *pData, UINT16 Address, UINT16 Len);
void HW_EscReadIsr(MEM_ADDR *pData, UINT16 Address, UINT16 Len);
void HW_EscWrite(MEM_ADDR *pData, UINT16 Address, UINT16 Len);
void HW_EscWriteIsr(MEM_ADDR *pData, UINT16 Address, UINT16 Len);
UINT16 HW_GetALEventRegister(void);
UINT16 HW_GetALEventRegister_Isr(void);
void HW_ResetALEventMask(UINT16 intMask);
void HW_SetALEventMask(UINT16 intMask);

void HW_SetLed(UINT8 RunLed, UINT8 ErrLed);
void HW_DisableSyncManChannel(UINT8 Channel);
void HW_EnableSyncManChannel(UINT8 Channel);
TSYNCMAN *HW_GetSyncMan(UINT8 Channel);
UINT32 HW_GetTimer(void);
void HW_ClearTimer(void);


#define HW_MfcReadByte(WordValue, Address) HW_MfcRead(((MEM_ADDR *)&(WordValue)),((UINT16)(Address)),1)
#define HW_MfcReadWord(WordValue, Address) HW_MfcRead(((MEM_ADDR *)&(WordValue)),((UINT16)(Address)),2)
#define HW_MfcReadDWord(DWordValue, Address) HW_MfcRead(((MEM_ADDR *)&(DWordValue)),((UINT16)(Address)),4)

#define HW_MfcReadByteIsr(WordValue, Address) HW_MfcReadIsr(((MEM_ADDR *)&(WordValue)),((UINT16)(Address)),1)
#define HW_MfcReadWordIsr(WordValue, Address) HW_MfcReadIsr(((MEM_ADDR *)&(WordValue)),((UINT16)(Address)),2)
#define HW_MfcReadDWordIsr(DWordValue, Address) HW_MfcReadIsr(((MEM_ADDR *)&(DWordValue)),((UINT16)(Address)),4)

#define HW_MfcWriteByte(WordValue, Address) HW_MfcWrite(((MEM_ADDR *)&(WordValue)),((UINT16)(Address)),1)
#define HW_MfcWriteWord(WordValue, Address) HW_MfcWrite(((MEM_ADDR *)&(WordValue)),((UINT16)(Address)),2)
#define HW_MfcWriteDWord(DWordValue, Address) HW_MfcWrite(((MEM_ADDR *)&(DWordValue)),((UINT16)(Address)),4)

#define HW_MfcWriteByteIsr(WordValue, Address) HW_MfcWriteIsr(((MEM_ADDR *)&(WordValue)),((UINT16)(Address)),1)
#define HW_MfcWriteWordIsr(WordValue, Address) HW_MfcWriteIsr(((MEM_ADDR *)&(WordValue)),((UINT16)(Address)),2)
#define HW_MfcWriteDWordIsr(DWordValue, Address) HW_MfcWriteIsr(((MEM_ADDR *)&(DWordValue)),((UINT16)(Address)),4)

void HW_MfcRead(UINT16 Address, UINT32 Size, UCHAR *Data);
void HW_MfcWrite(UINT16 Address, UINT32 Size, UCHAR *Data);
void HW_MfcReadIsr(UINT16 Address, UINT32 Size, UCHAR *Data);
void HW_MfcWriteIsr(UINT16 Address, UINT32 Size, UCHAR *Data);
void TMC_MfcWriteByte(UINT16 Address, UCHAR Value);
void TMC_MfcWriteWord(UINT16 Address, USHORT Value);
void TMC_MfcWriteDWord(UINT16 Address, UINT32 Value);
void TMC_MfcWriteArray(UINT16 Address, UCHAR *Array, UINT32 Length);
UCHAR TMC_MfcReadByte(UINT16 Address);
USHORT TMC_MfcReadWord(UINT16 Address);
UINT32 TMC_MfcReadDWord(UINT16 Address);
void TMC_MfcReadArray(UINT16 Address, UCHAR *Array, UINT32 Length);
void TMC_EscWriteByte(UINT16 Address, UCHAR Value);
void TMC_EscWriteWord(UINT16 Address, USHORT Value);
void TMC_EscWriteDWord(UINT16 Address, UINT32 Value);
UCHAR TMC_EscReadByte(UINT16 Address);
USHORT TMC_EscReadWord(UINT16 Address);
UINT32 TMC_EscReadDWord(UINT16 Address);

int SDGetActualPosition(UCHAR Axis);
void SDMoveToPosition(UCHAR Axis, int TargetPosition, int Speed);
void SDRotate(UCHAR Axis, int Velocity);

#endif /* HW_TMC_H_ */
