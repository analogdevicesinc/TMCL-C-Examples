/*******************************************************************************
* Copyright © 2026 Analog Devices, Inc.
*******************************************************************************/

#include "ecat_def.h"
#include "esc.h"
#include "hw_tmc.h"

#if defined(MK20DN512)
  #include "derivative.h"
  #include "bits.h"
  #include "TMCM-K20.h"
#elif defined(GD32F425)
  #include "gd32f4xx.h"
  #include "bits.h"
  #include "TMCM-GD4.h"
#else
  #include "stm32f2xx.h"
  #include "bits.h"
  #include "TMCM-F2.h"
#endif

#include "SPI-K20.h"
#include "SysTick-K20.h"

#define TMCESC_CMD_READ      0x02
#define TMCESC_CMD_READ_WAIT 0x03
#define TMCESC_CMD_WRITE     0x04
#define TMCESC_CMD_ADDR_EXT  0x06

#define __disable_irq() asm volatile("CPSID I\n")
#define __enable_irq()  asm volatile("CPSIE I\n")

extern volatile UINT32 ECATTimer;
TSYNCMAN TmpSyncMan;
volatile UINT16 AlEventMask;

extern void PDI_Isr(void);
extern void Sync0_Isr(void);
extern void Sync1_Isr(void);


UINT16 HW_Init(void)
{
  UINT Delay;
  USHORT InterfaceType;
  UCHAR ErrorFlag;

  //Reset des ESC
  #if defined(MK20DN512)
  GPIOC_PCOR=BIT15;
  Delay=GetSysTimer();
  while(abs(GetSysTimer()-Delay)<5);
  GPIOC_PSOR=BIT15;
  #elif defined(GD32F425)
  GPIO_BC(GPIOD)=BIT15;
  Delay=GetSysTimer();
  while(abs(GetSysTimer()-Delay)<5);
  GPIO_BOP(GPIOD)=BIT15;
  #else
  GPIOD->BSRRH=BIT7;
  Delay=GetSysTimer();
  while(abs(GetSysTimer()-Delay)<5);
  GPIOD->BSRRL=BIT7;
  #endif

  ErrorFlag=FALSE;
  
  //Warten auf das EEPROM_LOADED-Signal
  Delay=GetSysTimer();
  #if defined(MK20DN512)
  while(!(GPIOD_PDIR & BIT0))
  #elif defined(GD32F425)
  while(!(GPIO_ISTAT(GPIOD) & BIT12))
  #else
  while(!(GPIOD->IDR & BIT4))
  #endif
  {
  	if(abs(GetSysTimer()-Delay)>500)
  	{
  		ErrorFlag=TRUE;
  		break;
  	}
  }

  //Warten auf den ESC (Interface-Typ so lange lesen, bis dieser 0x05 (SPI) ist)
  Delay=GetSysTimer();
  do
  {
  	if(abs(GetSysTimer()-Delay)>500)
  	{
  		ErrorFlag=TRUE;
  		break;
  	}
  	
    HW_EscReadWord(InterfaceType, 0x140);
  } while((InterfaceType & 0xff)!=0x05);

  //Ein Dummy-Zugriff auf das zweite SPI
  TMC_MfcReadByte(TMC846x_AL_STATE_OVERRIDE);

//  ENABLE_SYNC0_INT();
//  ENABLE_SYNC1_INT();

  //SPI und GPIO für alle Zustände freischalten
  TMC_MfcWriteByte(TMC846x_AL_STATE_OVERRIDE, TMC846x_AL_OVR_GPIO|TMC846x_AL_OVR_SPI);

  //SPI erstmalig initialisieren
//  TMC_MfcWriteByte(TMC846x_SPI_TIME, 1);
//  TMC_MfcWriteWord(TMC846x_SPI_CONF, 0x0060);
//  TMC_MfcWriteByte(TMC846x_SPI_LENGTH, 39);

  return ErrorFlag;
}

void HW_Release(void)
{

}

