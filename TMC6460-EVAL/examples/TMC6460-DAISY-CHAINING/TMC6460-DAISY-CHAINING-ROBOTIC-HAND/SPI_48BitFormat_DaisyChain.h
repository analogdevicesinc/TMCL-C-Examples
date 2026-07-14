/*******************************************************************************
 * Copyright © 2026 Analog Devices Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *******************************************************************************/
//20260106TE

/*******************************************************************************
 ** TMC6460 Daisy Chain *********************************************************
 ********************************************************************************
 * This project allows to use an abritrary number of TMC6460 (and all parts with
 * compatible datagrams) in a daisy chain.
 ********************************************************************************/

#ifndef SPI_48BitFORMAT_DAISY_CHAIN_H_
	#define SPI_48BitFORMAT_DAISY_CHAIN_H_

	#define SPI_48BITFORMAT_NO_OF_NODES 					2               // defines the number of TMC6460 in the chain. In general all 1 byte addres + 5 byte datagrams are compatible.
	#define SPI_48BITFORMAT_DUMMY_READ_ADDRESS 		0x0009  		// Dummy address, if only one specific device in the chain shall be addressed, this address is used for a dummy read access.

	#define TMC6460_READ_INDICATOR 								0
	#define TMC6460_ADDRESS_0_MASK								0x3FF
	#define SPI_48BITFORMAT_WRITE_BIT       	  	0x8000
  #define SPI_48BITFORMAT_NO_OF_VALUE_BYTES		SPI_48BITFORMAT_NO_OF_NODES  * 6



	#include <stdint.h>

	/**
	 * Structure of a reply datagram to a single IC.
	 * The Union allows to access the value per byte or as int/uint.
	 * The order of value[4],status[1] within the struct allows to iterate through the bytes for receiving (last byte first).
	 */
	typedef struct
	{
		union {
			uint8_t bytes[4];
			uint32_t uint32;
			int32_t int32;
		} value;                // received reply value in different representations
		uint16_t status_address;       // received SPI address
	} SPIReply_48BitFormat_t; //

	/**
	 * Structure of a request (read or write) datagram to a single IC.
	 * The Union allows to access the value per byte or as int/uint.
	 * The order of value[4],address[1] within the struct allows to iterate through the bytes for sending (last byte first).
	 */
	typedef struct
	{
		union {
			uint8_t bytes[4];
			uint32_t uint32;
			int32_t int32;
		} value;                // SPI request to be sent in different representations
		uint16_t write_nRead_address;
	} SPIRequest_48BitFormat_t;

	/**
	 * Structure of a full transfer(read or write) with request and reply datagrams to all ICs in the chain.
	 * The union allows to access the value per byte or as struct for the respective request/reply datagram.
	 * The whole transaction is captured within this structure to prepare requests and access replies.
	 * Different functions for read/write access can used it for data handling.
	 */
	typedef struct
	{
		union {
			SPIReply_48BitFormat_t datagram[SPI_48BITFORMAT_NO_OF_NODES];
			uint8_t bytes[6 * SPI_48BITFORMAT_NO_OF_NODES];
		} in;

		union {
			SPIRequest_48BitFormat_t datagram[SPI_48BITFORMAT_NO_OF_NODES];
			uint8_t bytes[6 * SPI_48BITFORMAT_NO_OF_NODES];
		} out;
	} SPIChainTransfer_48BitFormat_t;

	/**
	 * This helper function presets the addresses for all nodes in the chain to a read request to register <address> (no write bit).
	 * It is used to preset dummy reads, if only single nodes are relevant for an access, or to request the same register from all nodes.
	 * @param transfer - data structure of the SPI transfer datagrams
	 * @param address - target read address (MSB(write bit) will be removed)
	 */
	void SPI_48BitFormat_setReadAddressForAll(SPIChainTransfer_48BitFormat_t *transfer, uint16_t address);

	/**
	 * This helper function presets the addresses for all nodes in the chain to a write request to register <address> with <value>.
	 * It is used to prepare transactions where the same registers shall be written to the same value for all nodes in the chain.
	 * @param transfer - data structure of the SPI transfer datagrams
	 * @param address - target write address (MSB(write bit) will be set)
	 * @param value - target value
	 */
	void SPI_48BitFormat_setWriteAddressAndValueForAll(SPIChainTransfer_48BitFormat_t *transfer, uint16_t address, uint32_t value);

	/**
	 * Send out the prepared request to <transfer> and read the response back to <transfer>.
	 * @param transfer - data structure of the SPII transfer datagrams
	 */
	void SPI_48BitFormat_readWriteChain(SPIChainTransfer_48BitFormat_t *transfer);

	/**
	 * Write the same <value> to the reqister <address> for all nodes in the chain.
	 * @param transfer - data structure of the SPI transfer datagrams
	 * @param address - target write address (MSB(write bit) will be set)
	 * @param value - target value
	 */
	void SPI_48BitFormat_writeAll(SPIChainTransfer_48BitFormat_t * transfer, uint16_t address, uint32_t value);

	/**
	 * Read the same register <address> for all nodes in the chain.
	 * @param transfer - data structure of the SPI transfer datagrams
	 * @param address - target write address (MSB(write bit) will be removed)
	 */
	void SPI_48BitFormat_readAll(SPIChainTransfer_48BitFormat_t *transfer, uint16_t address);


	/**
	 * Write  <value> to the register <address> for the specific node with <id>.
	 * All other nodes receive dummy read requests.
	 * <id> is numbered by the physical order in the chain, where the IC closest to the POCI (µC in) is 0.
	 * @param transfer - data structure of the SPI transfer datagrams
	 * @param id - target node id.
	 * @param address - target write address (MSB(write bit) will be set)
	 * @param value - target value
	 */
	void SPI_48BitFormat_writeNode(SPIChainTransfer_48BitFormat_t *transfer, uint8_t id, uint16_t address, int32_t value);

	/**
	 * Read the reqister <address> of the specific node with <id>.
	 * All other nodes receive dummy read requests.
	 * <id> is numbered by the physical order in the chain, where the IC closest to the POCI (µC in) is 0.
	 * @param transfer - data structure of the SPI transfer datagrams
	 * @param id - target node id.
	 * @param address - target write address (MSB(write bit) will be removed)
	 */
	int32_t SPI_48BitFormat_readNode(SPIChainTransfer_48BitFormat_t *transfer, uint8_t id, uint16_t address);

	/**
	 * This SPI function needs to be provided to allow for SPI bus access according to the HAL.
	 * With each call the chip select is pulled low and <dataByte> is transferred, while a byte is received in response and returned by the function.
	 * @param dataBytesingle - byte to write
	 * @param isLastByte - if <> 0, the chip select pin will be released
	 * @return received byte
	 */
	 extern uint8_t readWriteSPI(uint8_t dataByte, uint8_t isLastByte);

#endif
