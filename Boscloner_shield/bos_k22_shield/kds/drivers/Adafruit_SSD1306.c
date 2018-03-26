/*********************************************************************
 This is a library for our Monochrome OLEDs based on SSD1306 drivers

 Pick one up today in the adafruit shop!
 ------> http://www.adafruit.com/category/63_98

 These displays use SPI to communicate, 4 or 5 pins are required to
 interface

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 Written by Limor Fried/Ladyada  for Adafruit Industries.
 BSD license, check license.txt for more information
 All text above, and the splash screen below must be included in any redistribution
 *********************************************************************/

#ifdef __AVR__
#include <avr/pgmspace.h>
#elif defined(ESP8266)
#include <pgmspace.h>
#else
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif

#if !defined(ARDUINO_ARCH_SAM) && !defined(ARDUINO_ARCH_SAMD) && !defined(ESP8266) && !defined(ARDUINO_ARCH_STM32F2)
// #include <util/delay.h>
#endif

#include <stdlib.h>
#include <stdbool.h>

//#include <Wire.h>
//#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "oled_app.h"
#include "MK22F51212.h"
#include "spi_bb_driver.h"
//! filler defines
#define WIDTH 		SSD1306_LCDWIDTH			//!
#define HEIGHT 		SSD1306_LCDHEIGHT		//!
static uint8_t rotation = 0;
#define OUTPUT 1

//!  I think these need to be intialized.....
static int8_t _i2caddr, _vccstate, sid, sclk, dc, rst, cs;
void fastSPIwrite(uint8_t c);

bool hwSPI;
#ifdef PortReg
PortReg *mosiport, *clkport, *csport, *dcport;
PortMask mosipinmask, clkpinmask, cspinmask, dcpinmask;
#endif

inline void drawFastVLineInternal(int16_t x, int16_t y, int16_t h,
		uint16_t color) __attribute__((always_inline));
inline void drawFastHLineInternal(int16_t x, int16_t y, int16_t w,
		uint16_t color) __attribute__((always_inline));

// the memory buffer for the LCD

static uint8_t buffer[SSD1306_LCDHEIGHT * SSD1306_LCDWIDTH / 8] = { 0 };
static uint8_t buffer_save[SSD1306_LCDHEIGHT * SSD1306_LCDWIDTH / 8] = { 0 }; //allows to update screen and revert to old screen (updates/notifications).

#define ssd1306_swap(a, b) { int16_t t = a; a = b; b = t; }

void disp_save_buf() {
	uint32_t i;
	for (i = 0; i < (SSD1306_LCDHEIGHT * SSD1306_LCDWIDTH / 8); i++) {
		buffer_save[i] = buffer[i];
	}
}

//write old display buffer back to current
void disp_revert_buf() {
	uint32_t i;
	for (i = 0; i < (SSD1306_LCDHEIGHT * SSD1306_LCDWIDTH / 8); i++) {
		buffer[i] = buffer_save[i];
	}
}

// the most basic function, set a single pixel
void drawPixel(int16_t x, int16_t y, uint16_t color) {
	if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
		return;

	// check rotation, move pixel around if necessary
	switch (getRotation()) {
	case 1:
		ssd1306_swap(x, y)
		;
		x = WIDTH - x - 1;
		break;
	case 2:
		x = WIDTH - x - 1;
		y = HEIGHT - y - 1;
		break;
	case 3:
		ssd1306_swap(x, y)
		;
		y = HEIGHT - y - 1;
		break;
	}

	// x is which column
	switch (color) {
	case WHITE:
		buffer[x + (y / 8) * SSD1306_LCDWIDTH] |= (1 << (y & 7));
		break;
	case BLACK:
		buffer[x + (y / 8) * SSD1306_LCDWIDTH] &= ~(1 << (y & 7));
		break;
	case INVERSE:
		buffer[x + (y / 8) * SSD1306_LCDWIDTH] ^= (1 << (y & 7));
		break;
	}

}