void HW_EscRead(MEM_ADDR *pData, UINT16 Address, UINT16 Len)
{
  UINT i;
  UCHAR *TmpData;

  __disable_irq();

  TmpData=(UINT8 *) pData;
  ReadWriteSPI(SPI_DEV_ECAT, Address >> 5, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT, (Address << 3) | TMCESC_CMD_ADDR_EXT, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT, ((Address >> 8) & 0xE0) | (TMCESC_CMD_READ_WAIT << 2), FALSE);
  ReadWriteSPI(SPI_DEV_ECAT, 0xff, FALSE);

  for(i=0; i<Len; i++) TmpData[i]=ReadWriteSPI(SPI_DEV_ECAT, (i<Len-1) ? 0x00 : 0xff, (i<Len-1) ? FALSE : TRUE);

  __enable_irq();
}

void HW_EscReadIsr(MEM_ADDR *pData, UINT16 Address, UINT16 Len)
{
  UINT i;
  UCHAR *TmpData;

  TmpData=(UINT8 *) pData;
  ReadWriteSPI(SPI_DEV_ECAT, Address >> 5, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT, (Address << 3) | TMCESC_CMD_ADDR_EXT, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT, ((Address >> 8) & 0xE0) | (TMCESC_CMD_READ_WAIT << 2), FALSE);
  ReadWriteSPI(SPI_DEV_ECAT, 0xff, FALSE);

  for(i=0; i<Len; i++) TmpData[i]=ReadWriteSPI(SPI_DEV_ECAT, (i<Len-1) ? 0x00 : 0xff, (i<Len-1) ? FALSE : TRUE);
}

void HW_EscWrite(MEM_ADDR *pData, UINT16 Address, UINT16 Len)
{
  UINT i;
  UCHAR *TmpData;

  __disable_irq();

  TmpData=(UINT8 *) pData;

  ReadWriteSPI(SPI_DEV_ECAT, Address >> 5, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT, (Address << 3) | TMCESC_CMD_ADDR_EXT, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT, ((Address >> 8) & 0xE0) | (TMCESC_CMD_WRITE << 2), FALSE);

  for(i=0; i<Len; i++) ReadWriteSPI(SPI_DEV_ECAT, TmpData[i], (i<Len-1) ? FALSE : TRUE);

  __enable_irq();
}

void HW_EscWriteIsr(MEM_ADDR *pData, UINT16 Address, UINT16 Len)
{
  UINT i;
  UCHAR *TmpData;

  TmpData=(UCHAR *) pData;

  ReadWriteSPI(SPI_DEV_ECAT, Address >> 5, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT, (Address << 3) | TMCESC_CMD_ADDR_EXT, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT, ((Address >> 8) & 0xE0) | (TMCESC_CMD_WRITE << 2), FALSE);

  for(i=0; i<Len; i++) ReadWriteSPI(SPI_DEV_ECAT, TmpData[i], (i<Len-1) ? FALSE : TRUE);
}

UINT16 HW_GetALEventRegister(void)
{
  UCHAR Low;
  UCHAR High;

  __disable_irq();

  Low=ReadWriteSPI(SPI_DEV_ECAT, 0, FALSE);
  High=ReadWriteSPI(SPI_DEV_ECAT, TMCESC_CMD_READ, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT, 0xff, TRUE);

  __enable_irq();

  return (High << 8)|Low;
}

UINT16 HW_GetALEventRegister_Isr(void)
{
  UCHAR Low;
  UCHAR High;

  Low=ReadWriteSPI(SPI_DEV_ECAT, 0, FALSE);
  High=ReadWriteSPI(SPI_DEV_ECAT, 0, TRUE);

  return (High << 8)|Low;
}

void HW_ResetALEventMask(UINT16 intMask)
{
    UINT16 mask;

    HW_EscReadWord(mask, ESC_AL_EVENTMASK_OFFSET);

    mask &= intMask;
//    DISABLE_AL_EVENT_INT;
    HW_EscWriteWord(mask, ESC_AL_EVENTMASK_OFFSET);
    HW_EscReadWord(AlEventMask, ESC_AL_EVENTMASK_OFFSET);
//    ENABLE_AL_EVENT_INT;

}

