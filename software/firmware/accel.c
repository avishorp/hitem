
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
#include "gpio.h"



#define SPI_CS_SET(v)     GPIOPinWrite(GPIOA2_BASE, 0x2, v? 0x2 : 0x00)
#define SPI_SCL_SET(v)    MAP_GPIOPinWrite(GPIOA1_BASE, 0x40, v? 0x40 : 0)
#define SPI_MOSI_SET(v)   MAP_GPIOPinWrite(GPIOA2_BASE, 0x1, (v)? 0x01 : 0)
#define SPI_MISO_GET      GPIOPinRead(GPIOA1_BASE, 0x80)
#define SPI_DELAY         300

#define LIS3DH_REG_STATUS1       0x07
#define LIS3DH_REG_OUTADC1_L     0x08
#define LIS3DH_REG_OUTADC1_H     0x09
#define LIS3DH_REG_OUTADC2_L     0x0A
#define LIS3DH_REG_OUTADC2_H     0x0B
#define LIS3DH_REG_OUTADC3_L     0x0C
#define LIS3DH_REG_OUTADC3_H     0x0D
#define LIS3DH_REG_INTCOUNT      0x0E
#define LIS3DH_REG_WHOAMI        0x0F
#define LIS3DH_REG_TEMPCFG       0x1F
#define LIS3DH_REG_CTRL1         0x20
#define LIS3DH_REG_CTRL2         0x21
#define LIS3DH_REG_CTRL3         0x22
#define LIS3DH_REG_CTRL4         0x23
#define LIS3DH_REG_CTRL5         0x24
#define LIS3DH_REG_CTRL6         0x25
#define LIS3DH_REG_REFERENCE     0x26
#define LIS3DH_REG_STATUS2       0x27
#define LIS3DH_REG_OUT_X_L       0x28
#define LIS3DH_REG_OUT_X_H       0x29
#define LIS3DH_REG_OUT_Y_L       0x2A
#define LIS3DH_REG_OUT_Y_H       0x2B
#define LIS3DH_REG_OUT_Z_L       0x2C
#define LIS3DH_REG_OUT_Z_H       0x2D
#define LIS3DH_REG_FIFOCTRL      0x2E
#define LIS3DH_REG_FIFOSRC       0x2F
#define LIS3DH_REG_INT1CFG       0x30
#define LIS3DH_REG_INT1SRC       0x31
#define LIS3DH_REG_INT1THS       0x32
#define LIS3DH_REG_INT1DUR       0x33
#define LIS3DH_REG_CLICKCFG      0x38
#define LIS3DH_REG_CLICKSRC      0x39
#define LIS3DH_REG_CLICKTHS      0x3A
#define LIS3DH_REG_TIMELIMIT     0x3B
#define LIS3DH_REG_TIMELATENCY   0x3C
#define LIS3DH_REG_TIMEWINDOW    0x3D

_u8 SPIByte(_u8 tx) {
    _u8 rx = 0;

    int i;
    for(i = 0; i < 8; i++) {
        // Falling edge
        SPI_SCL_SET(0);
        SPI_MOSI_SET(tx & 0x80);
        UtilsDelay(SPI_DELAY);

        // Rising edge
        SPI_SCL_SET(1);
        rx = (rx << 1) | ((SPI_MISO_GET)? 0x01 : 0);
        UtilsDelay(SPI_DELAY);

        tx <<= 1;
    }

    return rx;
}

_u8 AccelReadRegister(_u8 addr)
{
    _u8 r;

    SPI_CS_SET(0);
    SPIByte(0x80 | (addr & 0x3f));
    r = SPIByte(0);
    SPI_CS_SET(1);

    return r;
}

void AccelWriteRegister(_u8 addr, _u8 value)
{
    SPI_CS_SET(0);
    SPIByte(addr & 0x3f);
    SPIByte(value);
    SPI_CS_SET(1);
}

