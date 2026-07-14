/*******************************************************************************
* Copyright © 2025 Analog Devices Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/
//20260701VT

/*******************************************************************************
** TMC6460 Daisy Chain Example *************************************************
********************************************************************************
* This example project was generated and tested with an Arduino Due ATSAM3X8E (which is a 32-bit microcontroller, 
* instead of the usual 8-bit microcontroller which is mostly found in Arduino Uno or Arduino Mega) in 
* combination with 2x TMC6460-Eval in an SPI daisy chain. 
* In general, this project allows to use an abritrary number of TMC6460 and
* (and all parts with compatible datagrams) in a daisy chain.
* The following code controls a finger of the robotic hand.


* Suitable Adaptations were required as switching from a 
* 8-bit microcontroller to a 32-bit microcontroller introduces paddings in between the 
* datagrams obtained from individual TMC6460.


* The code can also be used and tested on Arduino Uno or Arduino Mega instead, without requiring any changes.
*
* Arduino Uno Pins       Eval Board 1 Pins   Eval Board 0 Pins              Arduino Due Pins
* ----------------------------------------------------------------------------------------------------------------------------
* +5V                       05 +5V_USB           05 +5V_USB                 +5V OR 2 from SPI communication port
* +5V                       19 SLEEPN            19 SLEEPN                  +5V OR 2 from SPI communication port
* GND                        GND                  GND                       GND OR 6 from SPI communication port
* GND                       08 DRV_ENN           08 DRV_ENN                 GND OR 6 from SPI communication port

                                                                            Due has different port for SPI communication, with pin 1 marked as a dot 
* 52 SPI-SCK                31 SPI1_SCK          31 SPI1_SCK                3 SPI-SCK
* 25 SPI-CS                 30 SPI1_CSN          30 SPI1_CSN                25 SPI-CS

* 50 SPI-MISO               -                    33 SPI1_SDO                1 SPI-MISO
* 51 SPI-MOSI               32 SPI1_SDI          -                          4 SPI-MOSI
* -                         33 SPI1_SDO          32 SPI1_SDI                -
********************************************************************************************************************************/


#include <SPI.h>
#include <stdint.h>
extern "C" {

  #include "TMC6460_Fields.h"
  #include "TMC6460_Register.h"

  #include "SPI_48BitFormat_DaisyChain.h"
}

#define TMC6460_CSN 25   // Arduino SPI-CS 

//#define DEBUG_SPI 1  // uncomment to feed back transferred bytes via serial.

uint8_t readWriteSPI(uint8_t dataByte, uint8_t isLastByte){
  #ifdef DEBUG_SPI // optional debus feedback via serial
    if(digitalRead(TMC6460_CSN))
    {
      Serial.print("OUT: 0x");
    }
    if(dataByte < 16) Serial.print("0"); // add pending zeros for readability of single digit bytes
    Serial.print(dataByte,HEX);
  #endif 

  digitalWrite(TMC6460_CSN, LOW); // pull chp select low
 // delayMicroseconds(1); // wait until chip select is stable

  dataByte =  SPI.transfer(dataByte); // transfer byte and gather response byte
  
  if(isLastByte)  // is this the last byte for teh transfer?
  {
   // delayMicroseconds(1); // release chip select
    digitalWrite(TMC6460_CSN, HIGH); // release chip select
    #ifdef DEBUG_SPI // optional debus feedback via serial
      Serial.println("");
    #endif
  }

  return dataByte;
}

void setup() 
{
  Serial.begin(115200);  // start serial communication for potential debugging
  Serial.println("");

  pinMode(TMC6460_CSN, OUTPUT); // our chipselect
  digitalWrite(TMC6460_CSN, HIGH);  // set high => default idle state

  SPI.begin();
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE1));

  delay(1000);  

  TMC6460_motorInit();
  TMC6460_TorqueMode_operation();
TMC6460_DaisyChain_TorqueMode();


}



