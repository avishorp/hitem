
#include <ctype.h>
#include <math.h>

#include "accel.h"
#include "settings.h"
#include "console.h"

// Peripheral lib
#include "hw_types.h"
#include "hw_memmap.h"
#include "rom_map.h"
#include "spi.h"
#include "prcm.h"
#include "utils.h"

#define CALIBRATION (4.0 / 0x7fff)
void AccelWriteRegister(_u8 addr, _u8 value)
{
	unsigned char txb[2];
	unsigned char rxb[2];

	// First byte: Address + WRITE
	txb[0] = addr & 0x3f;
	// Seconf byte: data
	txb[1] = value;

	// Execute SPI command
    MAP_SPITransfer(GSPI_BASE,txb,rxb,2,SPI_CS_ENABLE|SPI_CS_DISABLE);
}

_u8 AccelReadRegister(_u8 addr)
{
		unsigned char txb[2];
		unsigned char rxb[2];

		// First byte: Address + READ
		txb[0] = (addr & 0x3f) | 0x80;
		// Seconf byte: ignored
		txb[1] = 0xff;

		// Execute SPI command
	    MAP_SPITransfer(GSPI_BASE,txb,rxb,2,SPI_CS_ENABLE|SPI_CS_DISABLE);

	    // The result is in the second RX byte
	    return rxb[1];
}

int m = 0;
void AccelInit()
{
	// Reset the SPI Peripheral
    MAP_SPIReset(GSPI_BASE);

    //MAP_SPIFIFODisable(GSPI_BASE, SPI_TX_FIFO);
    //MAP_SPIFIFOEnable(GSPI_BASE, SPI_RX_FIFO);


	// Configure the SPI
	SPIConfigSetExpClk(
			GSPI_BASE,
			MAP_PRCMPeripheralClockGet(PRCM_GSPI),   // The clock supplied to the SPI Module
			100000,     // Desired bitrate
			SPI_MODE_MASTER,  // Master mode
			SPI_SUB_MODE_0, // Sub-mode 0 (Polarity 0, Phase 0)
			SPI_WL_8 |         // Config: 8 bit
			SPI_CS_ACTIVELOW | // Config: CS Active Low
			SPI_SW_CTRL_CS |   // Config: HW controlled CS
			SPI_4PIN_MODE |    // Config: 4 pin mode
			SPI_TURBO_OFF      // Config: Turbo Mode Off
	);

    // Enable the SPI module
	SPIEnable(GSPI_BASE);

	ConsolePrint("Reading SPI\n\r");

	_u8 info1 = AccelReadRegister(0xd);
	_u8 info2 = AccelReadRegister(0xe);
	_u8 whoami = AccelReadRegister(0xf);

	ConsolePrintf("Identification: %02x %02x %02x\n\r", info1, info2, whoami);

	AccelWriteRegister(0x20, 0x6f); // CTRL4 - ODR=100Hz, BDU=1, XYZen = 1
	AccelWriteRegister(0x24, 0xc0); // CTRL5 - BW=50Hz, FSCALE = 4g
	AccelWriteRegister(0x25, 0x00); // CTR


	// Temperature
	int j;
	for(j=0; j < 1000; j++ ) {
		_u8 status2 = 0;
		while ((AccelReadRegister(0x27) & 0x08) == 0);

		_u8 temp = AccelReadRegister(0x0c);
		double accel_x = (((_i16)AccelReadRegister(0x28) + (AccelReadRegister(0x29) << 8)))*CALIBRATION;
		double accel_y = (((_i16)AccelReadRegister(0x2a) + (AccelReadRegister(0x2b) << 8)))*CALIBRATION;
		double accel_z = (((_i16)AccelReadRegister(0x2c) + (AccelReadRegister(0x2d) << 8)))*CALIBRATION;
		double accel = sqrt(accel_x*accel_x + accel_y*accel_y + accel_z*accel_z);

		if (accel < 3)
			ConsolePrintf("Temperature: %d accel: %f status: %02x\n\r", temp, accel, status2);
		//UtilsDelay(500000);
	}
/*
	unsigned char txb[4];
	unsigned char rxb[4];

	txb[0] = 0x1e;
	txb[1] = 0x55;
    MAP_SPITransfer(GSPI_BASE,txb,rxb,2,SPI_CS_ENABLE|SPI_CS_DISABLE);

	int addr;
	for (addr = 0; addr < 0x1f; addr++) {
		txb[0] = 0x80 | addr;
		txb[1] = 0x00;

	    MAP_SPITransfer(GSPI_BASE,txb,rxb,2,SPI_CS_ENABLE|SPI_CS_DISABLE);
	    ConsolePrintf("  | ADDR=%03x  DATA=%02x\n\r", addr, rxb[1]);
	}
*/
/*
	while(1) {
	txb[0] = 0x8c;
	txb[1] = 0;
	MAP_SPITransfer(GSPI_BASE,txb,rxb,2,SPI_CS_ENABLE|SPI_CS_DISABLE);
	ConsolePrintf("Temp: %d \n\r", rxb[1]);
	int j;
	for(j=0; j < 10000000; j++) m++;
	}

	ConsolePrintf("Read data: %d %d %d\n\r", rxb[0], rxb[1], rxb[2]);
*/
	/*
	//while(1)
	SPIDataPut(GSPI_BASE, 0x0f);
	SPIDataPut(GSPI_BASE, 0x00);
	SPIDataPut(GSPI_BASE, 0x00);
	unsigned long dd1 = 0, dd2 = 0;
	SPIDataGet(GSPI_BASE, &dd1);
	SPIDataGet(GSPI_BASE, &dd2);
	ConsolePrintf("Read data: %d %d\n\r", dd1, dd2);
*/
}
