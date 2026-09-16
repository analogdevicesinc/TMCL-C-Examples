# TMC5221 TMC-API - C Examples

## Overview
This library is intended to be operated with [TMC5221-Eval V1.x](https://www.analog.com/media/en/technical-documentation/data-sheets/tmc5221_datasheet_rev1.09.pdf) *(hereinafter referred to as Eval Board)* and older boards with Arduino boards. In the examples and the connection circuit mentioned, the representation is in context to Arduino Mega boards, however, the connections can be modified in order to operate the Eval Board with other versions of Arduino boards.

## Usage
### Hardware
To use the eval boards make the following connections between Arduino Mega and the Eval Boards:

Arduino MEGA Pin | TMC5221 Pin | Signal
-----------------|-------------|------------------
51 (MOSI)        | 32          | SPI1_SDI
50 (MISO)        | 33          | SPI1_SDO
52 (SCK)         | 31          | SPI1_SCK
53 (SS)          | 30          | SPI1_CSN
23 (D23)         | 19          | NSLEEP
GND              | 02, 03      | GND
GND              | 23 (CLK)    | GND
5V               | 05, 42      | +5V, +5V_VM

 ### Installing Libraries
 Adding the *TMC5221_TMC_API* library to your Arduino Libraries is quite a simple process, just follow these steps:
 1. Download the library from GitHub.
 2. Copy the TMC5221_TMC_API complete directory to *C:\Users\Your_User_Name\Documents\Arduino\libraries\TMC5221_TMC_API*
 3. There are two examples in TMC5221_TMC_API folder:
- Go to *TMC5221_TMC_API/examples/TMC5221_OTP_PROTOTYPE* and open [TMC5221_OTP_PROTOTYPE.ino](\examples\TMC5221_OTP_PROTOTYPE\TMC5221_OTP_PROTOTYPE.ino).
- Go to *TMC5221_TMC_API/examples/TMC5221_OTP_BURN* and open [TMC5221_OTP_BURN.ino](\examples\TMC5221_OTP_BURN\TMC5221_OTP_BURN.ino).
 4. Opening the arduino sketch might promt to create a directory, do so if this happens.
 5. Now, the code can be modified with respect to the application.

 ## Understanding the Code
 1. The Arduino code, [TMC5221_OTP_PROTOTYPE.ino](\examples\TMC5221_OTP_PROTOTYPE.ino) demonstrate the prototyping of the OTP (One-Time Programmable) feature of the TMC5221. Prototyping is reversible with a power cycle, allowing for experimentation with OTP settings without permanently altering the OTP.
 2. The Arduino code, [TMC5221_OTP_BURN.ino](\examples\TMC5221_OTP_BURN.ino) burns the data permanently into the OTP.
 3. `TMC5221BusType activeBus` - This is set to `IC_BUS_SPI` based on the communication protocol intended to be used for this Eval Board.
 
 > Pin numbers should be changed in the code if the connection has been altered or if any other Arduino boards are being used.

## Further Reading

For more information on the usage of TMC5221, refer the *TMC5221 guide* on [GitHub](https://github.com/analogdevicesinc/TMC-API/blob/master/tmc/ic/TMC5221/README.md).