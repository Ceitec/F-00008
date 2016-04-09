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
#include "Joystick.h"
#include "ShiftRegister.h"

//Definování pinù pro X, Y a Z osu
#define	AxisX_PIN	PINA3
#define	AxisY_PIN	PINA4
#define	AxisZ_PIN	PINA5

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------ Descriptor ------------------------------- */

PROGMEM const char usbHidReportDescriptor[137] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x04,                    // USAGE (Joystick)
    0xa1, 0x01,                    // COLLECTION (Application)
    0xa1, 0x00,                    //   COLLECTION (Physical)
	// Report ID 1
	/*
	1 Byte - Tlaèítka
	2 Byte - Wheel (Zoom)
	1 Byte - LED Status
	2 Byte - Sedmisegment status
	*/
    0x85, 0x01,                    //     REPORT_ID (1)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
    0x29, 0x08,                    //     USAGE_MAXIMUM (Button 8)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x95, 0x08,                    //     REPORT_COUNT (8)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x38,                    //     USAGE (Wheel)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x03,              //     LOGICAL_MAXIMUM (1023)
    0x75, 0x10,                    //     REPORT_SIZE (16)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x05, 0x08,                    //     USAGE_PAGE (LEDs)
    0x19, 0x01,                    //     USAGE_MINIMUM (Num Lock)
    0x29, 0x08,                    //     USAGE_MAXIMUM (Do Not Disturb)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x95, 0x08,                    //     REPORT_COUNT (8)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x05, 0x08,                    //     USAGE_PAGE (LEDs)
    0x19, 0x01,                    //     USAGE_MINIMUM (Num Lock)
    0x29, 0x10,                    //     USAGE_MAXIMUM (Repeat)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x27, 0xff, 0xff, 0x00, 0x00,  //     LOGICAL_MAXIMUM (65535)
    0x75, 0x10,                    //     REPORT_SIZE (16)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	// Report ID 2
	/*
	2 Byte - X osa
	2 Byte - Y osa
	2 Byte - Z osa
	*/
    0x85, 0x02,                    //     REPORT_ID (2)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x09, 0x32,                    //     USAGE (Z)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x03,              //     LOGICAL_MAXIMUM (1023)
    0x75, 0x10,                    //     REPORT_SIZE (16)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	// Report ID 3
	/*
	Vstupní data
	1 Byte - LED (Tlaèítka) set
	2 Byte - Segment set
	*/
    0x85, 0x03,                    //     REPORT_ID (3)
    0x05, 0x08,                    //     USAGE_PAGE (LEDs)
    0x19, 0x01,                    //     USAGE_MINIMUM (Num Lock)
    0x29, 0x03,                    //     USAGE_MAXIMUM (Scroll Lock)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //     LOGICAL_MAXIMUM (255)
    0x91, 0x02,                    //     OUTPUT (Data,Var,Abs)
    0x05, 0x08,                    //     USAGE_PAGE (LEDs)
    0x19, 0x01,                    //     USAGE_MINIMUM (Num Lock)
    0x29, 0x0a,                    //     USAGE_MAXIMUM (Tone Enable)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x27, 0xff, 0xff, 0x00, 0x00,  //     LOGICAL_MAXIMUM (65535)
    0x75, 0x10,                    //     REPORT_SIZE (16)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x91, 0x02,                    //     OUTPUT (Data,Var,Abs)
    0xc0,                          //   END_COLLECTION
    0xc0                           // END_COLLECTION
};

