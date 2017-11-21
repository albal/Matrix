/*
Name:		MatrixOrbital.ino
Created:	21/11/2017 14:13:51
Author:	BloodEdge
*/

/*
Al West, 2014, UK.
compiles to 6,240 bytes
Demonstrates the use a 20x4 LCD display as Matrix Orbital Device in LCD Smartie.
This should work all LCD displays that are compatible with the Hitachi HD44780 driver.
This sketch takes data from LCD Smartie emulating a Matrix Orbital and shows desired data.
The circuit:
+ LCD K   pin 16 Backlight GND   to GND
+ LCD A   pin 15 Backlight VDD   to digital pin 10 (PWM) Brightness
+ LCD D7  pin 14 Data 7          to digital pin 12
+ LCD D6  pin 13 Data 6          to digital pin 11
+ LCD D5  pin 12 Data 5          to digital pin 9
+ LCD D4  pin 11 Data 4          to digital pin 8
- LCD D3  pin 10                 NC
- LCD D2  pin 9                  NC
- LCD D1  pin 8                  NC
- LCD D0  pin 7                  NC
+ LCD E   pin 6  Enabled         to digital pin 6
+ LCD RW  pin 5  Read/Write      to digital pin 5
+ LCD RS  pin 4  Register Select to digital pin 4
+ LCD VO  pin 3  Contrast        to digital pin 3 (PWM) Contrast (use 100uf Cap between this pin and GND)
+ LCD 5V  pin 2  Supply          to 5V
+ LCD Gnd pin 1  Ground          to GND
D1-D4 not connected.
Public Domain - some parts used from other code in the Public Domain
some graphic functions not possible with HD44780 type devices but template code left in place
*/

// include the library code:
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <avr/power.h>
#include <avr/sleep.h>

// Bit Rate - using 19200 but LCD Smartie defaults to 9600
const int baud = 19200;

// EEPROM address values
const byte BRIGHTNESS = 0;	// Brightness
const byte CONTRAST = 1;	// Contrast
const byte SERIAL_LO = 2;	// Serial number low byte
const byte SERIAL_HI = 3;	// Serial number high byte

							// IO Pins
const byte GPIO = 13;		// D13 - Built in LED on Nano V3.0
const byte backLight = 10;	// D10 - Use PWM to change brightness
const byte contrast = 3;    // D3  - Use PWM to change contrast  Connect 100uF between pin and GND

							// Variables used in code - can probably optimise some of these out....
byte rxbyte;
byte temp;
byte addr;
byte level;
int val;
int i;
byte data[8];  // buffer for user character data
unsigned long previousMillis;
boolean pcready = 0;

// initialize the library with the numbers of the interface pins
// LiquidCrystal(rs, rw, enable, d4, d5, d6, d7) 
LiquidCrystal lcd(4, 5, 6, 8, 9, 11, 12);

