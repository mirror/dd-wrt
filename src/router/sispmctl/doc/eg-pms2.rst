EnerGenie EG-PMS2
=================

Controller board
----------------

On the controller circuit board are:

* microcontroller Atmel MEGA88PA
* real time clock S35390
* battery for running the real time clock without power
  (may need replacement after several years of service)

Schedule
--------

Schedules are stored in a 40 byte buffer per port.

The format deviates from prior EnerGenie and Gembird devices.

The first 5 bytes are:

* 1 byte indicating the port: 3 * port-number + 1
* 4 bytes seconds since 1970-01-01 in LSB format indicating time of programming

The next groups of five bytes indicate the scheduled actions:

* 1 byte where
  bit 0 indicates the action: 0 off, 1 on, and
  bit 1 indicates if the action shall be repeated in a loop.
* 4 bytes seconds since 1970-01-01 in LSB format indicating the scheduled time

The last 5 bytes are:

* 1 byte 0xE5
* 4 bytes loop duration in seconds in LSB format, 0x00000000 indicates no loop

The rest of the buffer is filled with 0x00

On a factory fresh device the content of the buffer is:

* 1 byte indicating the port: 3 * port number + 1
* 38 bytes 0x55
* 1 byte 0x00
