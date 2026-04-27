/*******************************************************************************
* Copyright © 2025 Analog Devices, Inc.
*******************************************************************************/

/*******************************************************************************
  Example of burning the OTP with TMC5221.
  It is highly recommended to prototype the desired OTP settings before actual burning.
  Prototyping can be done with the script "mtp_prototype.py" in this folder.
  
  The IC has the ability to burn 80 records in total.
  Exceeding this limit will not damage the device, but the burn procedure will be aborted and no changes will be made to the OTP.

  A record with the same RECORD_TYPE (and REGISTER_ADDRESS, depending on the Type) will overwrite a previous one,
  giving the ability to easily correct mistakes while burning.
  Only when BLOCK_MTP_ACCESS is set to "True", the OTP will be locked and no further changes are possible anymore, not even overwriting existing records.

  OTP burning needs a stable Supply Voltage of 8.7V ± 0.13V (1.5%).
  Do not program while the motor is in operation.
*
* Arduino MEGA to TMC5221 Eval Board Wiring:
*   Arduino MEGA Pin | TMC5221 Pin | Signal
*   -----------------|-------------|------------------
*   51 (MOSI)        | 32          | SPI1_SDI
*   50 (MISO)        | 33          | SPI1_SDO
*   52 (SCK)         | 31          | SPI1_SCK
*   53 (SS)          | 30          | SPI1_CSN
*   14 (TX3)         | 21          | UART_RX
*   15 (RX3)         | 22          | UART_TX
*   23 (D23)         | 19          | NSLEEP
*   GND              | 01, 02      | GND
*   GND              | 23 (CLK)    | GND
*   5V               | 05, 42      | +5V, +5V_VM
*
*******************************************************************************/

#include <SPI.h>

extern "C" {
#include "TMC5221_HW_Abstraction.h"
#include "TMC5221.h"
}

struct {
    uint8_t veri_fail;
    uint8_t mtp_full;
    uint8_t vpp_init_fail;
    uint8_t ov_during_burn_pulse;
    uint8_t ecc_err_1_bit;
    uint8_t ecc_err_2_bit;
} otp_status = {0};


#define IC_ID              0 
#define WRITEABLE          0
#define WRITE_PROTECT      1
#define DISABLE_IREF_FAULT 0
#define LOCK_OTP 1

#define INTERNAL_CLOCK    0
#define VOLTAGE_IN_RANGE  3
#define VOLTAGE_TOO_SMALL 0
#define VOLTAGE_TOO_HIGH  7

// Table 23 "MTP Record Types" in the datasheet 
#define RECORD_TYPE_0  0x0  // REGISTER_WRITE_LOCK [7], REGISTER_ADDRESS [6:0]
#define RECORD_TYPE_1  0x1  // REGISTER_DATA [31:0]
#define RECORD_TYPE_2  0x2  // REGISTER_LOCK_BITS [31:0]
#define RECORD_TYPE_3  0x3  // BLOCK_MTP_ACCESS [31], DISABLE_IREF_FAULT [3], I2C_NODE_ADDRESS [2:0]

// Set to "True" to burn the value of the registers in OTP 
// For prototyping/testing it's recommended to only burn registers that are not needed for motor operation.
// E.g. the USER_DATA registers.
#define LIMIT_VALUES  0
#define VMAX_LIMIT    0
#define USER_DATA_0   0
#define USER_DATA     0 

// Uncomment the below line to use UART for communication
static TMC5221BusType activeBus = IC_BUS_SPI;
//static TMC5221BusType activeBus = IC_BUS_UART;

int NSLEEP = 23;
int DRV_EN = 7;
int nCS = 22;

TMC5221BusType tmc5221_getBusType(uint16_t icID) {
  return activeBus;
}

void tmc5221_readWriteSPI(uint16_t icID, uint8_t *data, size_t dataLength) {
  digitalWrite(nCS, LOW);
  delayMicroseconds(5);

  for (uint32_t i = 0; i < dataLength; i++) {
    data[i] = SPI.transfer(data[i]);
  }

  delayMicroseconds(5);
  digitalWrite(nCS, HIGH);
}

bool tmc5221_readWriteUART(uint16_t icID, uint8_t *data, size_t writeLength, size_t readLength)
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