void SleepNow() {
	analogWrite(backLight, 0);
	digitalWrite(contrast, HIGH);
	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_enable();
	power_adc_disable();
	power_spi_disable();
	power_timer0_disable();
	power_timer1_disable();
	power_timer2_disable();
	power_twi_disable();
	sleep_mode();
	sleep_disable();
	power_all_enable();
	analogWrite(backLight, EEPROM.read(BRIGHTNESS));
	digitalWrite(contrast, (EEPROM.read(CONTRAST) & 0x01));
}
// Setup
void setup() {
	// Set the use ouf our output pins
	pinMode(GPIO, OUTPUT);
	pinMode(backLight, OUTPUT);
	pinMode(contrast, OUTPUT);

	// Gather default values from EEPROM (could add code here to handle default if EEPROM has never been written to)
	digitalWrite(GPIO, LOW);
	analogWrite(backLight, EEPROM.read(BRIGHTNESS));    //  Read brightness level out of EEPROM
	digitalWrite(contrast, (EEPROM.read(CONTRAST) & 0x01));       // Read contrast level out of EEPROM

																  // set up the LCD's number of columns and rows: 
	lcd.begin(20, 4);
	lcd.clear();

	lcd.print("Booting... ");

	// Custom Characters for the TSEW Logo
	/*byte tsew1[8] = { B00000, B11111, B00100, B00100, B00100, B00100, B00000, B00000 };
	byte tsew2[8] = { B00000, B11111, B10000, B11111, B00001, B11111, B00000, B00000 };
	byte tsew3[8] = { B00000, B11111, B10000, B11110, B10000, B11111, B00000, B00000 };
	byte tsew4[8] = { B00000, B10001, B10101, B10101, B10101, B11111, B00000, B00000 };
	lcd.createChar(4, tsew1);
	lcd.createChar(5, tsew2);
	lcd.createChar(6, tsew3);
	lcd.createChar(7, tsew4);
	lcd.setCursor(1, 2);
	lcd.write(4);
	lcd.setCursor(2, 2);
	lcd.write(5);
	lcd.setCursor(3, 2);
	lcd.write(6);
	lcd.setCursor(4, 2);
	lcd.write(7);
	lcd.setCursor(8, 2);
	// End of custome character generation and display
	//lcd.print("Input Ready");
	//lcd.setCursor(8, 3);
	//lcd.print("19200,8,N,1");*/
	Serial.begin(baud);
	// Wait until we receive some data - note we want to hold what data it is as we use it the first iteration
	rxbyte = serial_getch();

	// Clear display, wait a little and load in the custom icons 
	lcd.clear();
	delay(200);

	// Generate 3 different bar graph values for 33% 66% and 100%
	/*byte bara[8] = { B10000, B10000, B10000, B10000, B10000, B10000, B11111, B10000 };
	byte barb[8] = { B11100, B11100, B11100, B11100, B11100, B11100, B11111, B11100 };
	byte barc[8] = { B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111 };
	// Characters for CPU, Nivida Graphics Card, C: drive and D: drive and degrees symbol (DegC)
	byte cpu[8] = { B10111, B10101, B10111, B00000, B11101, B10101, B11101, B00000 };
	byte nv[8] = { B11101, B10110, B00000, B11111, B11101, B11111, B01100, B00000 };
	byte cdrive[8] = { B01100, B10001, B10000, B01101, B00000, B11111, B11101, B11111 };
	byte ddrive[8] = { B11000, B10101, B10100, B11001, B00000, B11111, B11101, B11111 };
	byte degs[8] = { B11000, B11000, B00000, B00110, B01000, B01000, B00110, B00000 };
	// Write characters into LCD RAM
	lcd.createChar(1, barc);
	lcd.createChar(2, bara);
	lcd.createChar(3, barb);
	lcd.createChar(4, cpu);
	lcd.createChar(5, nv);
	lcd.createChar(6, cdrive);
	lcd.createChar(7, ddrive);
	lcd.createChar(8, degs);
	delay(20);
	lcd.clear();*/
}

// Helper function to read serial input as clean 8 bit byte
byte serial_getch() {
	int ch;
	while (Serial.available() == 0) {
		if (((long)(millis() - previousMillis) >= 11000) && pcready) {
			SleepNow();
		}
	}
	// read the incoming byte:
	previousMillis = millis();
	if (!pcready) pcready = true;
	ch = Serial.read();
	return (byte)(ch & 0xff);
}