static void TMC6460_DaisyChain_datagrams( void )
{
  static SPIChainTransfer_48BitFormat_t transfer;
  static uint8_t v;

  volatile uint8_t in;
  SPI_48BitFormat_writeNode( &transfer, 0, TMC6460_RAMPER_V_MAX, v++);    //0x0A:
  SPI_48BitFormat_writeNode( &transfer, 1, TMC6460_RAMPER_V_MAX, v++);    //0x0A:

  in = SPI_48BitFormat_readNode( &transfer, 0, TMC6460_RAMPER_V_MAX);    //0x0A:
  Serial.println(in);
  in = SPI_48BitFormat_readNode( &transfer, 1, TMC6460_RAMPER_V_MAX);    //0x0A:
  Serial.println(in);
  in++;
}

void TMC6460_motorInit()
{
  static SPIChainTransfer_48BitFormat_t transfer;

  // Initialization of TMC6460 for beginning operation
  Serial.println("Initialization of TMC6460");
  // Motor setup
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_MCC_CONFIG_MOTOR_MOTION              , 0x00000200);        //PWM Mode ON
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_MCC_CONFIG_MOTOR_MOTION              , 0x00000381);        // BLDC Motor type and 1 pole pair
  
  // PWM setup
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_MCC_CONFIG_PWM_PERIOD             , 0xFFFF0277);           // MAX_COUNT : 631
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_MCC_CONFIG_PWM             , 0x00000017);                  // SV Mode : Harmonic

  //ADC and CSA setup
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_MCC_CONFIG_GDRV             , 0x81013402);                 // USE_INTERNAL_R_REF : False, LS_RES_ON : RES_200_MOHM, SLEW_RATE : SR_400_V_PER_US
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_MCC_ADC_CSA_GAIN             , 0x0000000F);                // CSA_GAIN : X4

  //Limits
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_FOC_PID_UQ_UD_LIMITS            , 0x00005A81);             //PID_UQ_UD_LIMITS : 23169
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_FOC_PID_VELOCITY_LIMIT            , 0x00005208);           //PID_VELOCITY_LIMIT : 21000
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_FOC_PID_TORQUE_FLUX_LIMITS            , 0x16DD16DD);       //PID_FLUX_LIMIT = PID_TORQUE_LIMIT : 299.9882892855207 mA RMS

  //Torque and flux PI
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_FOC_PID_CONFIG           , 0x00008500);                    //CURRENT_NORM_P : CUR_P_NO_SHIFT, CURRENT_NORM_I : CUR_I_NO_SHIFT
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_FOC_PID_FLUX_COEFF            , 0x00B40001);               //FLUX_P : 180, FLUX_I : 1
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_FOC_PID_TORQUE_COEFF            , 0x00B40001);             //TORQUE_P : 180, TORQUE_I : 1

  //Position PI
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_FOC_PID_CONFIG            , 0x00008500);                   //POSITION_NORM_I : I_SHIFT_BY_16, POSITION_NORM_P : P_SHIFT_BY_8
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_FOC_PID_POSITION_COEFF           , 0x01F40000);            //POSITION_P : 500, POSITION_I : 0

  //Velocity PI
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_FOC_PID_CONFIG            , 0x00008500);                   //VELOCITY_NORM_P : P_NO_SHIFT, VELOCITY_NORM_I : I_SHIFT_BY_8
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_FOC_PID_VELOCITY_COEFF            , 0x002D0003);           //VELOCITY_P : 45, VELOCITY_I : 3

  //Feedback engine - ABN feedback
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_ABN_CONFIG            , 0x0003FF00);                       //INV_DIR : False, CPR : 1023, CLN : False
  Serial.println("Reached Feedback engine - ABN feedback");
  //Feedback engine - ABN angle feedback via channel A
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_FEEDBACK_CONF_CH_A           , 0x00004000);                //SRC_SEL_A : ABN_1, CPR_INV_A : 2**24 // CPR
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_FEEDBACK_PHI_E_OFFSET           , 0x00000000);             //PHI_E_OFFSET : 0
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_FEEDBACK_LUT           , 0x00000000);                      //LOOKUP_A_EN : False
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_FEEDBACK_OUTPUT_CONF          , 0x00000001);               //PHI_E_SRC : LOOKUP_A, PHI_E_MUL_FACTOR : N_POLE_PAIRS = 1

  //Ensure the ramper does not override the feedback engine's PHI_E
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_MCC_CONFIG_MOTOR_MOTION          , 0x00000381);            //RAMP_USE_PHI_E : False

  //Feedback engine - ABN velocity feedback via channel B
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_FEEDBACK_CONF_CH_B          , 0x01000100);                 //SRC_SEL_B : ABN_1_FREE, CPR_INV_B : 256
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_FEEDBACK_LUT           , 0x00000000);                      //LOOKUP_B_EN : False

  //Configure fast velocity meter  
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_FEEDBACK_V_METER          , 0x001B4F00);                   //VELOCITY_SYNC_SRC : PWM_Z, VELOCITY_SAMPLING : 0, VELOCITY_SCALING : 6991

  //Configure slow velocity meter  
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_FEEDBACK_SV_METER          , 0xFFF00001);                  //POS_DEV_MIN : 1, POS_DEV_TIMER : 65520
  SPI_48BitFormat_writeAll( &transfer,  TMC6460_FEEDBACK_SV_METER_FILTER         , 0x00000003);            //FILTER_WIDTH : 3

  //Select velocity & position source
  SPI_48BitFormat_writeAll( &transfer, TMC6460_FEEDBACK_OUTPUT_CONF           , 0x00700001);               //VELOCITY_SRC : LOOKUP_B, POSITION_SRC : LOOKUP_B, VELOCITY_SELECTION : VELOCITY_PER
  Serial.println("End of initialization");
}

