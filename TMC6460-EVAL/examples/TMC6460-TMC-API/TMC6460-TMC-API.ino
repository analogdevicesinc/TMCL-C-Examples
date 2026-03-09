/*******************************************************************************
* Copyright © 2026 Analog Devices Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

/*******************************************************************************
* TMC6460 Motor Control Example
* 
* This example demonstrates open-loop and closed-loop control of a BLDC motor
* using the TMC6460 integrated motor control IC with field-oriented control (FOC).
*
* Motor Configuration:
*   Model:              QBL4208-61 BLDC Motor
*   Pole Pairs:         4
*   Encoder Type:       ABN (3-channel incremental)
*   Encoder Resolution: 1024 P/R
*
* Encoder to TMC6460 Wiring:
*   Encoder Signal | TMC6460 Pin
*   --------------|-------------
*   A (White)     | A+
*   B (Green)     | B+
*   N (Yellow)    | Z+
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
* Performance Notes:
*   - Torque and flux PI coefficients are tuned for the QBL4208-61 motor
*   - Coefficients must be retuned for alternative motor types
*   - SPI communication @ 7.5 MHz, UART @ 115200 baud (if enabled)
*
*******************************************************************************/

#include <SPI.h>

extern "C" {
    #include "TMC6460_HW_Abstraction.h"
    #include "TMC6460.h"
}

// Uncomment the below line to use UART for communication 
static TMC6460BusType activeBus = TMC6460_BUS_SPI;
//static TMC6460BusType activeBus = TMC6460_BUS_UART;

int DRV_EN = 6;
int NSLEEP = 23;

#define IC_ID 0

// Select which control mode to use
#define USE_OPEN_LOOP 1
//#define USE_OPEN_LOOP 0 // Closed-loop

/*******************************************************************************
* Auto-generated parameters from TMCL-IDE
*******************************************************************************/
#define MOTOR_TYPE   3  // BLDC
#define N_POLE_PAIRS 4
#define MAX_COUNT    4800

// ADC and CSA settings
#define USE_INTERNAL_R_REF 1
#define LS_RES_ON          3  // RES_55_MOHM
#define CSA_GAIN           0  // X1

// Limits
#define PID_UQ_UD_LIMITS   23169
#define PID_FLUX_LIMIT     10537
#define PID_TORQUE_LIMIT   10537

// Current PI settings
#define CURRENT_NORM_P 0  // CUR_P_NO_SHIFT
#define CURRENT_NORM_I 1  // CUR_I_SHIFT_BY_8
#define FLUX_P         240
#define FLUX_I         1887
#define TORQUE_P       240
#define TORQUE_I       1887

// Primary feedback: ABN
#define INV_DIR 1
#define CPR     4096

// Motion settings
#define PID_TORQUE_TARGET 2000

// Openloop settings
#define PID_FLUX_TARGET     2000
#define PID_VELOCITY_TARGET 500
/**************************************************************************/

enum TMC6460BusType tmc6460_getBusType(uint16_t icID)
{
    return activeBus;
}

