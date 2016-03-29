// Console (UART) output functions
// LICENSE: GPLv3

#include <stdlib.h>
#include <string.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "prcm.h"
#include "pin.h"
#include "uart.h"
#include "rom.h"
#include "rom_map.h"

#include "settings.h"

#include "console.h"

char g_cConsoleBuffer[CONSOLE_BUFFER_SIZE];
int g_iConsoleWritePtr;
int g_iConsoleReadPtr;
int g_iConsoleLength;

// Local forwards
void _ConsoleProcessTX();
void _TxIntHandler();

// Initialize the console
void ConsoleInit()
{
	// Configure the UART
	MAP_UARTConfigSetExpClk(CONSOLE,
			MAP_PRCMPeripheralClockGet(CONSOLE_PERIPH),
	        CONSOLE_BAUD_RATE,
			(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

	// Configure the interrupt to fire on end-of-transimission
	MAP_UARTTxIntModeSet(CONSOLE, UART_TXINT_MODE_EOT);
	MAP_UARTIntRegister(CONSOLE, _TxIntHandler);

	// Clear the console buffer
	memset(g_cConsoleBuffer, 0, sizeof(g_cConsoleBuffer));
	g_iConsoleWritePtr = 0;
	g_iConsoleReadPtr = 0;
	g_iConsoleLength = 0;
}

// Print a message to the console. This function copies the message
// to the console buffer, and it is non-blocking. If there is not enough space
// in the buffer, the message will be truncated and the last printed character
// will be CONSOLE_NOSPACE
void ConsolePrint(const char* pStr)
{
	const char* p = pStr;

	// Disable interrupts while wer'e working with the buffer
	MAP_UARTIntDisable(CONSOLE, UART_INT_TX);

	// Copy the message
	while ((*p != 0) && (g_iConsoleLength < CONSOLE_BUFFER_SIZE)) {
		g_cConsoleBuffer[g_iConsoleWritePtr] = *p;
		p++;
		g_iConsoleWritePtr = (g_iConsoleWritePtr + 1) % CONSOLE_BUFFER_SIZE;
		g_iConsoleLength++;
	}

#ifdef CONSOLE_TRUNCATE_SYM
	// Add truncation symbol
	if (*p != 0) {
		// Truncation has occured
		g_cConsoleBuffer[(CONSOLE_BUFFER_SIZE - g_iConsoleWritePtr - 1) % CONSOLE_BUFFER_SIZE] = CONSOLE_TRUNCATE_SYM;
	}
#endif

	// Kick the sending process (if not already in progress)
	_ConsoleProcessTX();

	// Enable interrupt
	MAP_UARTIntEnable(CONSOLE, UART_INT_TX);

}

void _ConsoleProcessTX()
{
	while(g_iConsoleLength > 0) {
		// There are characters to send
		tBoolean r = MAP_UARTCharPutNonBlocking(CONSOLE, g_cConsoleBuffer[g_iConsoleReadPtr]);

		if (r) {
			// The character was put successfully
			g_iConsoleReadPtr = (g_iConsoleReadPtr + 1) % CONSOLE_BUFFER_SIZE;
			g_iConsoleLength--;
		}
		else
			// No space in the UART buffer
			return;
	}
}


void _TxIntHandler()
{
	MAP_UARTIntClear(CONSOLE, UART_INT_TX);
	_ConsoleProcessTX();
}