/*//!
 Adafruit_SSD1306(int8_t SID, int8_t SCLK, int8_t DC, int8_t RST, int8_t CS) : Adafruit_GFX(SSD1306_LCDWIDTH, SSD1306_LCDHEIGHT) {
 cs = CS;
 rst = RST;
 dc = DC;
 sclk = SCLK;
 sid = SID;	//this must be mosi data  (serial input data?)
 hwSPI = false;
 }

 // constructor for hardware SPI - we indicate DataCommand, ChipSelect, Reset
 Adafruit_SSD1306(int8_t DC, int8_t RST, int8_t CS) : Adafruit_GFX(SSD1306_LCDWIDTH, SSD1306_LCDHEIGHT) {
 dc = DC;
 rst = RST;
 cs = CS;
 hwSPI = true;
 }

 // initializer for I2C - we only indicate the reset pin!
 Adafruit_SSD1306(int8_t reset) :
 Adafruit_GFX(SSD1306_LCDWIDTH, SSD1306_LCDHEIGHT) {
 sclk = dc = cs = sid = -1;
 rst = reset;
 }
 */

//reset =1 then use the reset pin.
//vccstate = selects power options for hte chip default: SSD1306_SWITCHCAPVCC, option:  SSD1306_EXTERNALVCC
//i2caddr = if using i2c
void ssd1306_begin(uint8_t vccstate, uint8_t i2caddr, bool reset) {
	_vccstate = vccstate; //SSD1306_SWITCHCAPVCC used by default in the spi example
	_i2caddr = i2caddr;

	rotation = getRotation();	//!

	// set pin directions
	if (sid != -1) {	//initialize spi
		//!pinMode(dc, OUTPUT);	//already setup
		//!pinMode(cs, OUTPUT);
#ifdef PortReg
		csport = portOutputRegister(digitalPinToPort(cs));
		cspinmask = digitalPinToBitMask(cs);
		dcport = portOutputRegister(digitalPinToPort(dc));
		dcpinmask = digitalPinToBitMask(dc);
#endif
		if (!hwSPI) {
			// set pins for software-SPI
			//!pinMode(sid, OUTPUT);		//must be mosi pin
			//!pinMode(sclk, OUTPUT); //already setup
#ifdef PortReg
			clkport = portOutputRegister(digitalPinToPort(sclk));
			clkpinmask = digitalPinToBitMask(sclk);
			mosiport = portOutputRegister(digitalPinToPort(sid));
			mosipinmask = digitalPinToBitMask(sid);
#endif
		}
		if (hwSPI) {
			//!SPI.begin();  //initialize spi
#ifdef SPI_HAS_TRANSACTION
			SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
#else
			//!SPI.setClockDivider(4);	//clock divider of hw spi
#endif
		}
	} else {	//otherwise intialize i2c
		// I2C Init
		//!Wire.begin();
#ifdef __SAM3X8E__
		// Force 400 KHz I2C, rawr! (Uses pins 20, 21 for SDA, SCL)
		TWI1->TWI_CWGR = 0;
		TWI1->TWI_CWGR = ((VARIANT_MCK / (2 * 400000)) - 4) * 0x101;
#endif
	}
	if ((reset) && (rst >= 0)) {	//use the reset pin
		// Setup reset pin direction (used by both SPI and I2C)
		//!pinMode(rst, OUTPUT);
		//!digitalWrite(rst, HIGH);
		PIN_RST_HIGH;
		// VDD (3.3V) goes high at start, lets just chill for a ms
		delay(1);
		// bring reset low
		//!digitalWrite(rst, LOW);
		PIN_RST_LOW;
		// wait 10ms
		delay(10);
		// bring out of reset
		//!digitalWrite(rst, HIGH);
		PIN_RST_HIGH;
		// turn on VCC (9V?)
	}

#if defined SSD1306_128_32
	// Init sequence for 128x32 OLED module
	ssd1306_command(SSD1306_DISPLAYOFF);// 0xAE
	ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);// 0xD5
	ssd1306_command(0x80);// the suggested ratio 0x80

	ssd1306_command(SSD1306_SETMULTIPLEX);// 0xA8
	ssd1306_command(0x1F);
	ssd1306_command(SSD1306_SETDISPLAYOFFSET);// 0xD3
	ssd1306_command(0x0);// no offset
	ssd1306_command(SSD1306_SETSTARTLINE | 0x0);// line #0
	ssd1306_command(SSD1306_CHARGEPUMP);// 0x8D
	if (vccstate == SSD1306_EXTERNALVCC)
	{	ssd1306_command(0x10);}
	else
	{	ssd1306_command(0x14);}
	ssd1306_command(SSD1306_MEMORYMODE);                    // 0x20
	ssd1306_command(0x00);// 0x0 act like ks0108
	ssd1306_command(SSD1306_SEGREMAP | 0x1);
	ssd1306_command(SSD1306_COMSCANDEC);
	ssd1306_command(SSD1306_SETCOMPINS);// 0xDA
	ssd1306_command(0x02);
	ssd1306_command(SSD1306_SETCONTRAST);// 0x81
	ssd1306_command(0x8F);
	ssd1306_command(SSD1306_SETPRECHARGE);// 0xd9
	if (vccstate == SSD1306_EXTERNALVCC)
	{	ssd1306_command(0x22);}
	else
	{	ssd1306_command(0xF1);}
	ssd1306_command(SSD1306_SETVCOMDETECT);                 // 0xDB
	ssd1306_command(0x40);
	ssd1306_command(SSD1306_DISPLAYALLON_RESUME);// 0xA4
	ssd1306_command(SSD1306_NORMALDISPLAY);// 0xA6