void TMC6460_TorqueMode_operation()
{
  SPIChainTransfer_48BitFormat_t transfer;
  volatile int32_t t_pos, a_pos1, a_pos2, r_pos;                                                                      



  Serial.println("Turning system on");

  //Enable current control mode
  SPI_48BitFormat_writeAll( &transfer, TMC6460_MCC_CONFIG_GDRV           , 0x81013402);                    // DRV_EN_BIT : True
  SPI_48BitFormat_writeAll( &transfer, TMC6460_MCC_CONFIG_PWM           , 0x00000017);                     // CHOP.choice : CENTERED
  SPI_48BitFormat_writeAll( &transfer, TMC6460_FOC_PID_TORQUE_FLUX_TARGET , 0x00000000);                   // PID_TORQUE_FLUX_TARGET : 0

  //To align the electrical and the encoder angle
  SPI_48BitFormat_writeAll( &transfer, TMC6460_MCC_CONFIG_MOTOR_MOTION , 0x00004581);                      //MOTION_MODE : TORQUE

  Serial.println("Initializing ABN feedback using forced zero method");

  //Set ramper for position mode and use it to generate a Phi E of 0
  SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_POSITION , 0x00000000);                              // RAMPER.POSITION.POSITION : 0
  SPI_48BitFormat_writeAll( &transfer, TMC6460_MCC_CONFIG_MOTOR_MOTION , 0x00004581);                      // RAMP_MODE.choice : RAMP_POSITION, RAMP_EN : True, RAMP_USE_PHI_E : True

  //Apply a taregt flux to pull the motor into a known position
  SPI_48BitFormat_writeAll( &transfer, TMC6460_FOC_PID_TORQUE_FLUX_TARGET , 0x000007AA);                   // PID_FLUX_TARGET : 100.5784463120369 mA RMS

  //Wait for motor to settle
  delay(1000);

  //Reading the position values for TARGET, ACTUAL and RAMPER, and setting all of them to 0
  t_pos = SPI_48BitFormat_readNode( &transfer,1, TMC6460_FOC_PID_POSITION_TARGET );
  Serial.print("Target position: ");
  Serial.println(t_pos);
  SPI_48BitFormat_writeAll( &transfer, TMC6460_FOC_PID_POSITION_ACTUAL , 0x00000000);                      //  PID_POSITION_ACTUAL : 0
  a_pos1 = SPI_48BitFormat_readNode( &transfer,0, TMC6460_FOC_PID_POSITION_ACTUAL );
  a_pos2 = SPI_48BitFormat_readNode( &transfer,1, TMC6460_FOC_PID_POSITION_ACTUAL );
  Serial.print("Actual Position for motor 1: ");
  Serial.println(a_pos1, HEX);
  Serial.print("Actual Position for motor 2: ");
  Serial.println(a_pos2, HEX);

  r_pos = SPI_48BitFormat_readNode( &transfer,1, TMC6460_RAMPER_POSITION );
  Serial.print("Ramper Position: ");
  Serial.println(r_pos);

  //Clear the ABN count, this ensures the encoder and the motor's Phi E are in sync
  SPI_48BitFormat_writeAll( &transfer, TMC6460_ABN_COUNT , 0x00000000);                                     // ABN.COUNT : 0

  // Turn off the flux input and resume closed loop operation
  SPI_48BitFormat_writeAll( &transfer, TMC6460_FOC_PID_TORQUE_FLUX_TARGET , 0x00000000);                    // PID_FLUX_TARGET : 0
  SPI_48BitFormat_writeAll( &transfer, TMC6460_MCC_CONFIG_MOTOR_MOTION , 0x00004581);                       // RAMP_USE_PHI_E : False

  //Setting CHIP.EVENTS
  SPI_48BitFormat_writeAll( &transfer, TMC6460_CHIP_EVENTS , 0xFFFFFFFF);                                   // EVENTS : FFFFFFFF

  Serial.println("Encoder Initialized");
  delay(1000);
