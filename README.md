Matrix
======

Matrix Orbital Emulated Arduino HD44780 Device for Arduino Nano V3.0 (AtMega328P)

Edit: Do not use a capacitor for Contrast just connect it to the pin directly for lcd 20x02 and use a diode (1n4148) for lcd 16x02
I recommend using a small NPN transistor to drive the backlight, emitter to ground, collector to backlight anode and base connected with a small resistor to the uC BackLight PIN(i use BC337 and 12k resistor).

Modified contrast control and added stand-by after 11 second/auto-resume on Serial receive.

Also Included a small python script to send OpenHardwareMonitor data to the lcd.

Also Making a C# App(work in progress): https://github.com/Tek465B/OHM_To_MatrixO

![Breadboard](http://s10.postimg.org/roe0sj1ll/tisplay_breadboard.png)

Circuit Diagram

![Circuit Diagram](http://s23.postimg.org/ijjwnjtqz/tisplay_circuit.png)
