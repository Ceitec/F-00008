#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>   /* need for usbdrv.h */
#include "usbdrv.h"
#include "encoder.h"
#include "oddebug.h"

static uchar reportBuffer[3] = {0,0,0} ;
static uchar idleRate;           /* in 4 ms units */
/*
PROGMEM const char usbHidReportDescriptor[25] = { // USB report descriptor
	0x05, 0x0c,                    // USAGE_PAGE (Consumer Devices)
	0x09, 0x01,                    // USAGE (Consumer Control)
	0xa1, 0x01,                    // COLLECTION (Application)
	0x85, 0x01,                    //   REPORT_ID (1)
	0x19, 0x00,                    //   USAGE_MINIMUM (Unassigned)
	0x2a, 0x3c, 0x02,              //   USAGE_MAXIMUM (AC Format)
	0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
	0x26, 0x3c, 0x02,              //   LOGICAL_MAXIMUM (572)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x75, 0x10,                    //   REPORT_SIZE (8)
	0x81, 0x00,                    //   INPUT (Data,Var,Abs)
	0xc0                           // END_COLLECTION
};
*/

#define USB_DESC_LENGHT 24

/*
PROGMEM const char usbHidReportDescriptor[20] = { // USB report descriptor
	0x05, 0x01,						// USAGE_PAGE (Consumer Devices)
	0x09, 0x04,						// USAGE (Consumer Control)
	0xa1, 0x01,						// COLLECTION (Application)
	0x85, 0x01,						//   REPORT_ID (1)
	0x15, 0x00,						//   LOGICAL_MINIMUM (0)
	0x26, 0xFF, 0x00,				//   LOGICAL_MAXIMUM (255)
	0x75, 0x08,						//   REPORT_SIZE (8)
	0x95, 0x01,						//   REPORT_COUNT (1)
	0x81, 0x00,						//   INPUT (Data,Var,Abs)
	0xc0							// END_COLLECTION
};

*/
PROGMEM const char usbHidReportDescriptor[72] = { // USB report descriptor
    0x05, 0x01,          // Usage page (desktop)
    0x09, 0x04,          // Usage (mouse)
    0xA1, 0x01,          // Collection (app)
    0x05, 0x09,          //		Usage page (buttons)
    0x19, 0x01,
    0x29, 0x03,
    0x15, 0x00,          //		Logical min (0)
    0x25, 0x01,          //		Logical max (1)
    0x95, 0x03,          //		Report count (3)
    0x75, 0x01,          //		Report size (1)
    0x81, 0x02,          //		Input (Data, Absolute)
    0x95, 0x01,          //		Report count (1)
    0x75, 0x05,          //		Report size (5)
    0x81, 0x03,          //		Input (Absolute, Constant)
    0x05, 0x01,          //		Usage page (desktop)
    0x09, 0x01,          //		Usage (pointer)
    0xA1, 0x00,          //		Collection (phys)
    0x09, 0x30,          //			Usage (x)
    0x09, 0x31,          //			Usage (y)
    0x15, 0x81,          //			Logical min (-127)
    0x25, 0x7F,          //			Logical max (127)
    0x75, 0x08,          //			Report size (8)
    0x95, 0x02,          //			Report count (2)
    0x81, 0x20, //			Input (Data, Rel=0x6, Abs=0x2)
    0xC0,                //		End collection
    0x09, 0x38,          // Usage (Wheel)
    0x95, 0x01,          // Report count (1)
    0x81, 0x02,          // Input (Data, Relative)
    0x09, 0x3C,          // Usage (Motion Wakeup)
    0x15, 0x00,          // Logical min (0)
    0x25, 0x01,          // Logical max (1)
    0x75, 0x01,          // Report size (1)
    0x95, 0x01,          // Report count (1)
    0xB1, 0x22,          // Feature (No preferred, Variable)
    0x95, 0x07,          // Report count (7)
    0xB1, 0x01,          // Feature (Constant)
    0xC0                 // End collection
};
/*
PROGMEM const char usbHidReportDescriptor[49] = { // USB report descriptor
	0x05, 0x01,						// USAGE_PAGE (Generic Desktop)
	0x09, 0x08,						// USAGE (Multi-Axis Controller)
	0xa1, 0x01,					    // COLLECTION (Application)
	0x85, 0x01,					    //   REPORT_ID (1)
	0x15, 0x00,					    //   LOGICAL_MINIMUM (0)
	0x26, 0x3c, 0x02,			    //   LOGICAL_MAXIMUM (572)
	0x95, 0x01,						//   REPORT_COUNT (1)
	0x75, 0x10,						//   REPORT_SIZE (16)
	0x81, 0x00,						//   INPUT (Data,Var,Abs)
	0xA1, 0x00,						//		COLLECTION(Physical)
	0x05 ,0x09,						//			Usage_Page (Button)
	0x19 ,0x01,						//			Usage_Minimum (Button 1)
	0x29 ,0x08,						//			Usage_Maximum (Button 8)
	0x15 ,0x00,						//			Logical_Minimum (0)
	0x25 ,0x01,						//			Logical_Maximum (1)
	0x05 ,0x01,						//			Usage_Page (Generic Desktop)
	0x09 ,0x30,						//			Usage (X)
	0x09 ,0x31,						//			Usage (Y)
	0x09 ,0x32,						//			Usage (Z)
	0x15 ,0x81,						//			Logical_Minimum (-127)
	0x25 ,0x7F,						//			Logical_Maximum (127)
	0x95 ,0x02,						//			Report_Count (2)
	0x81 ,0x02,						//			Input (Data, Var, Abs)
	0xc0,							// END_COLLECTION
	0xc0							// END_COLLECTION
};
*/
/*
PROGMEM const char usbHidReportDescriptor[48] = { // USB report descriptor
0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
0x09, 0x02,                    // USAGE (Mouse)
0xa1, 0x01,                    // COLLECTION (Application)
0x09, 0x01,                    //   USAGE (Pointer)
0xa1, 0x00,                    //   COLLECTION (Physical)
0x05, 0x09,                    //     USAGE_PAGE (Button)
0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
0x95, 0x03,                    //     REPORT_COUNT (3)
0x75, 0x01,                    //     REPORT_SIZE (1)
0x81, 0x02,                    //     INPUT (Data,Var,Abs)
0x95, 0x01,                    //     REPORT_COUNT (1)
0x75, 0x05,                    //     REPORT_SIZE (5)
0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
0x09, 0x30,                    //     USAGE (X)
0x09, 0x31,                    //     USAGE (Y)
0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
0x75, 0x08,                    //     REPORT_SIZE (8)
0x95, 0x02,                    //     REPORT_COUNT (2)
0x81, 0x06,                    //     INPUT (Data,Var,Rel)
0xc0,                          //   END_COLLECTION
0xc0                           // END_COLLECTION
};
*/
//DAF