void tmc6460_readWriteSPI(uint16_t icID, uint8_t *data, size_t dataLength) {
    digitalWrite(PIN_SPI_SS, LOW);
    delayMicroseconds(10);

    //Serial.println("SPI transaction");

    for (uint32_t i = 0; i < dataLength; i++) {
        //Serial.print(data[i], HEX);
        //Serial.print(" -> ");
        data[i] = SPI.transfer(data[i]);
        //Serial.println(data[i], HEX);
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

#ifdef USE_OPEN_LOOP
void tmc6460_openLoop(uint16_t icID) {
    uint32_t value = 0;
    
    /*** Openloop rotation ****************************************************/
    // Set ramper parameters for openloop velocity movement
    tmc6460_readRegister(icID, TMC6460_MCC_CONFIG_MOTOR_MOTION, &value);
    value = tmc6460_updateField(value, TMC6460_MCC_CONFIG_MOTOR_MOTION_RAMP_MODE_FIELD, 1); // RAMP_VELOCITY
    value = tmc6460_updateField(value, TMC6460_MCC_CONFIG_MOTOR_MOTION_RAMP_USE_PHI_E_FIELD, 1); 
    value = tmc6460_updateField(value, TMC6460_MCC_CONFIG_MOTOR_MOTION_RAMP_EN_FIELD, 1); 
    tmc6460_writeRegister(icID, TMC6460_MCC_CONFIG_MOTOR_MOTION, value);
    tmc6460_writeField(icID, TMC6460_RAMPER_A1_FIELD, 100); 
    tmc6460_writeField(icID, TMC6460_RAMPER_A2_FIELD, 200); 
    tmc6460_writeField(icID, TMC6460_RAMPER_A_MAX_FIELD, 100); 

    // Enable current control mode
    // Turning system on...
    tmc6460_writeField(icID, TMC6460_MCC_CONFIG_GDRV_DRV_EN_BIT_FIELD, 1); 
    tmc6460_writeField(icID, TMC6460_MCC_CONFIG_PWM_CHOP_FIELD, 7); // CENTERED
    tmc6460_writeRegister(icID, TMC6460_FOC_PID_TORQUE_FLUX_TARGET, 0); 
    tmc6460_writeField(icID, TMC6460_MCC_CONFIG_MOTOR_MOTION_MOTION_MODE_FIELD, 2); // TORQUE

    // Apply torque and start moving the ramper
    // Rotating motor...
    // We use a flux current in openloop angle operation to avoid a 90° angle offset
    tmc6460_writeField(icID, TMC6460_FOC_PID_TORQUE_FLUX_TARGET_FLUX_FIELD, PID_FLUX_TARGET);
    tmc6460_writeField(icID, TMC6460_FOC_PID_VELOCITY_TARGET_FIELD, PID_VELOCITY_TARGET);

    // Spin for a few seconds
    delay(5000);

    // Stop motor
    tmc6460_writeField(icID, TMC6460_FOC_PID_TORQUE_FLUX_TARGET_FLUX_FIELD, 0); 
    tmc6460_writeField(icID, TMC6460_FOC_PID_VELOCITY_TARGET_FIELD, 0); 
    delay(1000);

    // Turning system off...
    tmc6460_writeField(icID, TMC6460_MCC_CONFIG_MOTOR_MOTION_MOTION_MODE_FIELD, 1); // PWM_ON
}
#else
void tmc6460_closedLoop(uint16_t icID) {
    uint32_t value = 0;
    
    /*** Torque mode operation ************************************************/
    // Enable current control mode
    // Turning system on...
    tmc6460_writeField(icID, TMC6460_MCC_CONFIG_GDRV_DRV_EN_BIT_FIELD, 1); 
    tmc6460_writeField(icID, TMC6460_MCC_CONFIG_PWM_CHOP_FIELD, 7); // CENTERED
    tmc6460_writeRegister(icID, TMC6460_FOC_PID_TORQUE_FLUX_TARGET, 0); 
    tmc6460_writeField(icID, TMC6460_MCC_CONFIG_MOTOR_MOTION_MOTION_MODE_FIELD, 2); // TORQUE

    // Initializing ABN feedback using forced zero method
    // Set ramper for position mode and use it to generate a Phi E of 0
    tmc6460_writeField(icID, TMC6460_RAMPER_POSITION_FIELD, 0); 
    tmc6460_readRegister(icID, TMC6460_MCC_CONFIG_MOTOR_MOTION, &value);
    value = tmc6460_updateField(value, TMC6460_MCC_CONFIG_MOTOR_MOTION_RAMP_MODE_FIELD, 0); // RAMP_POSITION
    value = tmc6460_updateField(value, TMC6460_MCC_CONFIG_MOTOR_MOTION_RAMP_EN_FIELD, 1); 
    value = tmc6460_updateField(value, TMC6460_MCC_CONFIG_MOTOR_MOTION_RAMP_USE_PHI_E_FIELD, 1); 
    tmc6460_writeRegister(icID, TMC6460_MCC_CONFIG_MOTOR_MOTION, value);

    // Apply a target flux to pull the motor into a known position
    tmc6460_writeField(icID, TMC6460_FOC_PID_TORQUE_FLUX_TARGET_FLUX_FIELD, PID_FLUX_TARGET);
    
    // Wait for motor to settle
    delay(1000);

    // Clear the ABN count, this ensures the encoder and the motor's Phi E are in sync
    tmc6460_writeField(icID, TMC6460_ABN_COUNT_FIELD, 0); 

    // Turn off the flux input and resume closed loop operation
    tmc6460_writeField(icID, TMC6460_FOC_PID_TORQUE_FLUX_TARGET_FLUX_FIELD, 0); 
    tmc6460_writeField(icID, TMC6460_MCC_CONFIG_MOTOR_MOTION_RAMP_USE_PHI_E_FIELD, 0); 

    // Encoder initialized
    delay(1000);

    // Applying forward torque...
    tmc6460_writeField(icID, TMC6460_FOC_PID_TORQUE_FLUX_TARGET_TORQUE_FIELD, PID_TORQUE_TARGET);
    
    // Spin for a few seconds
    delay(3000);

    // Stop motor
    tmc6460_writeRegister(icID, TMC6460_FOC_PID_TORQUE_FLUX_TARGET, 0); 
    delay(1000);

    // Applying backward torque...
    tmc6460_writeField(icID, TMC6460_FOC_PID_TORQUE_FLUX_TARGET_TORQUE_FIELD, -PID_TORQUE_TARGET);
    
    // Spin for a few seconds
    delay(3000);

    // Stop motor
    tmc6460_writeRegister(icID, TMC6460_FOC_PID_TORQUE_FLUX_TARGET, 0); 
    delay(1000);

    // Turning system off...
    tmc6460_writeField(icID, TMC6460_MCC_CONFIG_MOTOR_MOTION_MOTION_MODE_FIELD, 1); // PWM_ON
    // Done!
}
#endif

/*******************************************************************************
* Auto-generated configuration function from TMCL-IDE
*******************************************************************************/
void tmc6460_autogeneratedExample(uint16_t icID) {
    uint32_t value = 0;

    /*** Configuration ********************************************************/
    // Ensure system is off before applying settings
    tmc6460_writeField(icID, TMC6460_MCC_CONFIG_MOTOR_MOTION_MOTION_MODE_FIELD, 1); // PWM_ON

    // Configuring TMC6460...

    // Motor setup
    tmc6460_readRegister(icID, TMC6460_MCC_CONFIG_MOTOR_MOTION, &value);
    value = tmc6460_updateField(value, TMC6460_MCC_CONFIG_MOTOR_MOTION_MOTOR_TYPE_FIELD, MOTOR_TYPE); // BLDC
    value = tmc6460_updateField(value, TMC6460_MCC_CONFIG_MOTOR_MOTION_N_POLE_PAIRS_FIELD, N_POLE_PAIRS); 
    tmc6460_writeRegister(icID, TMC6460_MCC_CONFIG_MOTOR_MOTION, value);

    // PWM setup
    tmc6460_writeField(icID, TMC6460_MCC_CONFIG_PWM_PERIOD_MAX_COUNT_FIELD, MAX_COUNT);
    tmc6460_writeField(icID, TMC6460_MCC_CONFIG_PWM_SV_MODE_FIELD, 1); // HARMONIC

    // ADC and CSA setup
    tmc6460_readRegister(icID, TMC6460_MCC_CONFIG_GDRV, &value);
    value = tmc6460_updateField(value, TMC6460_MCC_CONFIG_GDRV_USE_INTERNAL_R_REF_FIELD, USE_INTERNAL_R_REF); 
    value = tmc6460_updateField(value, TMC6460_MCC_CONFIG_GDRV_LS_RES_ON_FIELD, LS_RES_ON); // RES_55_MOHM
    value = tmc6460_updateField(value, TMC6460_MCC_CONFIG_GDRV_SLEW_RATE_FIELD, 2); // SR_400_V_PER_US
    tmc6460_writeRegister(icID, TMC6460_MCC_CONFIG_GDRV, value);
    tmc6460_writeField(icID, TMC6460_MCC_ADC_CSA_GAIN_CSA_GAIN_FIELD, CSA_GAIN);

    // Limits
    tmc6460_writeField(icID, TMC6460_FOC_PID_UQ_UD_LIMITS_FIELD, PID_UQ_UD_LIMITS);
    tmc6460_writeField(icID, TMC6460_FOC_PID_VELOCITY_LIMIT_FIELD, 2147483647); 
    tmc6460_readRegister(icID, TMC6460_FOC_PID_TORQUE_FLUX_LIMITS, &value);
    value = tmc6460_updateField(value, TMC6460_FOC_PID_TORQUE_FLUX_LIMITS_PID_FLUX_LIMIT_FIELD, PID_FLUX_LIMIT); 
    value = tmc6460_updateField(value, TMC6460_FOC_PID_TORQUE_FLUX_LIMITS_PID_TORQUE_LIMIT_FIELD, PID_TORQUE_LIMIT); 
    tmc6460_writeRegister(icID, TMC6460_FOC_PID_TORQUE_FLUX_LIMITS, value);

    // Torque and flux PI
    tmc6460_readRegister(icID, TMC6460_FOC_PID_CONFIG, &value);
    value = tmc6460_updateField(value, TMC6460_FOC_PID_CONFIG_CURRENT_NORM_P_FIELD, CURRENT_NORM_P); // CUR_P_NO_SHIFT
    value = tmc6460_updateField(value, TMC6460_FOC_PID_CONFIG_CURRENT_NORM_I_FIELD, CURRENT_NORM_I); // CUR_I_SHIFT_BY_8
    tmc6460_writeRegister(icID, TMC6460_FOC_PID_CONFIG, value);

    value = tmc6460_updateField(0, TMC6460_FOC_PID_FLUX_COEFF_FLUX_P_FIELD, FLUX_P); 
    value = tmc6460_updateField(value, TMC6460_FOC_PID_FLUX_COEFF_FLUX_I_FIELD, FLUX_I); 
    tmc6460_writeRegister(icID, TMC6460_FOC_PID_FLUX_COEFF, value);

    value = tmc6460_updateField(0, TMC6460_FOC_PID_TORQUE_COEFF_TORQUE_P_FIELD, TORQUE_P); 
    value = tmc6460_updateField(value, TMC6460_FOC_PID_TORQUE_COEFF_TORQUE_I_FIELD, TORQUE_I); 
    tmc6460_writeRegister(icID, TMC6460_FOC_PID_TORQUE_COEFF, value);

    // Feedback engine - ABN feedback
    tmc6460_writeField(icID, TMC6460_ABN_CONFIG_INV_DIR_FIELD, INV_DIR);
    tmc6460_readRegister(icID, TMC6460_ABN_CONFIG, &value);
    value = tmc6460_updateField(value, TMC6460_ABN_CONFIG_CPR_FIELD, CPR - 1); 
    value = tmc6460_updateField(value, TMC6460_ABN_CONFIG_CLN_FIELD, 0); 
    tmc6460_writeRegister(icID, TMC6460_ABN_CONFIG, value);

    // Feedback engine - ABN angle feedback via channel A
    value = tmc6460_updateField(0, TMC6460_FEEDBACK_CONF_CH_A_SRC_SEL_A_FIELD, 0); // ABN_1
    value = tmc6460_updateField(value, TMC6460_FEEDBACK_CONF_CH_A_CPR_INV_A_FIELD, ((1 << 24) / CPR)); 
    tmc6460_writeRegister(icID, TMC6460_FEEDBACK_CONF_CH_A, value);
    tmc6460_writeField(icID, TMC6460_FEEDBACK_PHI_E_OFFSET_FIELD, 0); 
    tmc6460_writeField(icID, TMC6460_FEEDBACK_LUT_LOOKUP_A_EN_FIELD, 0); 
    tmc6460_readRegister(icID, TMC6460_FEEDBACK_OUTPUT_CONF, &value);
    value = tmc6460_updateField(value, TMC6460_FEEDBACK_OUTPUT_CONF_PHI_E_SRC_FIELD, 0); // LOOKUP_A
    value = tmc6460_updateField(value, TMC6460_FEEDBACK_OUTPUT_CONF_PHI_E_MUL_FACTOR_FIELD, N_POLE_PAIRS); 
    tmc6460_writeRegister(icID, TMC6460_FEEDBACK_OUTPUT_CONF, value);

    // Ensure the ramper does not override the feedback engine's PHI_E
    tmc6460_writeField(icID, TMC6460_MCC_CONFIG_MOTOR_MOTION_RAMP_USE_PHI_E_FIELD, 0); 

    // Feedback engine - ABN velocity feedback via channel B
    value = tmc6460_updateField(0, TMC6460_FEEDBACK_CONF_CH_B_SRC_SEL_B_FIELD, 1); // ABN_1_FREE
    value = tmc6460_updateField(value, TMC6460_FEEDBACK_CONF_CH_B_CPR_INV_B_FIELD, 256); 
    tmc6460_writeRegister(icID, TMC6460_FEEDBACK_CONF_CH_B, value);
    tmc6460_writeField(icID, TMC6460_FEEDBACK_LUT_LOOKUP_B_EN_FIELD, 0); 

    // Configure fast velocity meter
    value = tmc6460_updateField(0, TMC6460_FEEDBACK_VELOCITY_FRQ_CONF_VELOCITY_SYNC_SRC_FIELD, 0); // PWM_Z
    value = tmc6460_updateField(value, TMC6460_FEEDBACK_VELOCITY_FRQ_CONF_VELOCITY_SAMPLING_FIELD, 0); 
    value = tmc6460_updateField(value, TMC6460_FEEDBACK_VELOCITY_FRQ_CONF_VELOCITY_SCALING_FIELD, 6991); 
    tmc6460_writeRegister(icID, TMC6460_FEEDBACK_VELOCITY_FRQ_CONF, value);

    // Configure slow velocity meter
    value = tmc6460_updateField(0, TMC6460_FEEDBACK_VELOCITY_PER_CONF_POS_DEV_MIN_FIELD, 1); 
    value = tmc6460_updateField(value, TMC6460_FEEDBACK_VELOCITY_PER_CONF_POS_DEV_TIMER_FIELD, 65520); 
    tmc6460_writeRegister(icID, TMC6460_FEEDBACK_VELOCITY_PER_CONF, value);
    tmc6460_writeField(icID, TMC6460_FEEDBACK_VELOCITY_PER_FILTER_FILTER_WIDTH_FIELD, 3); 

    // Select velocity & position source
    tmc6460_readRegister(icID, TMC6460_FEEDBACK_OUTPUT_CONF, &value);
    value = tmc6460_updateField(value, TMC6460_FEEDBACK_OUTPUT_CONF_VELOCITY_SRC_FIELD, 1); // LOOKUP_B
    value = tmc6460_updateField(value, TMC6460_FEEDBACK_OUTPUT_CONF_POSITION_SRC_FIELD, 1); // LOOKUP_B
    value = tmc6460_updateField(value, TMC6460_FEEDBACK_OUTPUT_CONF_VELOCITY_SELECTION_FIELD, 1); // VELOCITY_PER
    tmc6460_writeRegister(icID, TMC6460_FEEDBACK_OUTPUT_CONF, value);

    #ifdef USE_OPEN_LOOP
      tmc6460_openLoop(icID);
    #else
      tmc6460_closedLoop(icID);
    #endif
}
/**************************************************************************/

void setup()
{
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

    tmc6460_autogeneratedExample(IC_ID);
}

void loop() {
  uint32_t data;

  // Reading register TMC6460_CHIP_IO_MATRIX (0x06)
  tmc6460_readRegister(IC_ID, TMC6460_CHIP_IO_MATRIX, &data);
  Serial.print("Received Data: ");
  Serial.println(data, HEX);
  delay(10);
  Serial.print(" from register: ");
  Serial.println(TMC6460_CHIP_IO_MATRIX, HEX);
  delay(1000);
  
}