void HW_SetALEventMask(UINT16 intMask)
{
    UINT16 mask;

    HW_EscReadWord(mask, ESC_AL_EVENTMASK_OFFSET);

    mask |= intMask;
//    DISABLE_AL_EVENT_INT;
    HW_EscWriteWord(mask, ESC_AL_EVENTMASK_OFFSET);
    HW_EscReadWord(AlEventMask, ESC_AL_EVENTMASK_OFFSET);
//    ENABLE_AL_EVENT_INT;

}


void HW_SetLed(UINT8 RunLed, UINT8 ErrLed)
{
/*  if(RunLed)
    LED_ON();
  else
    LED_OFF();

  if(ErrLed)
    LED_ERROR_ON();
  else
    LED_ERROR_OFF();
*/    
}

void HW_DisableSyncManChannel(UINT8 Channel)
{
  UINT16 Offset;
  UINT8 smStatus;

  smStatus = SM_SETTING_PDI_DISABLE;
  Offset = (ESC_SM_PDICONTROL_OFFSET + (SIZEOF_SM_REGISTER*Channel));

  HW_EscWriteByte(smStatus, Offset);

  //wait until SyncManager is disabled
 do
  {
      HW_EscReadByte(smStatus, Offset);
  }
  while(!(smStatus & SM_SETTING_PDI_DISABLE));
}

void HW_EnableSyncManChannel(UINT8 Channel)
{
  UINT16 Offset;
  UINT8 smStatus;

  smStatus = 0;
  Offset = (ESC_SM_PDICONTROL_OFFSET + (SIZEOF_SM_REGISTER*Channel));

  HW_EscWriteByte(smStatus, Offset);

  //wait until SyncManager is enabled
  do
  {
      HW_EscReadByte(smStatus, Offset);
  }
  while((smStatus & SM_SETTING_PDI_DISABLE));
}

TSYNCMAN *HW_GetSyncMan(UINT8 Channel)
{
  HW_EscRead((MEM_ADDR *) &TmpSyncMan, ESC_SYNCMAN_REG_OFFSET+Channel*SIZEOF_SM_REGISTER, SIZEOF_SM_REGISTER);
  return &TmpSyncMan;
}

UINT32 HW_GetTimer(void)
{
  return ECATTimer;
}

void HW_ClearTimer(void)
{
  ECATTimer=0;
}

void HW_MfcRead(UINT16 Address, UINT32 Size, UCHAR *Data)
{
  UINT32 i;

  __disable_irq();
  ReadWriteSPI(SPI_DEV_ECAT_MFC, Address >> 5, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, (Address << 3) | TMCESC_CMD_ADDR_EXT, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, ((Address >> 8) & 0xE0) | (TMCESC_CMD_READ_WAIT << 2), FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, 0xff, FALSE);

  for(i=0; i<Size; i++) Data[i]=ReadWriteSPI(SPI_DEV_ECAT_MFC, (i<Size-1) ? 0x00 : 0xff, (i<Size-1) ? FALSE : TRUE);
  __enable_irq();
}

void HW_MfcWrite(UINT16 Address, UINT32 Size, UCHAR *Data)
{
  UINT32 i;

  __disable_irq();
  ReadWriteSPI(SPI_DEV_ECAT_MFC, Address >> 5, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, (Address << 3) | TMCESC_CMD_ADDR_EXT, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, ((Address >> 8) & 0xE0) | (TMCESC_CMD_WRITE << 2), FALSE);

  for(i=0; i<Size; i++) ReadWriteSPI(SPI_DEV_ECAT_MFC, Data[i], (i<Size-1) ? FALSE : TRUE);
  __enable_irq();
}