/*OTP Burning*/
bool check_voltage(void){
  for(uint8_t i = 0; i < 3; i++){
    uint32_t comparator_value = tmc5221_readRegister(IC_ID, TMC5221_CP_STATUS) & 0x7;
    if(comparator_value == VOLTAGE_IN_RANGE)
      return true;
    if(comparator_value == VOLTAGE_TOO_SMALL)
      Serial.println("voltage for burning is too low");
    if(comparator_value == VOLTAGE_TOO_HIGH)
      Serial.println("voltage for burning is too high");
    delay(2);
    Serial.print("Trial No. ");
    Serial.println(i+1);
  }
  return false;
}

void set_data_to_burn(uint8_t record_type, uint32_t data){
  // Writing 8 bits
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_DATA_0, data);
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_DATA_1, data >> 8);
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_DATA_2, data >> 16);
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_DATA_3, data >> 24);

  tmc5221_writeRegister(IC_ID, TMC5221_MTP_ADDR, (uint32_t)record_type);
}

void burn_data(){
  
  uint32_t mtp_status = tmc5221_readRegister(IC_ID, TMC5221_MTP_STATUS);
  uint8_t mtp_full = (mtp_status & 0x4) >> 2;

  if(!mtp_full){
    tmc5221_writeRegister(IC_ID, TMC5221_MTP_CONTROL, 0x1);
    uint8_t done_burning = false;

    while(!done_burning) {
      // Poll the done bit or error flags
      mtp_status = tmc5221_readRegister(IC_ID, TMC5221_MTP_STATUS);
      otp_status.veri_fail = (mtp_status & 0x02) >> 1;
      otp_status.mtp_full = (mtp_status & 0x04) >> 2;
      otp_status.vpp_init_fail = (mtp_status & 0x08) >> 3;
      otp_status.ov_during_burn_pulse = (mtp_status & 0x10) >> 4;
      otp_status.ecc_err_1_bit = (mtp_status & 0x20) >> 5;
      otp_status.ecc_err_2_bit = (mtp_status & 0x40) >> 6;
      done_burning = (mtp_status & 0x80) >> 7;
      Serial.println("Burning..");
      delay(10);
    }
    
    if(otp_status.veri_fail)
      Serial.println("VERI_FAIL: burning the otp 3 times did not produce a valid result");
    if(otp_status.vpp_init_fail)
      Serial.println("VPP_INIT_FAIL: programming voltage did not reach desired value. Burn abandoned");
    if(otp_status.ov_during_burn_pulse)
      Serial.println("OV_DURING_BURN_PULSE: programming voltage exceeded limits. Safe operation not guaranteed!!");
    if(otp_status.ecc_err_1_bit)
      Serial.println("ECC_ERR_1BIT: loaded record contains an ECC error that was corrected");
    if(otp_status.ecc_err_2_bit)
      Serial.println("ECC_ERR_2BIT: loaded record contains an ECC error could not be corrected");
  } else {
    Serial.println("All records are already burnt. Burn procedure will not be started and aborted.");
  }
}

void restore_otp(){
  // After burning this will load the otp values without the need of a powercycle
  tmc5221_writeRegister(IC_ID, TMC5221_MTP_CONTROL, 0x80);
}

void setup() {

  Serial.begin(9600);

  pinMode(NSLEEP, OUTPUT);
  pinMode(DRV_EN, OUTPUT);

  if (activeBus == IC_BUS_SPI) {
    pinMode(nCS, OUTPUT);
    digitalWrite(nCS, HIGH);
    SPI.begin();
    SPI.beginTransaction(SPISettings(3000000, MSBFIRST, SPI_MODE3)); // 3 MHz
  }

  else if (activeBus == IC_BUS_UART) {
    Serial3.begin(115200);
  }

  // Toggle the TMC5221 NSLEEP pin to reset it
  digitalWrite(NSLEEP, LOW);
  delayMicroseconds(10);
  digitalWrite(NSLEEP, HIGH);

  digitalWrite(DRV_EN, HIGH);

  // Add delay for the communication to get stable. Removing this delay creates problem in reading data.
  delay(1000);
}