#endif

#if defined SSD1306_128_64
	// Init sequence for 128x64 OLED module
	ssd1306_command(SSD1306_DISPLAYOFF);                    // 0xAE
	ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
	ssd1306_command(0x80);                           // the suggested ratio 0x80
	ssd1306_command(SSD1306_SETMULTIPLEX);                  // 0xA8
	ssd1306_command(0x3F);
	ssd1306_command(SSD1306_SETDISPLAYOFFSET);              // 0xD3
	ssd1306_command(0x0);                                   // no offset
	ssd1306_command(SSD1306_SETSTARTLINE | 0x0);            // line #0
	ssd1306_command(SSD1306_CHARGEPUMP);                    // 0x8D
	if (vccstate == SSD1306_EXTERNALVCC) {
		ssd1306_command(0x10);
	} else {
		ssd1306_command(0x14);
	}
	ssd1306_command(SSD1306_MEMORYMODE);                    // 0x20
	ssd1306_command(0x00);                                // 0x0 act like ks0108
	ssd1306_command(SSD1306_SEGREMAP | 0x1);
	ssd1306_command(SSD1306_COMSCANDEC);
	ssd1306_command(SSD1306_SETCOMPINS);                    // 0xDA
	ssd1306_command(0x12);
	ssd1306_command(SSD1306_SETCONTRAST);                   // 0x81
	if (vccstate == SSD1306_EXTERNALVCC) {
		ssd1306_command(0x9F);
	} else {
		ssd1306_command(0xCF);
	}
	ssd1306_command(SSD1306_SETPRECHARGE);                  // 0xd9
	if (vccstate == SSD1306_EXTERNALVCC) {
		ssd1306_command(0x22);
	} else {
		ssd1306_command(0xF1);
	}
	ssd1306_command(SSD1306_SETVCOMDETECT);                 // 0xDB
	ssd1306_command(0x40);
	ssd1306_command(SSD1306_DISPLAYALLON_RESUME);           // 0xA4
	ssd1306_command(SSD1306_NORMALDISPLAY);                 // 0xA6
#endif