void HW_MfcReadIsr(UINT16 Address, UINT32 Size, UCHAR *Data)
{
  UINT32 i;

  ReadWriteSPI(SPI_DEV_ECAT_MFC, Address >> 5, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, (Address << 3) | TMCESC_CMD_ADDR_EXT, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, ((Address >> 8) & 0xE0) | (TMCESC_CMD_READ_WAIT << 2), FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, 0xff, FALSE);

  for(i=0; i<Size; i++) Data[i]=ReadWriteSPI(SPI_DEV_ECAT_MFC, (i<Size-1) ? 0x00 : 0xff, (i<Size-1) ? FALSE : TRUE);
}

void HW_MfcWriteIsr(UINT16 Address, UINT32 Size, UCHAR *Data)
{
  uint32_t i;

  ReadWriteSPI(SPI_DEV_ECAT_MFC, Address >> 5, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, (Address << 3) | TMCESC_CMD_ADDR_EXT, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, ((Address >> 8) & 0xE0) | (TMCESC_CMD_WRITE << 2), FALSE);

  for(i=0; i<Size; i++) ReadWriteSPI(SPI_DEV_ECAT_MFC, Data[i], (i<Size-1) ? FALSE : TRUE);
}

void TMC_MfcWriteByte(UINT16 Address, UCHAR Value)
{
  __disable_irq();
   ReadWriteSPI(SPI_DEV_ECAT_MFC, Address >> 5, FALSE);
   ReadWriteSPI(SPI_DEV_ECAT_MFC, (Address << 3) | TMCESC_CMD_ADDR_EXT, FALSE);
   ReadWriteSPI(SPI_DEV_ECAT_MFC, ((Address >> 8) & 0xE0) | (TMCESC_CMD_WRITE << 2), FALSE);

   ReadWriteSPI(SPI_DEV_ECAT_MFC, Value, TRUE);
   __enable_irq();
}

void TMC_MfcWriteWord(UINT16 Address, USHORT Value)
{
  __disable_irq();
   ReadWriteSPI(SPI_DEV_ECAT_MFC, Address >> 5, FALSE);
   ReadWriteSPI(SPI_DEV_ECAT_MFC, (Address << 3) | TMCESC_CMD_ADDR_EXT, FALSE);
   ReadWriteSPI(SPI_DEV_ECAT_MFC, ((Address >> 8) & 0xE0) | (TMCESC_CMD_WRITE << 2), FALSE);

   ReadWriteSPI(SPI_DEV_ECAT_MFC, Value & 0xff, FALSE);
   ReadWriteSPI(SPI_DEV_ECAT_MFC, Value >> 8, TRUE);
   __enable_irq();
}

void TMC_MfcWriteDWord(UINT16 Address, UINT32 Value)
{
  __disable_irq();
   ReadWriteSPI(SPI_DEV_ECAT_MFC, Address >> 5, FALSE);
   ReadWriteSPI(SPI_DEV_ECAT_MFC, (Address << 3) | TMCESC_CMD_ADDR_EXT, FALSE);
   ReadWriteSPI(SPI_DEV_ECAT_MFC, ((Address >> 8) & 0xE0) | (TMCESC_CMD_WRITE << 2), FALSE);

   ReadWriteSPI(SPI_DEV_ECAT_MFC, Value & 0xff, FALSE);
   ReadWriteSPI(SPI_DEV_ECAT_MFC, Value >> 8, FALSE);
   ReadWriteSPI(SPI_DEV_ECAT_MFC, Value >> 16, FALSE);
   ReadWriteSPI(SPI_DEV_ECAT_MFC, Value >> 24, TRUE);
   __enable_irq();
}

void TMC_MfcWriteArray(UINT16 Address, UCHAR *Array, UINT32 Length)
{
  UINT32 i;

  __disable_irq();
  ReadWriteSPI(SPI_DEV_ECAT_MFC, Address >> 5, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, (Address << 3) | TMCESC_CMD_ADDR_EXT, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, ((Address >> 8) & 0xE0) | (TMCESC_CMD_WRITE << 2), FALSE);

   for(i=0; i<Length; i++) ReadWriteSPI(SPI_DEV_ECAT_MFC, Array[i], (i<Length-1) ? FALSE:TRUE);
   __enable_irq();
}

