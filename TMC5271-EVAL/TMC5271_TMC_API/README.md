# TMC5271 TMC-API - C Examples

## Overview
This library is intended to be operated with [TMC5271-Eval V1.x](https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/tmc5271-eval.html#eb-overview) (hereinafter referred to as eval board) and older boards with Arduino boards. In the examples and the connection circuit mentioned, the representation is in context to Arduino Mega boards; however, the connections can be modified in order to operate the Eval Board with other versions of Arduino boards respectively.

## Usage
### Hardware
To use the eval boards make the following connections between Arduino Mega and the Eval Boards:

 | Arduino Pins   | Eval Board Pins          |
 | -------------- | ------------------------ | 
 | 51 MOSI        | 32 SPI1_SDI              |
 | 50 MISO        | 33 SPI1_SDO              |
 | 52 SCK         | 31 SPI1_SCK              |
 | 53 SS          | 30 SPI1_CSN              |
 | 14 USART3_TX   | 22 UART_RX               |
 | 15 USART3_RX   | 21 UART_TX               |
 | GND            | 23 CLK16 -> use internal | 
 | 23 DIO         | 19 nSleep                |
 | GND            | 2 GND                    |
 | +5V            | 5 +5V_USB                |
 | 27 iRefR2      | 35 IREF_R2               |
 | 29 iRefR3      | 36 IRREF_R3              |
 | 31 uartMode    | 20 USEL                  |

 ### Installing Libraries
 Adding the *TMC5271_TMC_API* library to your Arduino Libraries is quite a simple process, just follow these steps:
 1. Download the library from GitHub.
 2. Copy the TMC5271_TMC_API complete directory to *C:\Users\Your_User_Name\Documents\Arduino\libraries\TMC5271_TMC_API*
 3. Go to *TMC5271_TMC_API/examples/* and open [TMC5271-TMC-API.ino](\examples\TMC5271-TMC-API.ino).
 4. Opening the arduino sketch might promt to create a directory, do so if this happens.
 5. Now, the code can be modified with respect to the application.

 ## Understanding the Code
 The entire library is very complex and inter-integrated as a whole; this section is a simple attempt to explain the basic aspects of the code.

 1. `TMC5271BusType activeBus` - This can be set to `IC_BUS_SPI` or `IC_BUS_UART` based on the communication protocol intended to be used for the application.
 2. Set the baud rate for `Serial.begin(XXXX)` and `Serial3.begin(XXXX)`.
 3. `tmc5271_rotateMotor(IC_ID, 0, 0xXXXXX)` - Set appropriate velocity based on the motor being used.
 
 > Pin numbers should be changed in the code if the connection has been altered or if any other Arduino boards are being used.

## Further Reading

For more information on the usage of TMC5271, refer the *TMC5271 guide* on [GitHub](https://github.com/analogdevicesinc/TMC-API/blob/master/tmc/ic/TMC5271/README.md).