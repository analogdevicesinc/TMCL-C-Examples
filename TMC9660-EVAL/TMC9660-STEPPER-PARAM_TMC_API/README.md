# TMC9660-STEPPER-EVAL TMC-API - C Examples

## Overview
This library is intended to be operated with [TMC9660-STEPPER-Eval](https://www.analog.com/media/en/technical-documentation/data-sheets/tmc9660.pdf) *(hereinafter referred to as Eval Board)* and older boards with Arduino boards. In the examples and the connection circuit mentioned, the representation is in context to Arduino Mega boards, however, the connections can be modified in order to operate the Eval Board with other versions of Arduino boards respectively.

## Usage
### Hardware
To use the eval boards make the following connections between Arduino Mega and the Eval Boards:

| Arduino Pins   	| Evalboard Pins 	|
|----------------	|----------------	|
| +5V            	| 5 +5V_USB      	|
| 51 MOSI        	| 32 SPI1_SDI    	|
| 50 MISO        	| 33 SPI1_SDO    	|
| 52 SCK         	| 31 SPI1_SCK    	|
| 53 SS          	| 30 SPI1_CSN    	|
| 14 USART3_TX   	| 22 UART_RX     	|
| 15 USART3_RX   	| 21 UART_TX     	|
| 41 HOLDN_FLASH 	| 34 DIO12       	|
| 49 RESET_CTRL  	| 19 DIO8        	|
| GND            	| 2 GND          	|

 ### Installing Libraries
 Adding the *TMC9660-STEPPER-PARAM_TMC_API* library to your Arduino Libraries is quite a simple process, just follow these steps:
 1. Download the library from GitHub.
 2. Copy the TMC9660-STEPPER-PARAM_TMC_API complete directory to *C:\Users\Your_User_Name\Documents\Arduino\libraries\TMC9660-STEPPER-PARAM_TMC_API*
 3. Go to *TMC9660-STEPPER-PARAM_TMC_API/examples/TMC9660-STEPPER-PARAM-TMC-API/* and open [TMC9660-STEPPER-PARAM-TMC-API.ino]().
4. Now, the code can be modified with respect to the application.
5. This will work with python scripts located [here](https://github.com/adi-innersource/pytrinamic-internal/tree/feature_add_tmc9660/examples/evalboards/TMC9660/arduino_wrapper).
6. Set the activeBus to the type of communication interface you are intending to use i.e TMC9660_BUS_SPI or TMC9660_BUS_UART.

 > Pin numbers should be changed in the code if the connection has been altered or if any other Arduino boards are being used.

## Further Reading

For more information on the usage of TMC9660-STEPPER-Eval in param mode, refer the *guide* [here](https://www.analog.com/media/en/technical-documentation/data-sheets/tmc9660-parameter-mode-reference-manual.pdf).