// SPI_48BitFormat_writeAll( &transfer, TMC6460_FOC_PID_TORQUE_FLUX_TARGET , 41549824);
}

static void TMC6460_DaisyChain_TorqueMode( void )
{
  SPIChainTransfer_48BitFormat_t transfer;
  volatile int32_t actual_vel1, actual_vel2, actual_pos;                                                                     // Rotation and start time for simple rotation of motors in torque mode 
  volatile int32_t gate_fail1, gate_fail2, gate_driver1, gate_driver2;
  volatile uint32_t target_torque1, target_torque2, pid_conf;

  SPI_48BitFormat_writeAll( &transfer, TMC6460_MCC_CONFIG_MOTOR_MOTION , 0x00004581); 
  SPI_48BitFormat_writeAll( &transfer, TMC6460_MCC_CONFIG_GDRV           , 0x81013402);
  gate_fail1 =  SPI_48BitFormat_readNode( &transfer,1, TMC6460_MCC_CONFIG_MOTOR_MOTION  );
  Serial.print("motor motion 1: ");
  Serial.println(gate_fail1, HEX);

  delay(1000);
  // Read the gate status bit and gate driver bit
  gate_fail1 =  SPI_48BitFormat_readNode( &transfer,0, TMC6460_CHIP_STATUS_FLAGS  );
  gate_fail2 = SPI_48BitFormat_readNode( &transfer,1, TMC6460_CHIP_STATUS_FLAGS  );
  Serial.print("Gate status bit for motor 1: ");
  Serial.println(gate_fail1, HEX);
  Serial.print("Gate status bit for motor 2: ");
  Serial.println(gate_fail2, HEX);

  gate_driver1 = SPI_48BitFormat_readNode( &transfer,0, TMC6460_MCC_CONFIG_GDRV );
  gate_driver2 = SPI_48BitFormat_readNode( &transfer,1, TMC6460_MCC_CONFIG_GDRV );
  Serial.print("Gate driver bit for motor 1: ");
  Serial.println(gate_driver1, HEX);
  Serial.print("Gate driver bit for motor 2: ");
  Serial.println(gate_driver2, HEX);
  
  Serial.println("Applying torque");
  
  SPI_48BitFormat_writeAll( &transfer, TMC6460_FOC_PID_TORQUE_FLUX_TARGET , 0x027A0000);
  SPI_48BitFormat_writeAll( &transfer, TMC6460_FOC_PID_POSITION_ACTUAL , 0x00000000);
  SPI_48BitFormat_writeAll( &transfer, TMC6460_CHIP_EVENTS , 0xFFFFFFFF);
  actual_pos = SPI_48BitFormat_readNode( &transfer,1, TMC6460_FOC_PID_POSITION_ACTUAL );

  while (actual_pos < 200000)
  {
  gate_fail1 =  SPI_48BitFormat_readNode( &transfer,1, TMC6460_MCC_CONFIG_MOTOR_MOTION  );
  Serial.print("motor motion 1: ");
  Serial.println(gate_fail1, HEX);


  gate_fail1 =  SPI_48BitFormat_readNode( &transfer,1, TMC6460_MCC_CONFIG_GDRV   );
  Serial.print("Gate driver 1: ");
  Serial.println(gate_fail1, HEX);

    SPI_48BitFormat_writeAll( &transfer, TMC6460_FOC_PID_TORQUE_FLUX_TARGET , 0x027A0000);
    SPI_48BitFormat_writeAll( &transfer, TMC6460_CHIP_EVENTS , 0x3FFFFFFF);

  target_torque1 = SPI_48BitFormat_readNode( &transfer,1, TMC6460_FOC_PID_TORQUE_FLUX_TARGET );
    
    target_torque2 = SPI_48BitFormat_readNode( &transfer,1, TMC6460_FOC_PID_TORQUE_FLUX_TARGET );
    Serial.print("Printing the current target torque 1 : ");
  Serial.println(target_torque1, HEX);

   Serial.print("Printing the current target torque 2 : ");
  Serial.println(target_torque2, HEX);


  actual_vel1 = SPI_48BitFormat_readNode( &transfer,1, TMC6460_FOC_PID_TORQUE_FLUX_ACTUAL );
  Serial.print("Chip 1 torque status: ");
  Serial.println(actual_vel1, HEX);


  pid_conf = SPI_48BitFormat_readNode( &transfer,1, TMC6460_CHIP_EVENTS  );
  Serial.print("Chip 1 pid config/ events : ");
  Serial.println(pid_conf, HEX);

  Serial.print("Printing the current actual position : ");
  Serial.println(actual_pos);
  actual_vel1 = SPI_48BitFormat_readNode( &transfer,1, TMC6460_FOC_PID_VELOCITY_ACTUAL );
  actual_pos = SPI_48BitFormat_readNode( &transfer,1, TMC6460_FOC_PID_POSITION_ACTUAL );
  delay(200);
  gate_fail1 = SPI_48BitFormat_readNode( &transfer,0, TMC6460_CHIP_STATUS_FLAGS  );
  gate_fail2 = SPI_48BitFormat_readNode( &transfer,1, TMC6460_CHIP_STATUS_FLAGS  );
  actual_vel1 = SPI_48BitFormat_readNode( &transfer,0, TMC6460_FOC_PID_VELOCITY_ACTUAL );
  actual_vel2 = SPI_48BitFormat_readNode( &transfer,1, TMC6460_FOC_PID_VELOCITY_ACTUAL );
  Serial.print("Chip 1 velocity: ");
  Serial.println(actual_vel1);
  Serial.print("Chip 2 velocity: ");
  Serial.println(actual_vel2);
  Serial.print("Chip 1 status flag: ");
  Serial.println(gate_fail1, HEX);
  Serial.print("Chip 2 status flag: ");
  Serial.println(gate_fail2, HEX);
  }
  Serial.println("Position reached");
  delay(1000);
}

