# TMC9660-STEPPER-EVAL

## How to integrate: overview

1. Include all the files of the TMC-API/ic/tmc/TMC9660-STEPPER-PARAM folder into the custom project.
2. Include the TMC9660.h, CONFIG_Map.h and TMC9660_STEPPER_PARAM_HW_Abstraction.h files in the custom source code.
3. Implement the necessary callback functions (see below).

### How to integrate: Callback functions
Implement the following callback functions:
1. Set activeBus to TMC9660_BUS_UART. The callback function **'tmc9660_getBusType()'** needs to be implemented.
2. Implement **'tmc9660_setBusAddresses()'** and **'tmc9660_getBusAddresses()'** to set and get the device and host addresses.
3. Implement **'tmc_getMicrosecondTimestamp()'** to return timestamp in microseconds.
4. Implement **tmc9660_readWriteUART()**, which is a HAL wrapper function that provides the necessary hardware access to UART lines.


