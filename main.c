/* Name: main.c
 * Project: hid-mouse, a very simple HID example
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-07
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 */

/*
This example should run on most AVRs with only little changes. No special
hardware resources except INT0 are used. You may have to change usbconfig.h for
different I/O pins for USB. Please note that USB D+ must be the INT0 pin, or
at least be connected to INT0 as well.

We use VID/PID 0x046D/0xC00E which is taken from a Logitech mouse. Don't
publish any hardware using these IDs! This is for demonstration only!
*/

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"
#include "oddebug.h"        /* This is also an example for using debug macros */
#include "ADC.h"
#include "encoder.h"
#include "common_defs.h"



volatile uint8_t Vystup[3];
volatile uint8_t encval = 0;
volatile uint16_t counter = 0;      //Èítaè

// Pøerušení Encoderu (ètení a zapsání hodnoty do èítaèe)
ISR(PCINT_Vect_Part)
{
	static uint8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
	static uint8_t old_AB = 0;
	/**/
	old_AB <<= 2;                   //remember previous state
	old_AB |= ( ENC_PORT & 0x03 );  //add current state
	encval = ( enc_states[( old_AB & 0x0f )]);
	if( encval )
	{
		counter += encval;
	}
}


/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------ Descriptor ------------------------------- */

PROGMEM const char usbHidReportDescriptor[62] = {
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x04,                    // USAGE (Joystick)
	0xa1, 0x01,                    // COLLECTION (Application)
	0x09, 0x01,                    //   USAGE (Pointer)
	0xa1, 0x00,                    //   COLLECTION (Physical)
	0x85, 0x01,                    //     REPORT_ID (1)
	0x05, 0x09,                    //     USAGE_PAGE (Button)
	0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
	0x29, 0x08,                    //     USAGE_MAXIMUM (Button 8)
	0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
	0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
	0x95, 0x08,                    //     REPORT_COUNT (8)
	0x75, 0x01,                    //     REPORT_SIZE (1)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0x09, 0x38,                    //     USAGE (Wheel)
	0x15, 0x00,				       //     LOGICAL_MINIMUM (0)
	0x26, 0xFF, 0x03,			   //     LOGICAL_MAXIMUM (1023)
	0x75, 0x10,                    //     REPORT_SIZE (10)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	
	0x85, 0x02,                    //     REPORT_ID (2)
	/*
nejprve do report id 1 dát tlaèítka a wheel a report id 2 bude x y a z. vždy usage_page a mezi input musí bejt size count atd o tom....
	0x75, 0x05,                    //     REPORT_SIZE (5) 
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x81, 0x03,                    //     INPUT (Const,Var,Abs)
*/
	//0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
	
	0x09, 0x30,                    //     USAGE (X)
	0x09, 0x31,                    //     USAGE (Y)
	0x09, 0x32,                    //     USAGE (Z)	
	0x15, 0x00,				       //     LOGICAL_MINIMUM (0)
	0x26, 0xFF, 0x03,			   //     LOGICAL_MAXIMUM (1023)
	0x75, 0x10,                    //     REPORT_SIZE (8)
	0x95, 0x03,                    //     REPORT_COUNT (3)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0xC0,                          //   END_COLLECTION
	0xC0,                           //               END_COLLECTION
};

/* This is the same report descriptor as seen in a Logitech mouse. The data
 * described by this descriptor consists of 4 bytes:
 *     B7 B6 B5 B4 B3 B2 B1 B0 .... one byte with mouse button states
 *     X7 X6 X5 X4 X3 X2 X1 X0 .... 8 bit signed relative coordinate x
 *     Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 .... 8 bit signed relative coordinate y
 *     W7 W6 W5 W4 W3 W2 W1 W0 .... 8 bit signed relative coordinate wheel
 */

//Definování pinù pro X, Y a Z osu
#define	AxisX_PIN	PINA3
#define	AxisY_PIN	PINA4
#define	AxisZ_PIN	PINA5