#if defined SSD1306_96_16
	// Init sequence for 96x16 OLED module
	ssd1306_command(SSD1306_DISPLAYOFF);// 0xAE
	ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);// 0xD5
	ssd1306_command(0x80);// the suggested ratio 0x80
	ssd1306_command(SSD1306_SETMULTIPLEX);// 0xA8
	ssd1306_command(0x0F);
	ssd1306_command(SSD1306_SETDISPLAYOFFSET);// 0xD3
	ssd1306_command(0x00);// no offset
	ssd1306_command(SSD1306_SETSTARTLINE | 0x0);// line #0
	ssd1306_command(SSD1306_CHARGEPUMP);// 0x8D
	if (vccstate == SSD1306_EXTERNALVCC)
	{	ssd1306_command(0x10);}
	else
	{	ssd1306_command(0x14);}
	ssd1306_command(SSD1306_MEMORYMODE);                    // 0x20
	ssd1306_command(0x00);// 0x0 act like ks0108
	ssd1306_command(SSD1306_SEGREMAP | 0x1);
	ssd1306_command(SSD1306_COMSCANDEC);
	ssd1306_command(SSD1306_SETCOMPINS);// 0xDA
	ssd1306_command(0x2);//ada x12
	ssd1306_command(SSD1306_SETCONTRAST);// 0x81
	if (vccstate == SSD1306_EXTERNALVCC)
	{	ssd1306_command(0x10);}
	else
	{	ssd1306_command(0xAF);}
	ssd1306_command(SSD1306_SETPRECHARGE);                  // 0xd9
	if (vccstate == SSD1306_EXTERNALVCC)
	{	ssd1306_command(0x22);}
	else
	{	ssd1306_command(0xF1);}
	ssd1306_command(SSD1306_SETVCOMDETECT);                 // 0xDB
	ssd1306_command(0x40);
	ssd1306_command(SSD1306_DISPLAYALLON_RESUME);// 0xA4
	ssd1306_command(SSD1306_NORMALDISPLAY);// 0xA6
#endif

	ssd1306_command(SSD1306_DISPLAYON);                 //--turn on oled panel
}

/*//!
 void invertDisplay(uint8_t i) {
 if (i) {
 ssd1306_command(SSD1306_INVERTDISPLAY);
 } else {
 ssd1306_command(SSD1306_NORMALDISPLAY);
 }
 }
 */

void ssd1306_command(uint8_t c) {
	if (sid != -1) {
		// SPI
#ifdef PortReg
		*csport |= cspinmask;
		*dcport &= ~dcpinmask;
		*csport &= ~cspinmask;
#else
		//!digitalWrite(cs, HIGH);
		PIN_CS_HIGH;
		//!digitalWrite(dc, LOW);
		PIN_C_D_LOW;  //THIS WAS WRONG! WAS HIGH
		//!digitalWrite(cs, LOW);
		PIN_CS_LOW;
#endif
		fastSPIwrite(c);
#ifdef PortReg
		*csport |= cspinmask;
#else
		//!digitalWrite(cs, HIGH);
		PIN_CS_HIGH;
#endif
	} else {
		// I2C
		uint8_t control = 0x00;   // Co = 0, D/C = 0
		//!Wire.beginTransmission(_i2caddr);
		//!Wire.write(control);
		//!Wire.write(c);
		//!Wire.endTransmission();
	}
}

// startscrollright
// Activate a right handed scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F) 
void startscrollright(uint8_t start, uint8_t stop) {
	ssd1306_command(SSD1306_RIGHT_HORIZONTAL_SCROLL);
	ssd1306_command(0X00);
	ssd1306_command(start);
	ssd1306_command(0X00);
	ssd1306_command(stop);
	ssd1306_command(0X00);
	ssd1306_command(0XFF);
	ssd1306_command(SSD1306_ACTIVATE_SCROLL);
}

// startscrollleft
// Activate a right handed scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F) 
void startscrollleft(uint8_t start, uint8_t stop) {
	ssd1306_command(SSD1306_LEFT_HORIZONTAL_SCROLL);
	ssd1306_command(0X00);
	ssd1306_command(start);
	ssd1306_command(0X00);
	ssd1306_command(stop);
	ssd1306_command(0X00);
	ssd1306_command(0XFF);
	ssd1306_command(SSD1306_ACTIVATE_SCROLL);
}