/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	usbRequest_t	*rq = (void *)data;
	
    usbMsgPtr = reportBuffer;
	
    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS)	// class request type
	{
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
		
    }
	else
	{
        // no vendor specific requests implemented 
    }
	
	switch(rq->bRequest)
	{
		case 0:
			if (rq->bRequest)
			{
			}
			break;
		default:
			break;
	}
	return 0;
}
/* ------------------------------------------------------------------------- */

int main(void)
{
    uchar encstate;
    uchar Btnstate = 0;
    uchar LastBtnstate = 0;
	uchar LastKeyPress = 0;
	uchar KeyPressed = 0;
 	odDebugInit();
	ENC_InitEncoder();
    usbInit();
    usbDeviceDisconnect();  
    uchar i = 0;
    while(--i){             
        _delay_ms(1);

    }
    
    usbDeviceConnect();     

    sei();                  
	reportBuffer[0] = 1;  // ReportID = 1
	reportBuffer[2] = 0;  
    DBG1(0x00, 0, 0);
	while(1)
	{
      usbPoll();          
      ENC_PollEncoder();
	  
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
        
	  if(LastKeyPress != KeyPressed){
		 DBG1(0x01, reportBuffer, 3);
		 if (usbInterruptIsReady()){
			LastKeyPress = KeyPressed;
			reportBuffer[1] = KeyPressed;
			DBG1(0x01, reportBuffer, 3);
			/* use last key and not current key status in order to avoid lost
             changes in key status. */
			usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
		 }	
		/* This block sets the the number of additional keypress a volume key. 
		This increases the rate of change of volume of from 2 to 50 times
		The number of additional keypress sets by the variable AdditionalKeyPress. 	*/
		uchar AdditionalKeyPress = 0;
		while(AdditionalKeyPress--){ 
			if ((KeyPressed == 0xea)||(KeyPressed == 0xe9)){
				while (!(usbInterruptIsReady())){}
				usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
			}
		}//End of block		
      }
	}
    return 0;
}
/* ------------------------------------------------------------------------- */ 
