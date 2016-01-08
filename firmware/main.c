/* Name: main.c
 * Project: PowerSwitch based on AVR USB driver
 * Author: Christian Starkjohann
 * Creation Date: 2005-01-16
 * Tabsize: 4
 * Copyright: (c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt) or proprietary (CommercialLicense.txt)
 * This Revision: $Id$
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

#include "usbdrv.h"
#include "oddebug.h"
#include <util/delay.h>

/*
This module implements an 8 bit parallel output controlled via USB. It is
intended to switch the power supply to computers and/or other electronic
devices.

Application examples:
- Rebooting computers located at the provider's site
- Remotely switch on/off of rarely used computers
- Rebooting other electronic equipment which is left unattended
- Control room heating from remote
*/

#ifndef TEST_DRIVER_SIZE    /* define this to check out size of pure driver */

USB_PUBLIC uchar usbFunctionSetup(uchar data[8])
{
usbRequest_t    *rq = (void *)data;
static uchar    replyBuf[2];

    usbMsgPtr = replyBuf;
    if(rq->bRequest == USBCMD_ECHO) {  /* ECHO */
        replyBuf[0] = rq->wValue.bytes[0];
        replyBuf[1] = rq->wValue.bytes[1];
        return 2;
    }
    if(rq->bRequest == USBCMD_GETPORT) {  /* GET_STATUS -> result = 1 bytes */
        replyBuf[0] = PINB;
        return 1;
    }
    if(rq->bRequest == USBCMD_SETPORT) { /* SET_STATUS. Payload byte is output. */
        DDRB = ~(rq->wIndex.bytes[0]);
    }
    return 0;
}

/* allow some inter-device compatibility */
#if !defined TCCR0 && defined TCCR0B
#define TCCR0   TCCR0B
#endif
#if !defined TIFR && defined TIFR0
#define TIFR    TIFR0
#endif

int main(void)
{
uchar   i;

    wdt_enable(WDTO_1S);
    odDebugInit();
    DDRB = 0;
    DDRD = ~USBMASK;   /* all outputs except PD2 = INT0 */
    PORTD = 0;
    PORTB = 0;          /* no pullups on USB pins */
/* We fake an USB disconnect by pulling D+ and D- to 0 during reset. This is
 * necessary if we had a watchdog reset or brownout reset to notify the host
 * that it should re-enumerate the device. Otherwise the host's and device's
 * concept of the device-ID would be out of sync.
 */
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    i = 0;
    while(--i){         /* fake USB disconnect for > 500 ms */
        wdt_reset();
        _delay_ms(2);
    }
    usbDeviceConnect();
    TCCR0 = 5;          /* set prescaler to 1/1024 */
    usbInit();
    PORTD = _BV(0); /* turn on power LED */
    sei();
    for(;;){    /* main event loop */
        wdt_reset();
        usbPoll();
        if(TIFR & (1 << TOV0)){
            TIFR |= 1 << TOV0;  /* clear pending flag */
            if (PINB & _BV(PB7))
                PORTD = _BV(0); /* SCL high : turn on power LED */
            else
                PORTD = _BV(1); /* SCL low (busy) : turn on activity LED */
        }
    }
    return 0;
}

#else   /* TEST_DRIVER_SIZE */

/* This is the minimum do-nothing function to determine driver size. The
 * resulting binary will consist of the C startup code, space for interrupt
 * vectors and our minimal initialization. The C startup code and vectors
 * (together ca. 70 bytes of flash) can not directly be considered a part
 * of this driver. The driver is therefore ca. 70 bytes smaller than the
 * resulting binary due to this overhead. The driver also contains strings
 * of arbitrary length. You can save another ca. 50 bytes if you don't
 * include a textual product and vendor description.
 */
uchar   usbFunctionSetup(uchar data[8])
{
    return 0;
}

int main(void)
{
#ifdef PORTD
    PORTD = 0;
    DDRD = ~USBMASK;   /* all outputs except PD2 = INT0 */
#endif
    PORTB = 0;          /* no pullups on USB pins */
    DDRB = 0;    /* all outputs except USB data */
    usbInit();
    sei();
    for(;;){    /* main event loop */
        usbPoll();
    }
    return 0;
}

#endif  /* TEST_DRIVER_SIZE */