// startscrolldiagright
// Activate a diagonal scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F) 
void startscrolldiagright(uint8_t start, uint8_t stop) {
	ssd1306_command(SSD1306_SET_VERTICAL_SCROLL_AREA);
	ssd1306_command(0X00);
	ssd1306_command(SSD1306_LCDHEIGHT);
	ssd1306_command(SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL);
	ssd1306_command(0X00);
	ssd1306_command(start);
	ssd1306_command(0X00);
	ssd1306_command(stop);
	ssd1306_command(0X01);
	ssd1306_command(SSD1306_ACTIVATE_SCROLL);
}

// startscrolldiagleft
// Activate a diagonal scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F) 
void startscrolldiagleft(uint8_t start, uint8_t stop) {
	ssd1306_command(SSD1306_SET_VERTICAL_SCROLL_AREA);
	ssd1306_command(0X00);
	ssd1306_command(SSD1306_LCDHEIGHT);
	ssd1306_command(SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL);
	ssd1306_command(0X00);
	ssd1306_command(start);
	ssd1306_command(0X00);
	ssd1306_command(stop);
	ssd1306_command(0X01);
	ssd1306_command(SSD1306_ACTIVATE_SCROLL);
}

void stopscroll(void) {
	ssd1306_command(SSD1306_DEACTIVATE_SCROLL);
}

// Dim the display
// dim = true: display is dimmed
// dim = false: display is normal
void dim(bool dim) {
	uint8_t contrast;

	if (dim) {
		contrast = 0; // Dimmed display
	} else {
		if (_vccstate == SSD1306_EXTERNALVCC) {
			contrast = 0x9F;
		} else {
			contrast = 0xCF;
		}
	}
	// the range of contrast to too small to be really useful
	// it is useful to dim the display
	ssd1306_command(SSD1306_SETCONTRAST);
	ssd1306_command(contrast);
}

void ssd1306_data(uint8_t c) {
	if (sid != -1) {
		// SPI
#ifdef PortReg
		*csport |= cspinmask;
		*dcport |= dcpinmask;
		*csport &= ~cspinmask;
#else
		digitalWrite(cs, HIGH);
		digitalWrite(dc, HIGH);
		digitalWrite(cs, LOW);
#endif
		fastSPIwrite(c);
#ifdef PortReg
		*csport |= cspinmask;
#else
		digitalWrite(cs, HIGH);
#endif
	} else {
		// I2C
		uint8_t control = 0x40;   // Co = 0, D/C = 1
		//!Wire.beginTransmission(_i2caddr);
		//!WIRE_WRITE(control);
		//! WIRE_WRITE(c);
		//!Wire.endTransmission();
	}
}

void display(void) {
	ssd1306_command(SSD1306_COLUMNADDR);
	ssd1306_command(0);   // Column start address (0 = reset)
	ssd1306_command(SSD1306_LCDWIDTH - 1); // Column end address (127 = reset)

	ssd1306_command(SSD1306_PAGEADDR);
	ssd1306_command(0); // Page start address (0 = reset)
#if SSD1306_LCDHEIGHT == 64
	ssd1306_command(7); // Page end address
#endif
#if SSD1306_LCDHEIGHT == 32
	ssd1306_command(3); // Page end address
#endif
#if SSD1306_LCDHEIGHT == 16
	ssd1306_command(1); // Page end address
#endif

	if (sid != -1) {
		// SPI
#ifdef PortReg
		*csport |= cspinmask;
		*dcport |= dcpinmask;
		*csport &= ~cspinmask;
#else
		//!digitalWrite(cs, HIGH);
		PIN_CS_HIGH;
		//!digitalWrite(dc, HIGH);
		PIN_C_D_HIGH;
		//!digitalWrite(cs, LOW);
		PIN_CS_LOW;
#endif
		//I think this is updating the RAM of the display
		for (uint16_t i = 0; i < (SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT / 8);
				i++) {
			fastSPIwrite(buffer[i]);
			//ssd1306_data(buffer[i]);
		}
#ifdef PortReg
		*csport |= cspinmask;
#else
		//!digitalWrite(cs, HIGH);
		PIN_CS_HIGH;
#endif
	} else {
		// save I2C bitrate
#ifdef TWBR
		uint8_t twbrbackup = TWBR;
		TWBR = 12; // upgrade to 400KHz!
#endif

		//Serial.println(TWBR, DEC);
		//Serial.println(TWSR & 0x3, DEC);

		// I2C
		for (uint16_t i = 0; i < (SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT / 8);
				i++) {
			// send a bunch of data in one xmission
			//!Wire.beginTransmission(_i2caddr);
			WIRE_WRITE(0x40);
			for (uint8_t x = 0; x < 16; x++) {
				WIRE_WRITE(buffer[i]);
				i++;
			}
			i--;
			//!Wire.endTransmission();
		}
#ifdef TWBR
		TWBR = twbrbackup;
#endif
	}
}