void loop() {

  // Read oscillator configuration to ensure internal clock is used
  uint8_t oscillator = tmc5221_fieldRead(IC_ID, TMC5221_EXT_CLK_FIELD);
  
  // Disable the driver before entering OTP mode
  tmc5221_fieldWrite(IC_ID, TMC5221_DRV_EN_SW_FIELD, 0x00);

  // Enter OTP mode
  tmc5221_writeRegister(IC_ID, TMC5221_OTP_MODE_OTP_MODE, 0x12A7);

  // Enable the window comparator required for voltage checks
  tmc5221_writeRegister(IC_ID, TMC5221_CP_CONTROL_2, 0x1B);
  
  // Verify the burn voltage is in the allowed range
  bool voltage_in_range = check_voltage();

  if (voltage_in_range && (oscillator == INTERNAL_CLOCK)) {
    Serial.println("Voltage ok");   

    if (LIMIT_VALUES) {
      // Burn write protection for LIMIT_VALUES register
      uint8_t start_address = 0x45;    // 0x45 -> LIMIT_VALUES
      uint8_t write_protect = 1;
      uint32_t data = (write_protect << 7) + start_address;
      set_data_to_burn(RECORD_TYPE_0, data);
      burn_data();

      // Burn the actual LIMIT_VALUES data
      set_data_to_burn(RECORD_TYPE_1, 0x0A1F0E64);
      burn_data();
      Serial.println("Limit burned");
    }

    if (VMAX_LIMIT) {
      // Burn write protection for VMAX_LIMIT register
      uint8_t start_address = 0x44;    // 0x44 -> VMAX_LIMIT
      uint8_t write_protect = 1;
      uint32_t data = (write_protect << 7) + start_address;
      set_data_to_burn(RECORD_TYPE_0, data);
      burn_data();

      // Burn the actual VMAX_LIMIT data
      set_data_to_burn(RECORD_TYPE_1, 0x000186A0);
      burn_data();
      Serial.println("VMAX_LIMIT burned");
    }

    if (USER_DATA_0) {
      // Burn USER_DATA_0 with write protection
      uint8_t start_address = 0x46;    // 0x46 -> USER_DATA_0
      uint8_t write_protect = 1;
      uint32_t data = (write_protect << 7) + start_address;
      set_data_to_burn(RECORD_TYPE_0, data);
      burn_data();

      // Burn user data payload
      set_data_to_burn(RECORD_TYPE_1, 0xF1F1F1F1);
      burn_data();
      Serial.println("USER_DATA_0 burned");
    }

    if (USER_DATA) {
      // Burn a sequence of USER_DATA records, useful for serial numbers or application-specific data
      uint8_t write_protect = 1;
      uint8_t start_address = 0x46;
      uint32_t data = (write_protect << 7) + start_address;
      set_data_to_burn(RECORD_TYPE_0, data);
      burn_data();

      set_data_to_burn(RECORD_TYPE_1, 0x5A5A5A5A);
      burn_data();
      set_data_to_burn(RECORD_TYPE_1, 0xA5A5A5A5);
      burn_data();
      set_data_to_burn(RECORD_TYPE_1, 0xBEEFBEEF);
      burn_data();
      set_data_to_burn(RECORD_TYPE_1, 0xCAFEBABE);
      burn_data();
      set_data_to_burn(RECORD_TYPE_1, 0xBABEBEEF);
      burn_data();
      set_data_to_burn(RECORD_TYPE_1, 0xDEADBEEF);
      burn_data();
      set_data_to_burn(RECORD_TYPE_1, 0x11111111);
      burn_data();
      set_data_to_burn(RECORD_TYPE_1, 0xFFFFFFFF);
      burn_data();
      Serial.println("User-Data burned");
    }

    // Optional: block further OTP access after burning
    bool BLOCK_MTP = false;
    
    if (BLOCK_MTP) {
      uint8_t i2c_node_address = 0b000;   // Avoid 110 or 111
      uint8_t disable_iref_fault = 0x0;
      uint8_t block_mtp_access = 0x1;
      uint32_t data = (block_mtp_access << 31) + (disable_iref_fault << 3) + i2c_node_address;
      set_data_to_burn(RECORD_TYPE_3, data);
      burn_data();
      Serial.println("OTP access blocked");
    }

    Serial.println("Program ended.");
  } else {
    Serial.println("program ended without burning as voltage is not within range.");
  }

  // Load OTP values without a power cycle
  restore_otp();

  // Leave OTP mode and restore normal operation
  tmc5221_writeRegister(IC_ID, TMC5221_CP_CONTROL_2, 0x0);
  tmc5221_writeRegister(IC_ID, TMC5221_OTP_MODE_OTP_MODE, 0x12A0);

  // Stop further execution
  while (1) {}

}