UCHAR TMC_MfcReadByte(UINT16 Address)
{
  UCHAR Data;

  __disable_irq();
  ReadWriteSPI(SPI_DEV_ECAT_MFC, Address >> 5, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, (Address << 3) | TMCESC_CMD_ADDR_EXT, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, ((Address >> 8) & 0xE0) | (TMCESC_CMD_READ_WAIT << 2), FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, 0xff, FALSE);

  Data=ReadWriteSPI(SPI_DEV_ECAT_MFC, 0xff, TRUE);
  __enable_irq();

  return Data;
}

USHORT TMC_MfcReadWord(UINT16 Address)
{
  UCHAR Low;
  UCHAR High;

  __disable_irq();
  ReadWriteSPI(SPI_DEV_ECAT_MFC, Address >> 5, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, (Address << 3) | TMCESC_CMD_ADDR_EXT, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, ((Address >> 8) & 0xE0) | (TMCESC_CMD_READ_WAIT << 2), FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, 0xff, FALSE);

  Low=ReadWriteSPI(SPI_DEV_ECAT_MFC, 0x00, FALSE);
  High=ReadWriteSPI(SPI_DEV_ECAT_MFC, 0xff, TRUE);
  __enable_irq();

  return (High<<8)|Low;
}

UINT32 TMC_MfcReadDWord(UINT16 Address)
{
  UCHAR Byte[4];

  __disable_irq();
  ReadWriteSPI(SPI_DEV_ECAT_MFC, Address >> 5, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, (Address << 3) | TMCESC_CMD_ADDR_EXT, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, ((Address >> 8) & 0xE0) | (TMCESC_CMD_READ_WAIT << 2), FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, 0xff, FALSE);

  Byte[0]=ReadWriteSPI(SPI_DEV_ECAT_MFC, 0x00, FALSE);
  Byte[1]=ReadWriteSPI(SPI_DEV_ECAT_MFC, 0x00, FALSE);
  Byte[2]=ReadWriteSPI(SPI_DEV_ECAT_MFC, 0x00, FALSE);
  Byte[3]=ReadWriteSPI(SPI_DEV_ECAT_MFC, 0xff, TRUE);
  __enable_irq();

  return (Byte[3] << 24) | (Byte[2] << 16) | (Byte[1] << 8) | Byte[0];
}

void TMC_MfcReadArray(UINT16 Address, UCHAR *Array, UINT32 Length)
{
  UINT32 i;

  __disable_irq();
  ReadWriteSPI(SPI_DEV_ECAT_MFC, Address >> 5, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, (Address << 3) | TMCESC_CMD_ADDR_EXT, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, ((Address >> 8) & 0xE0) | (TMCESC_CMD_READ_WAIT << 2), FALSE);
  ReadWriteSPI(SPI_DEV_ECAT_MFC, 0xff, FALSE);

  for(i=0; i<Length; i++) Array[i]=ReadWriteSPI(SPI_DEV_ECAT_MFC, (i<Length-1) ? 0x00 : 0xff, (i<Length-1) ? FALSE:TRUE);
  __enable_irq();
}


void TMC_EscWriteByte(UINT16 Address, UCHAR Value)
{
  __disable_irq();
   ReadWriteSPI(SPI_DEV_ECAT, Address >> 5, FALSE);
   ReadWriteSPI(SPI_DEV_ECAT, (Address << 3) | TMCESC_CMD_ADDR_EXT, FALSE);
   ReadWriteSPI(SPI_DEV_ECAT, ((Address >> 8) & 0xE0) | (TMCESC_CMD_WRITE << 2), FALSE);

   ReadWriteSPI(SPI_DEV_ECAT, Value, TRUE);
   __enable_irq();
}