// clear everything
void clearDisplay(void) {
	memset(buffer, 0, (SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT / 8));//! i THINK this is crashing things...
}

inline void fastSPIwrite(uint8_t d) {

	if (hwSPI) {
		//!(void) SPI.transfer(d);
	} else {
		for (uint8_t bit = 0x80; bit; bit >>= 1) {
#ifdef PortReg
			*clkport &= ~clkpinmask;
			if(d & bit) *mosiport |= mosipinmask;
			else *mosiport &= ~mosipinmask;
			*clkport |= clkpinmask;
#else
			//!digitalWrite(sclk, LOW);
			PIN_SCK_LOW;
			if (d & bit)
				//!digitalWrite(sid, HIGH);
				PIN_MOSI_HIGH;
			else
				//!digitalWrite(sid, LOW);
				PIN_MOSI_LOW;
			//!digitalWrite(sclk, HIGH);
			PIN_SCK_HIGH;
#endif
		}
	}
}

void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
	bool bSwap = false;
	switch (rotation) {
	case 0:
		// 0 degree rotation, do nothing
		break;
	case 1:
		// 90 degree rotation, swap x & y for rotation, then invert x
		bSwap = true;
		ssd1306_swap(x, y)
		;
		x = WIDTH - x - 1;
		break;
	case 2:
		// 180 degree rotation, invert x and y - then shift y around for height.
		x = WIDTH - x - 1;
		y = HEIGHT - y - 1;
		x -= (w - 1);
		break;
	case 3:
		// 270 degree rotation, swap x & y for rotation, then invert y  and adjust y for w (not to become h)
		bSwap = true;
		ssd1306_swap(x, y)
		;
		y = HEIGHT - y - 1;
		y -= (w - 1);
		break;
	}

	if (bSwap) {
		drawFastVLineInternal(x, y, w, color);
	} else {
		drawFastHLineInternal(x, y, w, color);
	}
}

void drawFastHLineInternal(int16_t x, int16_t y, int16_t w, uint16_t color) {
	// Do bounds/limit checks
	if (y < 0 || y >= HEIGHT) {
		return;
	}

	// make sure we don't try to draw below 0
	if (x < 0) {
		w += x;
		x = 0;
	}

	// make sure we don't go off the edge of the display
	if ((x + w) > WIDTH) {
		w = (WIDTH - x);
	}

	// if our width is now negative, punt
	if (w <= 0) {
		return;
	}

	// set up the pointer for  movement through the buffer
	register uint8_t *pBuf = buffer;
	// adjust the buffer pointer for the current row
	pBuf += ((y / 8) * SSD1306_LCDWIDTH);
	// and offset x columns in
	pBuf += x;

	register uint8_t mask = 1 << (y & 7);

	switch (color) {
	case WHITE:
		while (w--) {
			*pBuf++ |= mask;
		}
		;
		break;
	case BLACK:
		mask = ~mask;
		while (w--) {
			*pBuf++ &= mask;
		}
		;
		break;
	case INVERSE:
		while (w--) {
			*pBuf++ ^= mask;
		}
		;
		break;
	}
}