static void TMC6460_DaisyChain_PositionMode( uint32_t position )
{
// Settings of ramper for position mode
SPIChainTransfer_48BitFormat_t transfer;
volatile uint32_t target_pos, actual_pos;
volatile uint32_t actual_torque;


SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_V_START , 0x00000000);                            //RAMPER_V_START     : 0
SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_V_STOP , 0x00000060);                             //RAMPER_V_STOP      : 96
SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_V1 , 0x0000000A);                                 //RAMPER_V1          : 10
SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_V2 , 0x00000014);                                 //RAMPER_V2          : 20
SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_V_MAX , 0x00005208);                              //RAMPER_V_MAX       : 21000
SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_A1 , 0x00000064);                                 //RAMPER_A1          : 100
SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_A2 , 0x00000064);                                 //RAMPER_A2          : 100
SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_A_MAX , 0x000000FA);                              //RAMPER_A_MAX       : 250
SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_D1 , 0x00000064);                                 //RAMPER_D1          : 100
SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_D2 , 0x00000064);                                 //RAMPER_D2          : 100
SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_D_MAX , 0x000000FA);                              //RAMPER_D_MAX       : 250
SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_TIME_CONFIG , 0x00000000);                        //RAMPER_TIME_CONFIG : 0


delay(1000);