// Main loop - note we come into it with rxbyte already set from exit of Setup() function
void loop() {
	if (rxbyte == 254) //Matrix Orbital uses 254 prefix for commands
	{
		switch (serial_getch())
		{
		case 52: //set serial number /*USING EEPROM*/
			EEPROM.write(SERIAL_HI, serial_getch());
			EEPROM.write(SERIAL_LO, serial_getch());
			break;
		case 53: //read serial number /*USING EEPROM*/
			Serial.write(EEPROM.read(SERIAL_HI));
			Serial.write(EEPROM.read(SERIAL_LO));
			break;
		case 54: //read version number
			Serial.write(0x11);        //'v1.1
			break;
		case 55: //read module type
			Serial.write(0x05);        //'lcd_type'}='LCD2041'
			break;
		case 59: //exit flow-control mode
			break;
		case 64: // EEPROM Write (address, value) 
			addr = serial_getch();
			val = serial_getch();
			EEPROM.write(addr, val);
			break;
		case 65: // EEPROM Read  (address)
			addr = serial_getch(); // EEPROM address
			val = EEPROM.read(addr); //
			Serial.print(val);
			break;
		case 66: //backlight on (at previously set brightness)
				 // not implemented      
			analogWrite(backLight, level);
			temp = serial_getch();   // time value - not used
			break;
		case 67: //auto line-wrap on
		case 68: //auto line-wrap off
			break;
		case 70: //backlight off
			analogWrite(backLight, 0);
			break;
		case 71:  //set cursor position
			temp = (serial_getch() - 1);  //get column byte    
			lcd.setCursor(temp, serial_getch() - 1);
			break;
		case 72:  //cursor home (reset display position)
			lcd.setCursor(0, 0);
			break;
		case 74:  //show underline cursor
			lcd.command(0b00001110);
			break;
		case 75:  //underline cursor off
		case 84:  //block cursor off
			lcd.command(0b00001100);
			break;
		case 76:  //move cursor left
			lcd.command(16);
			break;
		case 77:  //move cursor right
			lcd.command(20);
			break;
		case 78:  //define custom char
			temp = serial_getch();  // Character ram value
			for (i = 0; i < 8; i++) {
				data[i] = serial_getch();
			}
			lcd.createChar(temp, data);
			break;
		case 80:  // Set contrast (but we save anyway)
		case 145: // Set Contrast and save
			level = serial_getch(); // Contrast value
			digitalWrite(contrast, level);
			EEPROM.write(CONTRAST, (level & 0x01));
			break;
		case 81: //auto scroll on
		case 82: //auto scroll off
			break;
		case 83:  //show blinking block cursor
			lcd.command(0b00001111);
			break;
		case 86:  //GPO OFF
			temp = serial_getch(); // GPIO Pin
			digitalWrite(GPIO, LOW);
			break;
		case 87:  //GPO ON
			temp = serial_getch(); // GPIO Pin
			digitalWrite(GPIO, HIGH);
			break;
		case 88:  //clear display, cursor home
			lcd.clear();
			break;
		case 96: //auto-repeat mode off (keypad)
			break;
		case 98: //Draw bitmap
			temp = serial_getch(); // refid
			temp = serial_getch(); // x
			temp = serial_getch(); // y
			break;
		case 99: // set drawing color
			temp = serial_getch();
			break;
		case 101: //Draw line continue
			temp = serial_getch(); // x
			temp = serial_getch(); // y
			break;
		case 104: //init horiz bar graph
			temp = serial_getch(); // x1
			temp = serial_getch(); // y1
			temp = serial_getch(); // x2
			temp = serial_getch(); // y2
			break;
		case 108:
			temp = serial_getch(); // x1
			temp = serial_getch(); // y1
			temp = serial_getch(); // x2
			temp = serial_getch(); // y2
			break;
		case 109: //init med size digits
			break;
		case 112: // Draw Pixel
			temp = serial_getch(); // x
			temp = serial_getch(); // y
			break;
		case 114: //draw rect
			temp = serial_getch(); // colour
			temp = serial_getch(); // x1
			temp = serial_getch(); // y1
			temp = serial_getch(); // x2
			temp = serial_getch(); // y2
			break;
		case 115: //init narrow vert bar graph
		case 118: //init wide vert bar graph
			break;
		case 120: //draw rect solid
			temp = serial_getch(); // colour
			temp = serial_getch(); // x1
			temp = serial_getch(); // y1
			temp = serial_getch(); // x2
			temp = serial_getch(); // y2
			break;
			// case 145:  //defined above see case 80:
		case 152: //set brightness and save 
			level = serial_getch();
			analogWrite(backLight, level);
			EEPROM.write(BRIGHTNESS, level);
			break;
		case 153: //set backlight brightness
			level = serial_getch();
			analogWrite(backLight, level);
			break;
		default:
			//all other commands ignored and parameter byte discarded
			temp = serial_getch();  //dump the command code
			break;
		}
	} //END OF COMMAND HANDLER
	else
	{
		// Not a command character but still might be a special character
		//change accented char to plain, detect and change descenders
		switch (rxbyte)
		{
			//chars that have direct equivalent in LCD charmap
		case 0xE4: //ASCII "a" umlaut
			rxbyte = 0xE1;
			break;
		case 0xF1: //ASCII "n" tilde
			rxbyte = 0xEE;
			break;
		case 0xF6: //ASCII "o" umlaut
			rxbyte = 0xEF; //was wrong in v0.86
			break;
		case 0xFC: //ASCII "u" umlaut
			rxbyte = 0xF5;
			break;
			//accented -> plain equivalent
			//and misc symbol translation
		case 0xA3: //sterling (pounds)
			rxbyte = 0xED;
			break;
		case 0xAC: //sterling (pounds)
			rxbyte = 0xB0;
			break;
		case 0xB0: //degrees symbol
			rxbyte = 0xDF;
			break;
		case 0xB5: //mu
			rxbyte = 0xE4;
			break;
		case 0xC0: //"A" variants
		case 0xC1:
		case 0xC2:
		case 0xC3:
		case 0xC4:
		case 0xC5:
			rxbyte = 0x41;
			break;
		case 0xC8: //"E" variants
		case 0xC9:
		case 0xCA:
		case 0xCB:
			rxbyte = 0x45;
			break;
		case 0xCC: //"I" variants
		case 0xCD:
		case 0xCE:
		case 0xCF:
			rxbyte = 0x49;
			break;
		case 0xD1: //"N" tilde -> plain "N"
			rxbyte = 0x43;
			break;
		case 0xD2: //"O" variants
		case 0xD3:
		case 0xD4:
		case 0xD5:
		case 0xD6:
		case 0xD8:
			rxbyte = 0x4F;
			break;
		case 0xD9: //"U" variants
		case 0xDA:
		case 0xDB:
		case 0xDC:
			rxbyte = 0x55;
			break;
		case 0xDD: //"Y" acute -> "Y"
			rxbyte = 0x59;
			break;
		case 0xE0: //"a" variants except umlaut
		case 0xE1:
		case 0xE2:
		case 0xE3:
		case 0xE5:
			rxbyte = 0x61;
			break;
		case 0xE7: //"c" cedilla -> "c"
			rxbyte = 0x63;
			break;
		case 0xE8: //"e" variants
		case 0xE9:
		case 0xEA:
		case 0xEB:
			rxbyte = 0x65;
			break;
		case 0xEC: //"i" variants
		case 0xED:
		case 0xEE:
		case 0xEF:
			rxbyte = 0x69;
			break;
		case 0xDF: //beta  // LCDSmartie degree symbol??
		case 0xF2: //"o" variants except umlaut
		case 0xF3:
		case 0xF4:
		case 0xF5:
		case 0xF8:
			rxbyte = 0x6F;
			break;
		case 0xF7: //division symbol
			rxbyte = 0xFD;
			break;
		case 0xF9: //"u" variants except umlaut
		case 0xFA:
		case 0xFB:
			rxbyte = 0x75;
			break;
		default:
			break;
		}
		// By now either special character is converted or it was printable already
		lcd.print((char)rxbyte);  //print it to lcd
	}
	rxbyte = serial_getch();    // Wait for the next byte and use value for next iteration
}