/* This is the same report descriptor as seen in a Logitech mouse. The data
 * described by this descriptor consists of 4 bytes:
 *		B07 B06 B05 B04 B03 B02 B01 B00 .... 8 bit with mouse button states
 *		X15 X14 X13 X12 X11 X10 X09 X08 .... First 8 bit signed relative coordinate x
 *		X07 X06 X05 X04 X03 X02 X01 X00 .... Second 8 bit signed relative coordinate x
 *		Y15 Y14 Y13 Y12 Y11 Y10 Y09 Y08 .... First 8 bit signed relative coordinate y
 *		Y07 Y06 Y05 Y04 Y03 Y02 Y01 Y00 .... Second 8 bit signed relative coordinate y
 *		W15 W14 W13 W12 W11 W10 W09 W08 .... First 8 bit signed relative coordinate wheel
 *		W07 W06 W05 W04 W03 W02 W01 W00 .... Second 8 bit signed relative coordinate wheel
 *		L07 L06 L05 L04 L03 L02 L01 L00 .... 8 bit with indicate LED status
 *		
 
 */

//Definování struktury ReportID1
typedef struct{
	uint8_t		ReportID;
	uint8_t		buttonMask;
	uint16_t	dWheel;
	uint8_t		LED_Status;
	uint16_t	Segment;
}report_IN_1;

//Definování struktury ReportID2
typedef struct{
	uint8_t		ReportID;
	uint16_t	dx;
	uint16_t	dy;
	uint16_t	dz;
}report_IN_2;

typedef struct{
	uint8_t		LED_set;
	uint16_t	Segment_set;
}report_OUT;


static report_IN_1 reportBuffer_Status;
static report_IN_2 reportBuffer_Axis;


//Nastavení Encoderu
volatile uint8_t Vystup[3];
volatile uint8_t encval = 0;
volatile uint16_t counter = 0;      //Èítaè
uint16_t led_status, led_status_temp;
uint8_t	zmena=1;
uint8_t i_word=0;
uint8_t Clock_1ms=0;

// Pøerušení pro shift register	
ISR(TIMER0_COMPA_vect)
{
	TCNT0 = 0;
	if ((i_word == 31) || (i_word == 32))
	{
		sbi(Display_PORT, Display_ALL);
		i_word++;
	}
	else if ((i_word == 33) || (i_word == 34))
	{
		nbi(Display_PORT, Display_ALL);
		led_status_temp = led_status;
		i_word = 0;
	}
	else
	{
		nbi(Display_PORT, Display_ALL);	
		if (zmena)
		{
			cbi(Display_PORT, Display_CLOCK); // Vytváøení clocku po 1ms
			if ((led_status_temp & 0x8000) == 0x8000)
			{
				sbi(Display_PORT, Display_DATA);
			} 
			else // if ((led_status_temp & 0x0000) == 0x0000)
			{
				cbi(Display_PORT, Display_DATA);
			}
			led_status_temp = led_status_temp << 1;
			zmena = 0;
		}
		else
		{
			sbi(Display_PORT, Display_CLOCK); // Vytváøení clocku po 1ms	
			zmena = 1;
		}
		i_word++;
	}
}


//Pøerušení pro Tlaèítka a nastavení LED diod.
ISR(PCINT2_vect)
{
	uint8_t PORT_Temp;
	PORT_Temp = TL_PORT_IN;
	if (((PORT_Temp & TL_MASK) & TL_VYP) == TL_VYP)
	{
		//Aktivní tlaèítko Vypnutí
		sbi(LED_PORT_OUT, LED_VYP);
		reportBuffer_Status.LED_Status ^= (1 << 0);
	}
	if (((PORT_Temp & TL_MASK) & TL_LEFT) == TL_LEFT)
	{
		//Aktivní tlaèítko Levý
		sbi(LED_PORT_OUT, LED_LEFT);
		reportBuffer_Status.LED_Status ^= (1 << 1);
	}
	if (((PORT_Temp & TL_MASK) & TL_RIGHT) == TL_RIGHT)
	{
		//Aktivní tlaèítko Pravý
		sbi(LED_PORT_OUT, LED_RIGHT);
		reportBuffer_Status.LED_Status ^= (1 << 2);
	}
	if (((PORT_Temp & TL_MASK) & TL_OK) == TL_OK)
	{
		//Aktivní tlaèítko Ok
		sbi(LED_PORT_OUT, LED_OK);
		reportBuffer_Status.LED_Status ^= (1 << 3);
	}
	if (((PORT_Temp & TL_MASK) & TL_XYZ) == TL_XYZ)
	{
		//Aktivní tlaèítko Ok
		reportBuffer_Status.LED_Status ^= (1 << 4);
	}
}