// Setting the tolerance values for the position and the necessary delay for it to stop correcting
SPI_48BitFormat_writeAll( &transfer, TMC6460_FOC_PID_POSITION_TOLERANCE , 0x00000001);                //PID_POSITION_TOLERANCE : 1
SPI_48BitFormat_writeAll( &transfer, TMC6460_FOC_PID_POSITION_TOLERANCE_DELAY , 0x0000251C);          //PID_POSITION_TOLERANCE_DELAY : 950

Serial.println("Switching to position mode");


//Setting motion mode to position, enabling ramper module for generating ramping movement profiles, enable velocity feedforward only when motion mode = position
SPI_48BitFormat_writeAll( &transfer, TMC6460_MCC_CONFIG_MOTOR_MOTION , 0x00084981);                   //MOTOR_MOTION  : POSITION

delay(200);

// Set the defined position for the robotic finger action
SPI_48BitFormat_writeAll( &transfer, TMC6460_FOC_PID_POSITION_TARGET , position);

target_pos = SPI_48BitFormat_readNode( &transfer,0, TMC6460_FOC_PID_POSITION_TARGET );
Serial.print("Position (target): ");
Serial.println(target_pos, HEX);

actual_pos = SPI_48BitFormat_readNode( &transfer,0, TMC6460_FOC_PID_POSITION_ACTUAL );
Serial.print("Position (actual): ");
Serial.println(actual_pos);

actual_torque = SPI_48BitFormat_readNode( &transfer,0, TMC6460_FOC_PID_POSITION_ACTUAL );
Serial.print("Torque (actual): ");
Serial.println(actual_torque);



while ((actual_pos < (target_pos - 10)) || (actual_pos > (target_pos + 10)))
{
    Serial.print("Current position (actual): ");
    Serial.println(actual_pos, HEX);
    delay(500);
}

}






static void TMC6460_DaisyChain_Individual_Motor_Control (uint32_t icID1, uint32_t position1, uint32_t icID2, uint32_t position2 )
{
SPIChainTransfer_48BitFormat_t transfer;
volatile uint32_t target_pos1, target_pos2, actual_pos1, actual_pos2;
volatile uint32_t actual_torque1, actual_torque2;




SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_V_START , 0x00000000);                            //RAMPER_V_START     : 0
SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_V_STOP , 0x00000060);                             //RAMPER_V_STOP      : 96
SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_V1 , 0x0000000A);                                 //RAMPER_V1          : 10
SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_V2 , 0x00000014);                                 //RAMPER_V2          : 20
SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_V_MAX , 0x00005208);                              //RAMPER_V_MAX       : 21000
SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_A1 , 0x00000064);                                 //RAMPER_A1          : 100
SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_A2 , 0x00000064);                                 //RAMPER_A2          : 100
SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_A_MAX , 0x000000FA);                              //RAMPER_A_MAX       : 250
SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_D1 , 0x00000064);                                 //RAMPER_D1          : 100
SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_D2 , 0x00000064);                                 //RAMPER_D2          : 100
SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_D_MAX , 0x000000FA);                              //RAMPER_D_MAX       : 250
SPI_48BitFormat_writeAll( &transfer, TMC6460_RAMPER_TIME_CONFIG , 0x00000000);                        //RAMPER_TIME_CONFIG : 0

delay(1000);


// Setting the tolerance values for the position and the necessary delay for it to stop correcting
SPI_48BitFormat_writeAll( &transfer, TMC6460_FOC_PID_POSITION_TOLERANCE , 0x00000001);                //PID_POSITION_TOLERANCE : 1
SPI_48BitFormat_writeAll( &transfer, TMC6460_FOC_PID_POSITION_TOLERANCE_DELAY , 0x0000251C);          //PID_POSITION_TOLERANCE_DELAY : 950

Serial.println("Switching to position mode");