void TMC_EscWriteWord(UINT16 Address, USHORT Value)
{
  __disable_irq();
   ReadWriteSPI(SPI_DEV_ECAT, Address >> 5, FALSE);
   ReadWriteSPI(SPI_DEV_ECAT, (Address << 3) | TMCESC_CMD_ADDR_EXT, FALSE);
   ReadWriteSPI(SPI_DEV_ECAT, ((Address >> 8) & 0xE0) | (TMCESC_CMD_WRITE << 2), FALSE);

   ReadWriteSPI(SPI_DEV_ECAT, Value & 0xff, FALSE);
   ReadWriteSPI(SPI_DEV_ECAT, Value >> 8, TRUE);
   __enable_irq();
}

void TMC_EscWriteDWord(UINT16 Address, UINT32 Value)
{
  __disable_irq();
   ReadWriteSPI(SPI_DEV_ECAT, Address >> 5, FALSE);
   ReadWriteSPI(SPI_DEV_ECAT, (Address << 3) | TMCESC_CMD_ADDR_EXT, FALSE);
   ReadWriteSPI(SPI_DEV_ECAT, ((Address >> 8) & 0xE0) | (TMCESC_CMD_WRITE << 2), FALSE);

   ReadWriteSPI(SPI_DEV_ECAT, Value & 0xff, FALSE);
   ReadWriteSPI(SPI_DEV_ECAT, Value >> 8, FALSE);
   ReadWriteSPI(SPI_DEV_ECAT, Value >> 16, FALSE);
   ReadWriteSPI(SPI_DEV_ECAT, Value >> 24, TRUE);
   __enable_irq();
}

UCHAR TMC_EscReadByte(UINT16 Address)
{
  UCHAR Data;

  __disable_irq();
  ReadWriteSPI(SPI_DEV_ECAT, Address >> 5, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT, (Address << 3) | TMCESC_CMD_ADDR_EXT, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT, ((Address >> 8) & 0xE0) | (TMCESC_CMD_READ_WAIT << 2), FALSE);
  ReadWriteSPI(SPI_DEV_ECAT, 0xff, FALSE);

  Data=ReadWriteSPI(SPI_DEV_ECAT, 0xff, TRUE);
  __enable_irq();

  return Data;
}

USHORT TMC_EscReadWord(UINT16 Address)
{
  UCHAR High;
  UCHAR Low;

  __disable_irq();
  ReadWriteSPI(SPI_DEV_ECAT, Address >> 5, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT, (Address << 3) | TMCESC_CMD_ADDR_EXT, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT, ((Address >> 8) & 0xE0) | (TMCESC_CMD_READ_WAIT << 2), FALSE);
  ReadWriteSPI(SPI_DEV_ECAT, 0xff, FALSE);

  Low=ReadWriteSPI(SPI_DEV_ECAT, 0x00, FALSE);
  High=ReadWriteSPI(SPI_DEV_ECAT, 0xff, TRUE);
  __enable_irq();

  return (High<<8)|Low;
}

UINT32 TMC_EscReadDWord(UINT16 Address)
{
  UINT8 Byte[4];

  __disable_irq();
  ReadWriteSPI(SPI_DEV_ECAT, Address >> 5, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT, (Address << 3) | TMCESC_CMD_ADDR_EXT, FALSE);
  ReadWriteSPI(SPI_DEV_ECAT, ((Address >> 8) & 0xE0) | (TMCESC_CMD_READ_WAIT << 2), FALSE);
  ReadWriteSPI(SPI_DEV_ECAT, 0xff, FALSE);

  Byte[0]=ReadWriteSPI(SPI_DEV_ECAT, 0x00, FALSE);
  Byte[1]=ReadWriteSPI(SPI_DEV_ECAT, 0x00, FALSE);
  Byte[2]=ReadWriteSPI(SPI_DEV_ECAT, 0x00, FALSE);
  Byte[3]=ReadWriteSPI(SPI_DEV_ECAT, 0xff, TRUE);
  __enable_irq();

  return (Byte[3] << 24) | (Byte[2] << 16) | (Byte[1] << 8) | Byte[0];
}
