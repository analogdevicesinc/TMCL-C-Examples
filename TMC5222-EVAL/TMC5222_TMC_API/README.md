# TMC5222 TMC-API - C Examples

## Overview
This library is intended to be operated with [TMC5222-Eval V1.x](https://www.analog.com/media/en/technical-documentation/data-sheets/tmc5222_datasheet_rev1.09.pdf) *(hereinafter referred to as Eval Board)* and older boards with Arduino boards. In the examples and the connection circuit mentioned, the representation is in context to Arduino Mega boards, however, the connections can be modified in order to operate the Eval Board with other versions of Arduino boards.

## Usage
### Hardware
To use the eval boards make the following connections between Arduino Mega and the Eval Boards:

Arduino MEGA Pin | TMC5222 Pin | Signal
-----------------|-------------|------------------
20 (SDA)         | 13          | I2C_SDA
21 (SCL )        | 12          | I2C_SCL
06 (D6)          | 19          | NSLEEP
07 (D7)          | 08          | DRV_EN
GND              | 02, 03      | GND
GND              | 23 (CLK)    | GND
5V               | 05, 42      | +5V, +5V_VM

 ### Installing Libraries
 Adding the *TMC5222_TMC_API* library to your Arduino Libraries is quite a simple process, just follow these steps:
 1. Download the library from GitHub.
 2. Copy the TMC5222_TMC_API complete directory to *C:\Users\Your_User_Name\Documents\Arduino\libraries\TMC5222_TMC_API*
 3. There are two examples in TMC5222_TMC_API folder:
- Go to *TMC5222_TMC_API/examples/TMC5222_OTP_PROTOTYPE* and open [TMC5222_OTP_PROTOTYPE.ino](\examples\TMC5222_OTP_PROTOTYPE\TMC5222_OTP_PROTOTYPE.ino).
- Go to *TMC5222_TMC_API/examples/TMC5222_OTP_BURN* and open [TMC5222_OTP_BURN.ino](\examples\TMC5222_OTP_BURN\TMC5222_OTP_BURN.ino).
 4. Opening the arduino sketch might promt to create a directory, do so if this happens.
 5. Now, the code can be modified with respect to the application.

 ## Understanding the Code
 1. The Arduino code, [TMC5222_OTP_PROTOTYPE.ino](\examples\TMC5222_OTP_PROTOTYPE.ino) demonstrate the prototyping of the OTP (One-Time Programmable) feature of the TMC5222. Prototyping is reversible with a power cycle, allowing for experimentation with OTP settings without permanently altering the OTP.
 2. The Arduino code, [TMC5222_OTP_BURN.ino](\examples\TMC5222_OTP_BURN.ino) burns the data permanently into the OTP.
 3. `TMC5222BusType activeBus` - This is set to `IC_BUS_IIC` based on the communication protocol intended to be used for this Eval Board.
3. `deviceAddress` - This is set to `0x60`, which is the default address of the Eval Board. The `Wire.h` library is used for I2C communication. The function `Wire.beginTransmission(deviceAddress)` expects the device address without the R/W bit.
 
 > Pin numbers should be changed in the code if the connection has been altered or if any other Arduino boards are being used.

## Further Reading

For more information on the usage of TMC5222, refer the *TMC5222 guide* on [GitHub](https://github.com/analogdevicesinc/TMC-API/blob/master/tmc/ic/TMC5222/README.md).