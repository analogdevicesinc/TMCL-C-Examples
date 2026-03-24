# TMC6460: HEX file upload to the SRAM

## Overview
This library is intended to be operated with [TMC6460-Eval V1.x](https://www.analog.com/media/en/technical-documentation/data-sheets/tmc6460.pdf) *(hereinafter referred to as Eval Board)* and older boards with Arduino boards. In the examples and the connection circuit mentioned, the representation is in context to Arduino Mega boards, however, the connections can be modified in order to operate the Eval Board with other versions of Arduino boards.

## Usage
### Hardware
To use the eval boards make the following connections between Arduino Mega and the Eval Boards:

Arduino MEGA Pin | TMC6460 Pin | Signal
-----------------|-------------|------------------
51 (MOSI)        | 32          | SPI1_SDI
50 (MISO)        | 33          | SPI1_SDO
52 (SCK)         | 31          | SPI1_SCK
53 (SS)          | 30          | SPI1_CSN
14 (TX3)         | 21          | UART_RX
15 (RX3)         | 22          | UART_TX
06 (D6)          | 08          | DRV_EN
23 (D23)         | 19          | NSLEEP
GND              | 02          | GND
5V               | 05, 42      | +5V, +5V_VM

 ### Installing Libraries
 Adding the *TMC6460_TMC_API* library to your Arduino Libraries is quite a simple process, just follow these steps:
 1. Download the library from GitHub.
 2. Copy the TMC6460_TMC_API complete directory to *C:\Users\Your_User_Name\Documents\Arduino\libraries\TMC6460_TMC_API*
 3. Go to *TMC6460_TMC_API/examples/* and open [TMC6460_HEX_UPLOAD.ino](\examples\TMC6460_HEX_UPLOAD.ino).
 4. Opening the arduino sketch might promt to create a directory, do so if this happens.
 5. Now, the code can be modified with respect to the application.

 ## Understanding the Code
 The default communication protocol used is `SPI` which can be changed using the `activeBus` variable in [TMC6460_HEX_UPLOAD.ino](\examples\TMC6460_HEX_UPLOAD.ino).
 
 ## Converting the .hex file to .h file
 .hex file is not directly usable in the Arduino sketch, by converting the HEX file into a C header (.h) containing a byte array, the firmware data becomes part of the compiled program and is stored in the microcontroller’s flash memory.
 Following steps needs to be followed to generate a header file:
 1. Convert the hex file to a binary file. Write command `objcopy -I ihex -O binary firmware.hex firmware.bin` in the command prompt. Tool required: objcopy(part of GNU binutils / AVR‑GCC / ARM‑GCC).
 2. Convert binary into the c array. Write command `xxd -i firmware.bin > firmware.h`. This generates a firmware.h file in the project folder.
 3. The generated firmware.h file has an array containing the firmware bytes and a length variable indicating the array size.
 4. Put **PROGMEM** from <avr/pgmspace.h> library after the array name in the header file. PROGMEM is required to store large, constant firmware data in flash memory instead of RAM, preventing memory exhaustion on resource‑constrained Arduino boards.
 5. Place the generated .h file in the Arduino project folder and include it. The firmware bytes can now be accessed directly and used for programming. 

## Further Reading

For more information on the usage of TMC6460, refer the *TMC6460 guide* on [GitHub](https://github.com/analogdevicesinc/TMC-API/blob/master/tmc/ic/TMC6460/README.md).