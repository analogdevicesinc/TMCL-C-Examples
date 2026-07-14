# TMC6460 DAISY-CHAINING ROBOTIC-HAND - C Examples

## Overview
This library is intended to be operated with [TMC6460-Eval V1.x](https://www.analog.com/media/en/technical-documentation/data-sheets/tmc6460.pdf) *(hereinafter referred to as Eval Board)* with Arduino boards. In the examples and the connection circuit mentioned, the representation is in context to Arduino Due boards, which houses a 32-bit ATMEL ATSAM3X8E. However, the connections can be modified in order to operate the Eval Board with other versions of Arduino boards respectively. The motor being used for this application is Maxom 6mm BLDC Motors B843BBAEC7DA.

## Usage
### Hardware
To use the eval boards make the following connections between Arduino Due and the Eval Boards:

 | Arduino Due Pins    |   Eval Board 1 Pins    |   Eval Board 2 Pins    | 
 | --------------------|----------------------- |----------------------- |
 | +5V                 |   05 +5V_USB           |     05 +5V_USB         |
 | +5V                 |   19 SLEEPN            |     19 SLEEPN          |
 | GND                 |   GND                  |      GND               |
 | +5V                 |   08 DRV_EN            |     08 DRV_EN          |
 | 03 SPI-SCK          |   31 SPI1_SCK          |     31 SPI1_SCK        |
 | 25 SPI-CS           |   30 SPI1_CSN          |     30 SPI1_CSN        |
 | 1 SPI-MISO          |   -                    |     33 SPI1_SDO        |
 | 4 SPI-MOSI          |   32 SPI1_SDI          |      -                 |
 | -                   |   33 SPI1_SDO          |     32 SPI1_SDI        |

 > SPI connections for Arduino mentioned here (SPI-SCK, SPI-MISO & SPI-MOSI) are the SPI communication port, as Arduino Due has a separate set of pins for SPI communication. SPI-CS pin, however, belongs to the general digital pins port. 


 ### Installing Libraries
 Adding the *TMC6460-DAISY-CHAINING* library to your Arduino Libraries is quite a simple process, just follow these steps:
 1. Download the library from GitHub.
 2. Copy the TMC6460-DAISY-CHAINING complete directory to *C:\Users\Your_User_Name\Documents\Arduino\libraries\TMC6460-DAISY-CHAINING*
 3. Go to *TMC6460-DAISY-CHAINING/TMC6460-DAISY-CHAINING-ROBOTIC-HAND* and open [SPI_DaisyChain_RoboticHand.ino](\TMC6460-DAISY-CHAINING-ROBOTIC-HAND\SPI_DaisyChain_RoboticHand.ino).
 4. Opening the arduino sketch might promt to create a directory, do so if this happens.
 5. Now, the code can be modified with respect to the application.

 ## Understanding the Code
 This code implements SPI daisy chaining with TMC6460 for a Robotic Hand application; this section is a simple attempt to explain the basic aspects of the code.

 1. The Arduino code, [SPI_DaisyChain_RoboticHand.ino](\TMC6460-DAISY-CHAINING-ROBOTIC-HAND\SPI_DaisyChain_RoboticHand.ino) is capable of controlling multiple nodes using SPI communication. Initiallize the desired number of nodes / TMC6460 chips involved by changing the value of `SPI_48BITFORMAT_NO_OF_NODES` in [SPI_48BitFormat_DaisyChain.h](\TMC6460-DAISY-CHAINING-ROBOTIC-HAND\SPI_48BitFormat_DaisyChain.h)
 2. The code, based on how many TMC6460's are involved in the process, creates a datagram, to attempt to read or write to all the chips in one go. If only one of the involved chips has to be written to or read, the rest receive a dummy datagram, while the requried one receives the desired datagram.
 3.  There are five functions, `TMC6460_motorInit`, `TMC6460_TorqueMode_operation`, `TMC6460_DaisyChain_TorqueMode`, `TMC6460_DaisyChain_PositionMode` and `TMC6460_DaisyChain_Individual_Motor_Control` in the Arduino code which are responsible for controlling TMC6460's behavior, out of which three are used to drive the motor in torque mode, one for position mode, and one for controlling individual chips involved in daisy chain. Two additional functions have also been implemented, `CalculateHomePosition` and `Homing_Function`, which are responsible for bringing the motor back to its home position. 
 4. `TMC6460_motorInit` function is responsible for initialization of TMC6460s for beginning operations. This includes, motor setup, writing P & I values for Torque, flux, Velocity and Position, and enabling the feedback mechanism. These values were obtained after tuning individual motors using TMCL-IDE.
 5. `TMC6460_TorqueMode_operation` and `TMC6460_DaisyChain_TorqueMode` are responsible for setting up torque mode operations which includes applying the desired flux and torque target, switching motion mode to torque, and then running the motors in open loop for a few seconds in one direction.
 6. `TMC6460_DaisyChain_PositionMode` is responsible for operating the motors in position mode. For the robotic hand application, it is useful for pre-defining certain hand positions. 
 7. `TMC6460_DaisyChain_Individual_Motor_Control` function can be used when individual TMC6460 chips have to addressed or read back.



 > Pin numbers should be changed in the code if the connection has been altered or if any other Arduino boards are being used.

## Further Reading

For more information on the usage of TMC6460, refer the *TMC6460 guide* on [GitHub](https://github.com/analogdevicesinc/TMC-API/blob/master/tmc/ic/TMC6460/README.md).