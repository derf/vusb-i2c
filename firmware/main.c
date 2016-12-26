/*
 * Copyright (c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH
 *       and (c) 2016 by Daniel Friesel
 * License: GNU GPL v2
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

#include "usbdrv.h"
#include "oddebug.h"
#include <util/delay.h>

USB_PUBLIC uchar usbFunctionSetup(uchar data[8])
{
	usbRequest_t    *rq = (void *)data;
	static uchar    replyBuf[2];
	static uchar    bv_sda, bv_scl; // bit vector, e.g. 0b01000000 for bit6
	static uchar    i;
	static uchar    buf; // r/w buffer for wValue.bytes[0]

	usbMsgPtr = replyBuf;

	if (rq->bRequest == USBCMD_ECHO) {
		replyBuf[0] = rq->wValue.bytes[0];
		replyBuf[1] = rq->wValue.bytes[1];
		return 2;
	}
	if (rq->bRequest == USBCMD_GETPORT) {
		replyBuf[0] = PINB;
		return 1;
	}
	if (rq->bRequest == USBCMD_SETPORT) {
		DDRB = ~(rq->wIndex.bytes[0]);
		return 0;
	}
	if (rq->bRequest == USBCMD_SETBITS) {
		bv_sda = rq->wIndex.bytes[0];
		bv_scl = rq->wIndex.bytes[1];
		return 0;
	}
	if (rq->bRequest == USBCMD_START) {
		DDRB &= ~(bv_sda | bv_scl);
		_delay_us(1000);
		DDRB |= bv_sda;
		_delay_us(1000);
		DDRB |= bv_scl;
		return 0;
	}
	if (rq->bRequest == USBCMD_STOP) {
		DDRB |= bv_sda;
		_delay_us(100);
		DDRB &= ~bv_scl;
		_delay_us(100);
		DDRB &= ~bv_sda;
		_delay_us(100);
		return 0;
	}
	if (rq->bRequest == USBCMD_TX) {
		buf = rq->wIndex.bytes[0]; // byte to transmit
		for (i = 0; i <= 8; i++) {
			if ((buf & 0x80) || (i == 8)) {
				DDRB &= ~bv_sda;
			} else {
				DDRB |= bv_sda;
			}
			buf <<= 1;
			_delay_us(100);
			DDRB &= ~bv_scl;
			_delay_us(100);
			if (i == 8) {
				if (PINB & bv_sda) {
					replyBuf[0] = 0;
				} else {
					replyBuf[0] = 1;
				}
			}
			DDRB |= bv_scl;
			_delay_us(100);
		}
		return 1;
	}
	if (rq->bRequest == USBCMD_RX) {
		buf = rq->wIndex.bytes[0]; // true <=> transmit ack
		DDRB &= ~bv_sda;
		replyBuf[0] = 0;
		for (i = 0; i <= 8; i++) {
			_delay_us(20);
			DDRB &= ~bv_scl;
			_delay_us(20);
			if ((i < 8) && ( PINB & bv_sda)) {
				replyBuf[0] |= _BV(7-i);
			}
			_delay_us(20);
			DDRB |= bv_scl;
			_delay_us(20);
			if ((i == 7) && buf) {
				DDRB |= bv_sda;
			}
			else if ((i == 8) && buf) {
				DDRB &= ~bv_sda;
			}
		}
		return 1;
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
	PORTD = _BV(1); /* turn on power LED */
	sei();
	for(;;){    /* main event loop */
		wdt_reset();
		usbPoll();
		if (TIFR & (1 << TOV0)){
			TIFR |= 1 << TOV0;  /* clear pending flag */
		if (PINB & _BV(PB7))
			PORTD = _BV(1); /* SCL high : turn on power LED */
		else
			PORTD = _BV(0); /* SCL low (busy) : turn on activity LED */
		}
	}
	return 0;
}