void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
	bool bSwap = false;
	switch (rotation) {
	case 0:
		break;
	case 1:
		// 90 degree rotation, swap x & y for rotation, then invert x and adjust x for h (now to become w)
		bSwap = true;
		ssd1306_swap(x, y)
		;
		x = WIDTH - x - 1;
		x -= (h - 1);
		break;
	case 2:
		// 180 degree rotation, invert x and y - then shift y around for height.
		x = WIDTH - x - 1;
		y = HEIGHT - y - 1;
		y -= (h - 1);
		break;
	case 3:
		// 270 degree rotation, swap x & y for rotation, then invert y
		bSwap = true;
		ssd1306_swap(x, y)
		;
		y = HEIGHT - y - 1;
		break;
	}

	if (bSwap) {
		drawFastHLineInternal(x, y, h, color);
	} else {
		drawFastVLineInternal(x, y, h, color);
	}
}

void drawFastVLineInternal(int16_t x, int16_t __y, int16_t __h, uint16_t color) {

	// do nothing if we're off the left or right side of the screen
	if (x < 0 || x >= WIDTH) {
		return;
	}

	// make sure we don't try to draw below 0
	if (__y < 0) {
		// __y is negative, this will subtract enough from __h to account for __y being 0
		__h += __y;
		__y = 0;

	}

	// make sure we don't go past the height of the display
	if ((__y + __h) > HEIGHT) {
		__h = (HEIGHT - __y);
	}

	// if our height is now negative, punt
	if (__h <= 0) {
		return;
	}

	// this display doesn't need ints for coordinates, use local byte registers for faster juggling
	register uint8_t y = __y;
	register uint8_t h = __h;

	// set up the pointer for fast movement through the buffer
	register uint8_t *pBuf = buffer;
	// adjust the buffer pointer for the current row
	pBuf += ((y / 8) * SSD1306_LCDWIDTH);
	// and offset x columns in
	pBuf += x;

	// do the first partial byte, if necessary - this requires some masking
	register uint8_t mod = (y & 7);
	if (mod) {
		// mask off the high n bits we want to set
		mod = 8 - mod;

		// note - lookup table results in a nearly 10% performance improvement in fill* functions
		// register uint8_t mask = ~(0xFF >> (mod));
		static uint8_t premask[8] = { 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC,
				0xFE };
		register uint8_t mask = premask[mod];

		// adjust the mask if we're not going to reach the end of this byte
		if (h < mod) {
			mask &= (0XFF >> (mod - h));
		}

		switch (color) {
		case WHITE:
			*pBuf |= mask;
			break;
		case BLACK:
			*pBuf &= ~mask;
			break;
		case INVERSE:
			*pBuf ^= mask;
			break;
		}

		// fast exit if we're done here!
		if (h < mod) {
			return;
		}

		h -= mod;

		pBuf += SSD1306_LCDWIDTH;
	}

	// write solid bytes while we can - effectively doing 8 rows at a time
	if (h >= 8) {
		if (color == INVERSE) { // separate copy of the code so we don't impact performance of the black/white write version with an extra comparison per loop
			do {
				*pBuf = ~(*pBuf);

				// adjust the buffer forward 8 rows worth of data
				pBuf += SSD1306_LCDWIDTH;

				// adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
				h -= 8;
			} while (h >= 8);
		} else {
			// store a local value to work with
			register uint8_t val = (color == WHITE) ? 255 : 0;

			do {
				// write our value in
				*pBuf = val;

				// adjust the buffer forward 8 rows worth of data
				pBuf += SSD1306_LCDWIDTH;

				// adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
				h -= 8;
			} while (h >= 8);
		}
	}

	// now do the final partial byte, if necessary
	if (h) {
		mod = h & 7;
		// this time we want to mask the low bits of the byte, vs the high bits we did above
		// register uint8_t mask = (1 << mod) - 1;
		// note - lookup table results in a nearly 10% performance improvement in fill* functions
		static uint8_t postmask[8] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F,
				0x7F };
		register uint8_t mask = postmask[mod];
		switch (color) {
		case WHITE:
			*pBuf |= mask;
			break;
		case BLACK:
			*pBuf &= ~mask;
			break;
		case INVERSE:
			*pBuf ^= mask;
			break;
		}
	}
}