//Definování struktury ReportID1
typedef struct{
	uint8_t		ReportID;
    uint8_t		buttonMask;
    uint16_t	dWheel;
}report_t1;

//Definování struktury ReportID2
typedef struct{
	uint8_t		ReportID;
	uint16_t	dx;
	uint16_t	dy;
	uint16_t	dz;
}report_t2;

static report_t1 reportBuffer_REPORT1;
static report_t2 reportBuffer_REPORT2;
static uchar    idleRate;   /* repeat rate for keyboards, never used for mice */


/* The following function advances sin/cos by a fixed angle
 * and stores the difference to the previous coordinates in the report
 * descriptor.
 * The algorithm is the simulation of a second order differential equation.
 */

/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
usbRequest_t    *rq = (void *)data;

    /* The following requests are never used. But since they are required by
     * the specification, we implement them in this example.
     */
    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
        DBG1(0x50, &rq->bRequest, 1);   /* debug output: print our request */
        if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
            /* we only have one report type, so don't look at wValue */
            usbMsgPtr = (void *)&reportBuffer_REPORT1;
            return sizeof(reportBuffer_REPORT1);
        }else if(rq->bRequest == USBRQ_HID_GET_IDLE){
            usbMsgPtr = &idleRate;
            return 1;
        }else if(rq->bRequest == USBRQ_HID_SET_IDLE){
            idleRate = rq->wValue.bytes[1];
        }
    }else{
        /* no vendor specific requests implemented */
    }
    return 0;   /* default for not implemented requests: return no data back to host */
}

/* ------------------------------------------------------------------------- */

int main(void)
{
	uchar   i;
    wdt_enable(WDTO_1S);
    /* Even if you don't use the watchdog, turn it off here. On newer devices,
     * the status of the watchdog (on/off, period) is PRESERVED OVER RESET!
     */
    /* RESET status: all port bits are inputs without pull-up.
     * That's the way we need D+ and D-. Therefore we don't need any
     * additional hardware initialization.
     */
    odDebugInit();
    DBG1(0x00, 0, 0);       /* debug output: main starts */
    usbInit();
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    i = 0;
    while(--i)
	{             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
    }
    usbDeviceConnect();
    sei();
    DBG1(0x01, 0, 0);       /* debug output: main loop starts */
	reportBuffer_REPORT1.ReportID=0x01;
	reportBuffer_REPORT1.buttonMask=0x01;
	reportBuffer_REPORT1.dWheel=8466;
	
	//Výchozí hodnoty Reportù.
	reportBuffer_REPORT1.ReportID = 0x01;
	reportBuffer_REPORT2.ReportID = 0x02;
	// Inicializace ADC pøevodníku
	InitADC();
	
	//volatile counter=0;
	ENC_InitEncoder();
	ENC_Intterupt_Set();
	
	sei();
	
    while(1)
	{                /* main event loop */
        DBG1(0x02, 0, 0);   /* debug output: main loop iterates */
        wdt_reset();
        usbPoll();
        
		reportBuffer_REPORT1.dWheel = (counter & 0x03FF);
		while(!usbInterruptIsReady())
		{
			usbPoll();
        }
		usbSetInterrupt((void *)&reportBuffer_REPORT1, sizeof(reportBuffer_REPORT1));
		
		// Pøeètení ADC hodnoty pinu X,Y a Z.
		reportBuffer_REPORT2.dx = ReadADC(AxisX_PIN);
		reportBuffer_REPORT2.dy = ReadADC(AxisY_PIN);
		reportBuffer_REPORT2.dz = ReadADC(AxisZ_PIN);
		while(!usbInterruptIsReady())
		{
			usbPoll();
		}
		usbSetInterrupt((void *)&reportBuffer_REPORT2, sizeof(reportBuffer_REPORT2));
    }
}

/* ------------------------------------------------------------------------- */
