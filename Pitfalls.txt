If something does not work as it should, please check the following list.
It is compiled from the feedback we received so far.


FIRMWARE:
- If you use an ATMega or other device with internal RC oscillator, make sure
  that you flash the fuse for the external crystal oscillator.
- Make sure that all USB inputs (D+, D- and the interrupt) are configured as
  inputs with no internal pullup resistor. You must write a "0" to the port
  output bit and to the data direction bit of each input line.
- Make sure that you have updated the defines in usbconfig.h to reflect your
  pin assignments.


HOST DRIVER:
- On Linux you must be root in order to use libusb (unless you have configured
  hotplug appropriately). The command line utility must therefore be executed
  by root.
- Our host software can be compiled on Windows using MinGW and libusb-win32.
- libusb-win32 version 0.1.10.1 (others have not been tested) has a bug with
  suspend mode. Once the PC was in suspend mode, the device is "lost".


HARDWARE:
- EMI: Our reference implementation is susceptible to EMI problems. We found
  out that the chip performs a reset although the external reset pin does
  not change level (checked with a DSO connected to reset and triggered by
  USB reset). This may be a consequence of over-clocking (we had the AT90S2313
  in stock and it just worked). We recommend EMI precautions in any case:
  (1) Use the watchdog timer of the device, (2) generate an USB reset state
  for an extended period of time after device reset (this is outside the USB
  spec, but it works: the hub or host interprets it as disconnect). Otherwise
  the host's and device's notion of the device-ID may get out of sync.
- If you use the EEPROM, it is very likely that you need some kind of brownout
  protection. A capacitor of several 100 microfarads in the supply is enough
  to reliably cause brownout problems. See Atmel's Application Note AN180
  for more information. Your local supplier's price table may be a better
  selection guide for brownout protection chips, though. Newer AVRs have
  brownout protection built-in. It can be configured with the fuse bits.
- Operating Voltage: PowerSwitch has been designed to operate at 3.5 V in
  order to meet the data signal voltage specification. The circuit does
  not work reliably with 5 V levels on D+ and D-. These levels exceed the
  common mode range of the USB I/O driver chips found in some hosts and hubs.
  We therefore recommend that you either reduce the CPU's supply voltage or
  add 3.6 V zener diodes to USB D+ and D- to clamp the voltage at 3.3 V.


AVR DEVICES CONFIRMED TO WORK:
Although the driver should work with all AVR microcontrollers which meet the
criteria, it may be good to know that somebody has actually tried a particular
chip. If you have built an AVR-USB project with a chip not listed here,
please drop us a note. The following chips have been tested:

  AT90S2313   works, although it is overclocked
  ATTiny2313
  ATTiny45
  ATMega8
  ATMega88
  ATMega8535
  ATMega32

