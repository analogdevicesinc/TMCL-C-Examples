/*******************************************************************************
* Copyright © 2025 Analog Devices, Inc.
*******************************************************************************/

/*******************************************************************************
* Example of burning the OTP with TMC5222.
* It is highly recommended to prototype the desired OTP settings before actual burning.
* Prototyping can be done with the script "mtp_prototype.py" in this folder.
*  
* The IC has the ability to burn 80 records in total.
* Exceeding this limit will not damage the device, but the burn procedure will be aborted and no changes will be made to the OTP.
*
* A record with the same RECORD_TYPE (and REGISTER_ADDRESS, depending on the Type) will overwrite a previous one,
* giving the ability to easily correct mistakes while burning.
* Only when BLOCK_MTP_ACCESS is set to "True", the OTP will be locked and no further changes are possible anymore, not even overwriting existing records.
*
* OTP burning needs a stable Supply Voltage of 8.7V ± 0.13V (1.5%).
* Do not program while the motor is in operation.
*
* Arduino MEGA to TMC5222 Eval Board Wiring:
*   Arduino MEGA Pin | TMC5222 Pin | Signal
*   -----------------|-------------|------------------
*   20 (SDA)         | 13          | I2C_SDA
*   21 (SCL )        | 12          | I2C_SCL
*   06 (D6)          | 19          | NSLEEP
*   07 (D7)          | 08          | DRV_EN
*   GND              | 02, 03      | GND
*   GND              | 23 (CLK)    | GND
*   5V               | 05, 42      | +5V, +5V_VM
*
*******************************************************************************/

#include <Wire.h>

extern "C" {
#include "TMC5222_HW_Abstraction.h"
#include "TMC5222.h"
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

static TMC5222BusType activeBus = IC_BUS_IIC;
static uint8_t deviceAddress = 0x60; // 0b110000(W/R)

int NSLEEP = 6;
int DRV_EN = 7;

TMC5222BusType tmc5222_getBusType(uint16_t icID) {
  return activeBus;
}

uint8_t tmc5222_getDeviceAddress(uint16_t icID)
{
  return deviceAddress;
}

bool tmc5222_readWriteIIC(uint16_t icID, uint8_t *data, size_t writeLength, size_t readLength) {

  // ---------- WRITE ----------
  Wire.beginTransmission(deviceAddress);

  if (writeLength > 0) {
    // Write starting at data[1]
    Wire.write(&data[1], writeLength);
  }

  // If we are going to read, keep the bus active
  uint8_t status = Wire.endTransmission(readLength > 0 ? false : true);
  if (status != 0) {
    Serial.println("I2C write error");
    return false;
  }

  // ---------- READ ----------
  if (readLength > 0) {
    size_t received = Wire.requestFrom(deviceAddress, readLength);

    if (received != readLength) {
      Serial.println("I2C read length mismatch");
      return false;
    }

    for (size_t i = 0; i < readLength; i++) {
      if (Wire.available()) {
        data[2 + i] = Wire.read();
      }
    }
  }

  return true;
}

/*OTP Burning*/
bool check_voltage(void){
  for(uint8_t i = 0; i < 3; i++){
    uint32_t comparator_value = tmc5222_readRegister(IC_ID, TMC5222_CP_STATUS) & 0x7;
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
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_DATA_0, data);
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_DATA_1, data >> 8);
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_DATA_2, data >> 16);
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_DATA_3, data >> 24);

  tmc5222_writeRegister(IC_ID, TMC5222_MTP_ADDR, (uint32_t)record_type);
}

void burn_data(){
  
  uint32_t mtp_status = tmc5222_readRegister(IC_ID, TMC5222_MTP_STATUS);
  uint8_t mtp_full = (mtp_status & 0x4) >> 2;

  if(!mtp_full){
    tmc5222_writeRegister(IC_ID, TMC5222_MTP_CONTROL, 0x1);
    uint8_t done_burning = false;

    while(!done_burning) {
      // Poll the done bit or error flags
      mtp_status = tmc5222_readRegister(IC_ID, TMC5222_MTP_STATUS);
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
  tmc5222_writeRegister(IC_ID, TMC5222_MTP_CONTROL, 0x80);
}

void setup() {

  Serial.begin(9600);

  pinMode(NSLEEP, OUTPUT);
  pinMode(DRV_EN, OUTPUT);

  // Join I2C bus as master
  Wire.begin(); 

  // Toggle the TMC5222 NSLEEP pin to reset it
  digitalWrite(NSLEEP, LOW);
  delayMicroseconds(10);
  digitalWrite(NSLEEP, HIGH);

  digitalWrite(DRV_EN, HIGH);

  delay(1000);
}

void loop() {

  // Read oscillator configuration to ensure internal clock is used
  uint8_t oscillator = tmc5222_fieldRead(IC_ID, TMC5222_EXT_CLK_FIELD);
  
  // Disable the driver before entering OTP mode
  tmc5222_fieldWrite(IC_ID, TMC5222_DRV_EN_SW_FIELD, 0x00);

  // Enter OTP mode
  tmc5222_writeRegister(IC_ID, TMC5222_OTP_MODE_OTP_MODE, 0x12A7);

  // Enable the window comparator required for voltage checks
  tmc5222_writeRegister(IC_ID, TMC5222_CP_CONTROL_2, 0x1B);
  
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
  tmc5222_writeRegister(IC_ID, TMC5222_CP_CONTROL_2, 0x0);
  tmc5222_writeRegister(IC_ID, TMC5222_OTP_MODE_OTP_MODE, 0x12A0);

  // Stop further execution
  while (1) {}

}
