# TMC2209 TMC-API - C Examples

## Overview
This library is intended to be operated with [TMC2209-Eval V1.x](https://www.analog.com/media/en/technical-documentation/data-sheets/tmc2209_datasheet_rev1.09.pdf) *(hereinafter referred to as Eval Board)* and older boards with Arduino boards. In the examples and the connection circuit mentioned, the representation is in context to Arduino Mega boards, however, the connections can be modified in order to operate the Eval Board with other versions of Arduino boards respectively.

## Usage
### Hardware
To use the eval boards make the following connections between Arduino Mega and the Eval Boards:

 | Arduino Pins    |   Eval Board Pins    | 
 | ----------------|--------------------- |
 | +5V             |   42 +5V_VM          |   
 | 18 UART_TX1     |   21 UART_TX (DI010) |       
 | 19 UART_RX1     |   22 UART_RX (DIO11) |     
 | 10 PIN_8        |   08 ENN (DIO0)      |  
 | 11 PIN_11       |   17 INT_STEP (DIO6) |      
 | 12 PIN_12       |   18 INT_DIR (DIO7)  |     
 | GND             |   23 CLK             |    
 | GND             |   GND                | 

 ### Installing Libraries
 Adding the *TMC2209_TMC_API* library to your Arduino Libraries is quite a simple process, just follow these steps:
 1. Download the library from GitHub.
 2. Copy the TMC2209_TMC_API complete directory to *C:\Users\Your_User_Name\Documents\Arduino\libraries\TMC2209_TMC_API*
 3. Go to *TMC2209_TMC_API/examples/* and open [TMC2209-TMC-API.ino](\examples\TMC2209-TMC-API.ino).
 4. Opening the arduino sketch might promt to create a directory, do so if this happens.
 5. Now, the code can be modified with respect to the application.

 ## Understanding the Code
 The entire library is very complex and inter-integrated as a whole; this section is a simple attempt to explain the basic aspects of the code.

 1. The Arduino code, [TMC2209-TMC-API.ino](\examples\TMC2209-TMC-API.ino) is capable of controlling multiple nodes (upto 4) using UART communication. Initiallize the respective `MOTORx_ID` with `initAllMotors` function. Make sure to configure control pins too for respective motors. 
 2. The STEP control for driving the motor is provided via `TIMER1` ISR.
 3. The `initAllMotors` function configures default settings for the Eval Board, in order to perform simple rotation. This includes the current setting for the motors. Make sure to change these settings depending on the motor being used. The settings are defined for [QSH-4218-35-10-027](https://www.analog.com/media/en/technical-documentation/data-sheets/QSH4218_datasheet_rev1.10.pdf).
 
 > Pin numbers should be changed in the code if the connection has been altered or if any other Arduino boards are being used.

## Further Reading

For more information on the usage of TMC2209, refer the *TMC2209 guide* on [GitHub](https://github.com/analogdevicesinc/TMC-API/blob/master/tmc/ic/TMC2209/README.md).