//Setting motion mode to position, enabling ramper module for generating ramping movement profiles, enable velocity feedforward only when motion mode = position
SPI_48BitFormat_writeAll( &transfer, TMC6460_MCC_CONFIG_MOTOR_MOTION , 0x00084981);                   //MOTOR_MOTION  : POSITION

delay(200);

// Set the defined position for the robotic finger action
SPI_48BitFormat_writeNode( &transfer, icID2, TMC6460_FOC_PID_POSITION_TARGET , position2);
SPI_48BitFormat_writeNode( &transfer, icID1, TMC6460_FOC_PID_POSITION_TARGET , position1);


Serial.println(position2);
//Reading the values of Motor 1
target_pos1 = SPI_48BitFormat_readNode( &transfer, icID1, TMC6460_FOC_PID_POSITION_TARGET );
Serial.print("Position (target) of motor 1: ");
Serial.println(target_pos1, HEX);

actual_pos1 = SPI_48BitFormat_readNode( &transfer, icID1, TMC6460_FOC_PID_POSITION_ACTUAL );
Serial.print("Position (actual) of motor 1: ");
Serial.println(actual_pos1);

actual_torque1 = SPI_48BitFormat_readNode( &transfer, icID1, TMC6460_FOC_PID_POSITION_ACTUAL );
Serial.print("Torque (actual) of motor 1: ");
Serial.println(actual_torque1);



//Reading the values of Motor 2
target_pos2 = SPI_48BitFormat_readNode( &transfer, icID2, TMC6460_FOC_PID_POSITION_TARGET );
Serial.print("Position (target) of motor 2: ");
Serial.println(target_pos2, HEX);

actual_pos2 = SPI_48BitFormat_readNode( &transfer, icID2, TMC6460_FOC_PID_POSITION_ACTUAL );
Serial.print("Position (actual) of motor 2: ");
Serial.println(actual_pos2);

actual_torque2 = SPI_48BitFormat_readNode( &transfer, icID2, TMC6460_FOC_PID_POSITION_ACTUAL );
Serial.print("Torque (actual) of motor 2: ");
Serial.println(actual_torque2);



// stay in loop till actual position of both motors have reached their target positions
while ((actual_pos1 < (target_pos1 - 10)) || (actual_pos1 > (target_pos1 + 10))   && (actual_pos2 < (target_pos2 - 10)) || (actual_pos2 > (target_pos2 + 10)))
{
    Serial.print("Current position (actual) of motor 1: ");
    Serial.println(actual_pos1, HEX);

    Serial.print("Current position (actual) of motor 2: ");
    Serial.println(actual_pos2, HEX);
    delay(500);
}


}



#define HALF_CYCLE_POS 221250L

static long CalculateHomePosition(long current_pos)
{
    if (current_pos == 0)
        return 0;

    long turns = current_pos / HALF_CYCLE_POS;

    return ((turns % 2) == 0)
           ? (turns * HALF_CYCLE_POS)
           : ((turns + 1) * HALF_CYCLE_POS);
}

static void Homing_function(void)
{
    SPIChainTransfer_48BitFormat_t transfer;

    long current_pos1 =
        SPI_48BitFormat_readNode(&transfer, 0,
                                 TMC6460_FOC_PID_POSITION_ACTUAL);

    long current_pos2 =
        SPI_48BitFormat_readNode(&transfer, 1,
                                 TMC6460_FOC_PID_POSITION_ACTUAL);

    Serial.print("Motor 1 Position: ");
    Serial.println(current_pos1);

    Serial.print("Motor 2 Position: ");
    Serial.println(current_pos2);

    if ((current_pos1 == 0) && (current_pos2 == 0))
    {
        Serial.println("Both motors already at home position");
        return;
    }

    long new_pos1 = CalculateHomePosition(current_pos1);
    long new_pos2 = CalculateHomePosition(current_pos2);

    Serial.print("Motor 1 Home Target: ");
    Serial.println(new_pos1);

    Serial.print("Motor 2 Home Target: ");
    Serial.println(new_pos2);

    TMC6460_DaisyChain_Individual_Motor_Control(
        1, new_pos1,
        2, new_pos2
    );
}




void loop() 
{    

}