void AccelInit()
{
	unsigned int whoami = AccelReadRegister(0x0f);
	ConsolePrintf("Accel Identification: %02x\n\r", whoami);

	UtilsDelay(30000000);

	AccelWriteRegister(LIS3DH_REG_CTRL1, 0x57);
	AccelWriteRegister(LIS3DH_REG_CTRL4, 0x88);

	// Setup interrupt-on-click
	AccelWriteRegister(LIS3DH_REG_CTRL3, 0x80); // turn on int1 click
	AccelWriteRegister(LIS3DH_REG_CTRL5, 0x08); // latch interrupt on int1
	AccelWriteRegister(LIS3DH_REG_CLICKCFG, 0x15); // turn on single tap on Z axis
	AccelWriteRegister(LIS3DH_REG_CLICKSRC, 0x10);


	AccelWriteRegister(LIS3DH_REG_CLICKTHS,  0x80 | 10); // Set thershold + LIR bit
	AccelWriteRegister(LIS3DH_REG_TIMELIMIT, 0x40); // arbitrary
	//  writeRegister8(LIS3DH_REG_TIMELATENCY, timelatency); // arbitrary
	AccelWriteRegister(LIS3DH_REG_TIMEWINDOW, 0x7f); // arbitrary



	_u8 status = 0;
/*
	while(1) {
	    status = AccelReadRegister(LIS3DH_REG_STATUS1) & 0x04;
	    if (status) {
	        unsigned int z = AccelReadRegister(LIS3DH_REG_OUT_Z_L) + (AccelReadRegister(LIS3DH_REG_OUT_Z_H) << 8);
	        ConsolePrintf("Z=%d\n\r", z);
	        UtilsDelay(10000);
	    }
	}
*/

    while(1) {
        //status = AccelReadRegister(LIS3DH_REG_CLICKSRC) & 0x07;
        status = GPIOPinRead(GPIOA0_BASE, 0x10);
        if (status) {
            //AccelReadRegister(LIS3DH_REG_CLICKSRC);
            UtilsDelay(50);

            ConsolePrintf("Click ");
        }
    }

}

			SPI_CS_ACTIVELOW | // Config: CS Active Low
			SPI_SW_CTRL_CS |   // Config: SW controlled CS
			SPI_4PIN_MODE |    // Config: 4 pin mode
			SPI_TURBO_OFF      // Config: Turbo Mode Off
	);

	SPIIntDisable(GSPI_BASE, 0);

    // Enable the SPI module
	SPIEnable(GSPI_BASE);
     */
	ConsolePrint("Reading SPI\n\r");
	unsigned int whoami = AccelReadRegister(0x0f);

	ConsolePrintf("Identification: %02x\n\r", whoami);
    UtilsDelay(30000000);

	AccelWriteRegister(LIS3DH_REG_CTRL1, 0x57);
	AccelWriteRegister(LIS3DH_REG_CTRL4, 0x88);

	// Setup interrupt-on-click
	AccelWriteRegister(LIS3DH_REG_CTRL3, 0x80); // turn on int1 click
	AccelWriteRegister(LIS3DH_REG_CTRL5, 0x08); // latch interrupt on int1
	AccelWriteRegister(LIS3DH_REG_CLICKCFG, 0x15); // turn on single tap on Z axis
	AccelWriteRegister(LIS3DH_REG_CLICKSRC, 0x10);


	AccelWriteRegister(LIS3DH_REG_CLICKTHS,  10); // Set thershold + LIR bit
	AccelWriteRegister(LIS3DH_REG_TIMELIMIT, 0x30); // arbitrary
	//  writeRegister8(LIS3DH_REG_TIMELATENCY, timelatency); // arbitrary
	//  writeRegister8(LIS3DH_REG_TIMEWINDOW, timewindow); // arbitrary



	_u8 status = 0;
/*
	while(1) {
	    status = AccelReadRegister(LIS3DH_REG_STATUS1) & 0x04;
	    if (status) {
	        unsigned int z = AccelReadRegister(LIS3DH_REG_OUT_Z_L) + (AccelReadRegister(LIS3DH_REG_OUT_Z_H) << 8);
	        ConsolePrintf("Z=%d\n\r", z);
	        UtilsDelay(10000);
	    }
	}
*/

    while(1) {
        //status = AccelReadRegister(LIS3DH_REG_CLICKSRC) & 0x07;
        status = GPIOPinRead(GPIOA0_BASE, 0x10);
        if (status) {
            AccelReadRegister(LIS3DH_REG_CLICKSRC);
            UtilsDelay(50);

            ConsolePrintf("Click ");
        }
    }

}


}
