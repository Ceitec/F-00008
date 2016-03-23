#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>   /* need for usbdrv.h */
#include "usbdrv.h"
#include "encoder.h"
#include "oddebug.h"
#include "ADC.h"

typedef struct{
	uchar	buttons;
	uchar	diodes;
	uchar	dx;
	uchar	dy;
	uchar	dz;
	uchar	dWheel;
}report_t;

static report_t reportBuffer;
static uchar idleRate;           /* in 4 ms units */

#ifdef USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH
	#undef USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH
#endif
	#define USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH	66
	

PROGMEM const char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = { // USB report descriptor
    0x06, 0xFF, 0x00,			// Usage page (desktop)
    0x09, 0x00,					// Usage (USB HID Joystick)
    0xA1, 0x01,					// Collection (app)
	// Všechny odchozí pøíkazy 
	0x09, 0x01,					//	USAGE(Pointer)
	0xA1, 0x00,					//	COLLECTION (Physical)
	// Tlaèítka
    0x05, 0x09,					//		Usage page (buttons)
    0x19, 0x01,					//		Usage minimum (1)
    0x29, 0x05,					//		Usage maximum (5)
    0x15, 0x00,					//		Logical min (0)
    0x25, 0x01,					//		Logical max (1)
    0x95, 0x05,			        //		Report count (5)
    0x75, 0x01,				    //		Report size (1)
	//LED diody
	0x05, 0x08,					//		USAGE PAGE (LEDs)
	//0x19, 0x01,					//		Usage minimum (1)
	//0x29, 0x05,					//		Usage maximum (5)
	0x09, 0x06,					//		Usage (POWER)
	0x09, 0x00,					//		Usage (UD)
	0x09, 0x00,					//		Usage (UD)
	0x09, 0x00,					//		Usage (UD)
	0x09, 0x00,					//		Usage (UD)
		
	0x15, 0x00,					//		Logical min (0)
	0x25, 0x01,					//		Logical max (1)
	0x95, 0x05,			        //		Report count (5)
	0x75, 0x01,				    //		Report size (1)
	
	//X, Y, Z a Zoom
	0x05 ,0x01,					//		Usage_Page (Generic Desktop)
	0x09, 0x30, 		        //		Usage (x)
    0x09, 0x31, 		        //		Usage (y)
	0x09, 0x32,					//		Usage (y)
	0x09, 0x38,					//		Usage (Wheel)
	0x15, 0x00,					//		Logical min (0)
	0x26, 0xFF, 0x00,			//		Logical max (255)
	0x95, 0x05,			        //		Report count (5)
	0x75, 0x08,				    //		Report size (1)
	0xC0,        		        // End collection
	0xC0        		        // End collection
};


/* ------------------------------------------------------------------------- */




usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	usbRequest_t	*rq = (void *)data;
	usbMsgPtr = (void *)&reportBuffer;
    switch(rq->bmRequestType & USBRQ_TYPE_MASK)
	{
		case USBRQ_TYPE_CLASS:
			if(rq->bRequest == USBRQ_HID_GET_REPORT)	  // wValue: ReportType (highbyte), ReportID (lowbyte)
			{
				// we only have one report type, so don't look at wValue
				DBG1(0x21,rq,8);
				return sizeof(reportBuffer);
			}
			else if(rq->bRequest == USBRQ_HID_GET_IDLE)
			{
				usbMsgPtr = &idleRate;
				DBG1(0x22,rq,8);
				return 1;
			}
			else if(rq->bRequest == USBRQ_HID_SET_IDLE)
			{
				DBG1(0x23,rq,8);
				idleRate = rq->wValue.bytes[1];
				
			}
			else if(rq->bRequest == USBRQ_HID_GET_PROTOCOL)
			{
				DBG1(0x24,rq,8);
				
			}
			else if(rq->bRequest == USBRQ_HID_SET_PROTOCOL)
			{
				DBG1(0x25,rq,8);
			}
			break;
		case USBRQ_TYPE_VENDOR:
			
			break;
		default:
			// no vendor specific requests implemented
			break;
	}
    
	return 0;
}
/* ------------------------------------------------------------------------- */

/*
PA0 - ADC0 - X osa
PA1 - ADC1 - Y osa
PA2 - ADC2 - Z osa
*/

int main(void)
{
    /*uchar encstate;
    uchar Btnstate = 0;
    uchar LastBtnstate = 0;
	uchar LastKeyPress = 0;
	uchar KeyPressed = 0;
	*/
 	//odDebugInit();
	//ENC_InitEncoder();
    usbInit();
    usbDeviceDisconnect();  
    uchar i = 0;
    while(--i)
	{             
        _delay_ms(1);
    }
    usbDeviceConnect();     
    sei();                  
	/*
	reportBuffer[0] = 1;  // ReportID = 1
	reportBuffer[1] = 1;
	reportBuffer[2] = 0;  
	*/
    //DBG1(0x00, 0, 0);
	/*
	reportBuffer.buttons = 0x12;
	reportBuffer.diodes = 0x34;
	reportBuffer.dx = 0x56;
	reportBuffer.dy = 0x78;
	reportBuffer.dz = 0x9A;
	reportBuffer.dWheel = 0xBC;*/
	while(1)
	{
		usbPoll();
		if (usbInterruptIsReady())
		{
			usbSetInterrupt((void *)&reportBuffer, sizeof(reportBuffer));
		}
		
      /*ENC_PollEncoder();
	  
      ///////////////////////////////////////////////
      
	  KeyPressed = 0;
	  encstate = ENC_GetStateEncoder();
      if (LEFT_SPIN == encstate)
      {
         KeyPressed = 0xea;
      }
      else if (RIGHT_SPIN == encstate)
      {
         KeyPressed = 0xe9;
      }
	  Btnstate = ENC_GetBtnState();
	  if (Btnstate != LastBtnstate)
	  {
	    if (Btnstate != 1) KeyPressed = 0xe2;				
		LastBtnstate = Btnstate;
	  }	
      
	  if(LastKeyPress != KeyPressed)
	  {
		 DBG1(0x01, reportBuffer, 3);
		 if (usbInterruptIsReady())
		 {
			LastKeyPress = KeyPressed;
			reportBuffer[1] = KeyPressed;
			DBG1(0x01, reportBuffer, 3);
			// use last key and not current key status in order to avoid lost changes in key status.
			usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
		 }	
		// This block sets the the number of additional keypress a volume key. 
		// This increases the rate of change of volume of from 2 to 50 times
		// The number of additional keypress sets by the variable AdditionalKeyPress.
		uchar AdditionalKeyPress = 0;
		while(AdditionalKeyPress--)
		{ 
			if ((KeyPressed == 0xea)||(KeyPressed == 0xe9))
			{
				while (!(usbInterruptIsReady())){}
				usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
			}
		}//End of block		
      }
	  */
	}
    return 0;
}
/* ------------------------------------------------------------------------- */ 
