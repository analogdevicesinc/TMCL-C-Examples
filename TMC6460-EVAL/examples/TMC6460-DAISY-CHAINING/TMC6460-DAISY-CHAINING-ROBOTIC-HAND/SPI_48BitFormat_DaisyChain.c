/*******************************************************************************
 * Copyright © 2025 Analog Devices Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *******************************************************************************/
//20251209TE
#include "SPI_48BitFormat_DaisyChain.h"

void SPI_48BitFormat_readWriteChain(SPIChainTransfer_48BitFormat_t *transfer) {
 int8_t 
  i_nodes,
  i_bytes,
  no_bytes=0;

 for (i_nodes = SPI_48BITFORMAT_NO_OF_NODES-1; i_nodes>=0; i_nodes--) { 
  transfer->in.datagram[i_nodes].status_address = readWriteSPI((transfer->out.datagram[i_nodes].write_nRead_address>>8) & 0xFF,0);
  transfer->in.datagram[i_nodes].status_address <<=8;
  transfer->in.datagram[i_nodes].status_address |= readWriteSPI((transfer->out.datagram[i_nodes].write_nRead_address>>0) & 0xFF,0);

  no_bytes+=2;
  
  for (i_bytes = 4-1; i_bytes>=0; i_bytes--) { 
   transfer->in.datagram[i_nodes].value.bytes[i_bytes] = readWriteSPI(transfer->out.datagram[i_nodes].value.bytes[i_bytes], (++no_bytes == SPI_48BITFORMAT_NO_OF_VALUE_BYTES));
  }
 } 
}

void SPI_48BitFormat_setReadAddressForAll(SPIChainTransfer_48BitFormat_t *transfer, uint16_t address) {
	for (uint32_t i = 0; i < SPI_48BITFORMAT_NO_OF_NODES; i++) {
		transfer->out.datagram[i].write_nRead_address	= address & TMC6460_ADDRESS_0_MASK; // preset address
		transfer->out.datagram[i].value.uint32				= 0; // preset value to '0' to make the scope reading easier :)
	}
}

void SPI_48BitFormat_setWriteAddressAndValueForAll(SPIChainTransfer_48BitFormat_t *transfer, uint16_t address, uint32_t value) {
	for (uint32_t i = 0; i < SPI_48BITFORMAT_NO_OF_NODES; i++) {
		transfer->out.datagram[i].write_nRead_address	= address | SPI_48BITFORMAT_WRITE_BIT; // preset address
		transfer->out.datagram[i].value.uint32  			= value; // preset value
	}
}

void SPI_48BitFormat_writeAll(SPIChainTransfer_48BitFormat_t * transfer, uint16_t address, uint32_t value) {
	SPI_48BitFormat_setWriteAddressAndValueForAll(transfer, address, value); // set up write address and value for all
	SPI_48BitFormat_readWriteChain(transfer);
}

void SPI_48BitFormat_readAll(SPIChainTransfer_48BitFormat_t *transfer, uint16_t address) {
	SPI_48BitFormat_setReadAddressForAll(transfer, address); // set up the read address for all
	SPI_48BitFormat_readWriteChain(transfer); // first read request to set up a value
	SPI_48BitFormat_readWriteChain(transfer); // second read request to retrieve the reply
}

void SPI_48BitFormat_writeNode(SPIChainTransfer_48BitFormat_t *transfer, uint8_t id, uint16_t address, int32_t value) {
	if (id > SPI_48BITFORMAT_NO_OF_NODES) return; // do nothing if <id> exceeds the number of expected nodes in the chain

	volatile uint8_t node = id - 1;
	SPI_48BitFormat_setReadAddressForAll(transfer, SPI_48BITFORMAT_DUMMY_READ_ADDRESS); // set up dummy read addresses for all
	transfer->out.datagram[node].write_nRead_address  = address | SPI_48BITFORMAT_WRITE_BIT; // set target address
	transfer->out.datagram[node].value.int32  				= value; // set target value for targeted node only
	SPI_48BitFormat_readWriteChain(transfer);
	
}


int32_t SPI_48BitFormat_readNode(SPIChainTransfer_48BitFormat_t *transfer, uint8_t id, uint16_t address) {

	
	if (id >= SPI_48BITFORMAT_NO_OF_NODES) return 0; // do nothing if <id> exceeds the number of expected nodes in the chain
	SPI_48BitFormat_setReadAddressForAll(transfer, SPI_48BITFORMAT_DUMMY_READ_ADDRESS); // set up dummy read addresses for all
	transfer->out.datagram[id].write_nRead_address     = address & TMC6460_ADDRESS_0_MASK; // preset address
	SPI_48BitFormat_readWriteChain(transfer);  // first read request to set up a value
	SPI_48BitFormat_readWriteChain(transfer);  // second read request to retrieve the reply
	return transfer->in.datagram[id].value.int32; // return received value for convenience :)
}