// Pøerušení Encoderu (ètení a zapsání hodnoty do èítaèe)
ISR(PCINT_Vect_Part)
{
	static uint8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
	static uint8_t old_AB = 0;
	/**/
	old_AB <<= 2;                   //remember previous state
	old_AB |= ( ENC_PORT & 0x03 );  //add current state
	encval = ( enc_states[( old_AB & 0x0f )]);
	if( encval == 0xFF)
	{
		counter++;
	}
	else if (encval == 0x01)
	{
		counter--;
	}
}


/* ------------------------------Zaèátek USB protokolu--------------------------------- */

static uint8_t buffer[16];

uchar   usbFunctionRead(uchar *data, uchar len)
{
    uint8_t i;

   for (i=0;i<len;++i) data[i]=buffer[i];

   return len;
}

/* usbFunctionWrite() is called when the host sends a chunk of data to the
 * device. For more information see the documentation in usbdrv/usbdrv.h.
 */
uchar   usbFunctionWrite(uchar *data, uchar len)
{
    uint8_t i;

   for (i=0;i<len;++i) buffer[i]=data[i];
   reportBuffer_Status.LED_Status = buffer[1];
   reportBuffer_Status.Segment = buffer[2] + ((unsigned short) buffer[3] << 8);
      
   return 1;               /* end of transfer */
}

/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
usbRequest_t    *rq = (void *)data;

    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* HID class request */
        if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
            /* since we have only one report type, we can ignore the report-ID */
            return USB_NO_MSG;  /* use usbFunctionRead() to obtain data */
        }else if(rq->bRequest == USBRQ_HID_SET_REPORT){
            /* since we have only one report type, we can ignore the report-ID */
            return USB_NO_MSG;  /* use usbFunctionWrite() to receive data from host */
        }
    }else{
        /* ignore vendor type requests, we don't use any */
    }
    return 0;
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
	
	reportBuffer_Status.ReportID=0x01;
	reportBuffer_Status.buttonMask=0x01;
	reportBuffer_Status.dWheel=8466;
	
	//Výchozí hodnoty Reportù.
	reportBuffer_Status.ReportID = 0x01;
	reportBuffer_Axis.ReportID = 0x02;
	//Inicializace
	reportBuffer_Status.LED_Status = 0xFF;
	reportBuffer_Status.Segment = 0x3031;
	// Inicializace ADC pøevodníku
	InitADC();
	
	//volatile counter=0;
	ENC_InitEncoder();
	ENC_Intterupt_Set();
	led_status_temp = led_status;
	sei();
	
    while(1)
	{                /* main event loop */
        DBG1(0x02, 0, 0);   /* debug output: main loop iterates */
        wdt_reset();
        usbPoll();
        
		reportBuffer_Status.dWheel = counter;
		while(!usbInterruptIsReady())
		{
			usbPoll();
        }
		usbSetInterrupt((void *)&reportBuffer_Status, sizeof(reportBuffer_Status));
		
		// Pøeètení ADC hodnoty pinu X,Y a Z.
		reportBuffer_Axis.dx = ReadADC(AxisX_PIN);
		reportBuffer_Axis.dy = ReadADC(AxisY_PIN);
		reportBuffer_Axis.dz = ReadADC(AxisZ_PIN);
		while(!usbInterruptIsReady())
		{
			usbPoll();
		}
		usbSetInterrupt((void *)&reportBuffer_Axis, sizeof(reportBuffer_Axis));
    }
}

/* ------------------------------------------------------------------------- */
