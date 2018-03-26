/*
 * oled_app.c
 *
 *  Created on: Dec 15, 2015
 *      Author: MJ
 */

//#include <SPI.h>
//#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdlib.h>
#include "bosclone_graphic.h"
#include "printf.h"


void testfillroundrect(void);
void testfillroundrect(void) ;
void init_disp_update() ;
void reset_display() ;
void print_clone_status(uint8_t * pArray) ;
void printRecivedCardApp(uint8_t *pArray) ;
void printRecivedCardWiegand(uint8_t *pArray) ;
void LCD_Init() ;
void bosclone_display(void) ;
void testPrintf() ;
void testPrintFast() ;
void testWrite() ;
void oled_test() ;
void testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h) ;
void testdrawchar(void) ;
void testdrawcircle(void) ;
void testfillrect(void) ;
void testdrawtriangle(void) ;
void testscrolltext(void) ;
void testfilltriangle(void);
void testdrawroundrect(void);
void testfillroundrect(void);
void testdrawrect(void);
void testdrawline();


#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

// If using software SPI (the default case):
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13
//!Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);	//sets up hte pins to be used for comm

/* Uncomment this block to use hardware SPI
 #define OLED_DC     6
 #define OLED_CS     7
 #define OLED_RESET  8
 Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);
 */

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16
//!static const unsigned char PROGMEM logo16_glcd_bmp[] =
static const unsigned char logo16_glcd_bmp[] = { 0b00000000, 0b11000000,
		0b00000001, 0b11000000, 0b00000001, 0b11000000, 0b00000011, 0b11100000,
		0b11110011, 0b11100000, 0b11111110, 0b11111000, 0b01111110, 0b11111111,
		0b00110011, 0b10011111, 0b00011111, 0b11111100, 0b00001101, 0b01110000,
		0b00011011, 0b10100000, 0b00111111, 0b11100000, 0b00111111, 0b11110000,
		0b01111100, 0b11110000, 0b01110000, 0b01110000, 0b00000000, 0b00110000 };

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


void testdrawline();


//TEST THE DISPLAY BACKUP BUFFER MECHANISM
void test_disp_update() {

	disp_save_buf(); //backs up current display
	//write new data

	clearDisplay();
	display();
	setCursor(0, 0);
	testdrawchar();
	display();
	DelayMS(4000);
	disp_revert_buf();
	display();

}

//SAVE THE CURRENT LCD BUFFER
void init_disp_update() {
	if (!update_being_displayed()) {//only save the current display buffer if display not currently  on.
		disp_save_buf();
	}
	reset_display();
	TimerStart(dsp_tmr1, DEF_DSP_UPDATE_TIME);	//user default update time
}

//RESET THE LCD
void reset_display() {
	clearDisplay();
	setTextSize(1);
	setCursor(0, 0);
	display();
}




//PRINTS THE CLONE STATUS CHANGE TO LCD
void print_clone_status(uint8_t * pArray) {
	//setup display update
	init_disp_update();
	if (*pArray != 0) {	//print custom string if valid
		printf("%s\n", pArray);
	}
	printf("Clone status Changed\n");
	printf("Clone Status: %u\n", clone_status);
	display();
}


//PRINTS TEH CARD DATA TO LCD
//only print the recieved hex value from the app.
void printRecivedCardApp(uint8_t *pArray) {
	uint32_t hi, lo;
	uint32_t card, facility;

	//setup display update
	init_disp_update();
	TimerStart(dsp_tmr1, CARD_ID_UPDATE_TIME);//set custom time to timeout, not default time used

	hi = get_card_hi_app();
	lo = get_card_lo_app();

	if (*pArray != 0) {	//print custom string if valid
		printf("%s\n", pArray);
	}
	printf("App Recieved ID:\n");
	setTextSize(2);
	if (hi >> 4 == 0) {
		printf("0");
	}
	printf("%x", hi);
	if (lo >> 28 == 0) {//if high  nibble = zero must print explicitly a the 0, otherwise does not print.
		printf("0");
	}
	printf("%x\n", lo);	//if lo has 0 as hi 4 nibbles it will not print, must combine
	setTextSize(2);
	printf("Cloned!\n");
	display();
}

void printRecivedCardWiegand(uint8_t *pArray) {
	uint32_t hex1, hex2;
	uint32_t card, facility;

	//setup display update
	init_disp_update();
	TimerStart(dsp_tmr1, CARD_ID_UPDATE_TIME);//set custom time to timeout, not default time used

	card = get_facility_code();
	facility = get_card_code();
	hex1 = get_card_hex1();
	hex2 = get_card_hex2();
	//hex = cardChunk2<<32
	clearDisplay();
	setCursor(0, 0);
	setTextSize(1);
	if (*pArray != 0) {	//print custom string if valid
		printf("%s", pArray);
	}
	printf("Clone status: %u\n", clone_status);
	setTextSize(2);
	printf("F: %u\n", facility);
	printf("C: %u\n", card);
	setTextSize(1);
	printf("Hex format:\n");
	setTextSize(2);
	if (hex1 >> 12 == 0) {
		printf("0");
	}
	printf("%x", hex1);
	if (hex2 >> 20 == 0) {//if high  nibble = zero must print explicitly a the 0, otherwise does not print.
		printf("0");
	}
	printf("%x\n", hex2);//if lo has 0 as hi 4 nibbles it will not print, must combine
	display();

}

//INIT THE LCD
void LCD_Init() {
	Adafruit_GFX_Init();	//need to call the gfx init function
	ssd1306_begin(SSD1306_SWITCHCAPVCC, 0, 1);	//power mode, x, reset
	setTextSize(1);
	setTextColor(WHITE);
	setCursor(0, 0);
	clearDisplay();	//clear the buffer (adafruit by default)
	display();		//update the screen contents
}

//DISPLAY THE BOSCLONE LOGO
void bosclone_display(void) {
	drawBitmap(0, 0, boscloner_logo_128_64, 128, 64, WHITE);
	display();
}



//PRINTF TEST FUNCTION
void testPrintf() {
	setTextColor(WHITE);
	setCursor(0, 0);
	clearDisplay();

	setTextSize(1);
	printf("boscloner Version 0.1");
	//setCursor(0, 2);	//this screws it up
	printf("HEX: %X\n", 123456789);
	setTextSize(2);	//at size 2 it only fits up to "d" on the screen=10chars
	printf("F:%u\n", 113);
	printf("C:%u\n", 6223);

	//setCursor(0, 2);	//this screws it up
	printf("num: %u\n", 12345);

	display();
	delay(2000);
	clearDisplay();

}

//TEST FUNCTION
void testPrintFast() {
	uint16_t i=0;
	setTextColor(WHITE);
	setCursor(0, 0);
	clearDisplay();

	setTextSize(1);
	printf("boscloner V0.1\r\n");
	//setCursor(0, 2);	//this screws it up
	printf("HEX: %X\n", i++);
	setTextSize(2);	//at size 2 it only fits up to "d" on the screen=10chars
	printf("F: %u\n", i++);
	printf("C: %u\n", i++);
	//setCursor(0, 2);	//this screws it up
	printf("num: %u\n", i++);

	display();
	delay(2000);
	clearDisplay();

}

//TEST FUNCTION
void testWrite() {
	setTextColor(WHITE);
	setCursor(0, 0);
	clearDisplay();

	//size 1
	setTextSize(1);
	for (uint8_t i = 49; i < 86; i++) {
		write(i);
	}
	setTextSize(2);
	for (uint8_t i = 49; i < 86; i++) {
		write(i);
	}
	setTextSize(3);
	for (uint8_t i = 49; i < 55; i++) {
		write(i);
	}
	display();
	delay(2000);
}

//GRAPHIC LOOP TEST FEATURES
void oled_test() {

	drawBitmap(0, 0, boscloner_logo_128_64, 128, 64, WHITE);
	display();
	delay(2000);
	clearDisplay();

	testdrawchar();
	display();
	delay(2000);
	clearDisplay();

	// draw a single pixel
	drawPixel(10, 10, WHITE);
	// Show the display buffer on the hardware.
	// NOTE: You _must_ call display after making any drawing commands
	// to make them visible on the display hardware!
	display();
	delay(2000);
	clearDisplay();

	// draw many lines
	testdrawline();
	display();
	delay(2000);
	clearDisplay();

	// draw rectangles
	testdrawrect();
	display();
	delay(2000);
	clearDisplay();

	// draw multiple rectangles
	testfillrect();
	display();
	delay(2000);
	clearDisplay();

	// draw mulitple circles
	testdrawcircle();
	display();
	delay(2000);
	clearDisplay();

	// draw a white circle, 10 pixel radius
	fillCircle(width() / 2, height() / 2, 10, WHITE);
	display();
	delay(2000);
	clearDisplay();

	testdrawroundrect();
	delay(2000);
	clearDisplay();

	testfillroundrect();
	delay(2000);
	clearDisplay();

	testdrawtriangle();
	delay(2000);
	clearDisplay();

	testfilltriangle();
	delay(2000);
	clearDisplay();

	// miniature bitmap display
	clearDisplay();
	drawBitmap(30, 16, logo16_glcd_bmp, 16, 16, 1);
	display();
	delay(2000);
	clearDisplay();

	drawBitmap(0, 0, boscloner_logo_128_64, 128, 64, WHITE);
	display();

}

void testdrawbitmap(const uint8_t *bitmap, uint8_t w, uint8_t h) {
	uint8_t icons[NUMFLAKES][3];

	// initialize
	for (uint8_t f = 0; f < NUMFLAKES; f++) {
		//icons[f][XPOS] = rand(width());
		//!srand(width());
		//!icons[f][XPOS] = rand();	//! was crashing program.
		icons[f][YPOS] = 0;
		//!icons[f][DELTAY] = rand(5) + 1;
		//!srand(5);
		//!icons[f][DELTAY] = rand() + 1;

		//Serial.print("x: ");
		//Serial.print(icons[f][XPOS], DEC);
		//Serial.print(" y: ");
		//Serial.print(icons[f][YPOS], DEC);
		//Serial.print(" dy: ");
		//Serial.println(icons[f][DELTAY], DEC);
	}

	//!while (1) {
	for (uint8_t i = 0; i < 16; i++) {//don't sit here for ever, will loop the demo display
		// draw each icon
		for (uint8_t f = 0; f < NUMFLAKES; f++) {
			drawBitmap(icons[f][XPOS], icons[f][YPOS], logo16_glcd_bmp, w, h,
			WHITE);
		}
		display();
		delay(200);

		// then erase it + move it
		for (uint8_t f = 0; f < NUMFLAKES; f++) {
			drawBitmap(icons[f][XPOS], icons[f][YPOS], logo16_glcd_bmp, w, h,
			BLACK);
			// move it
			icons[f][YPOS] += icons[f][DELTAY];
			// if its gone, reinit
			if (icons[f][YPOS] > height()) {
				//icons[f][XPOS] = random(width());
				//!icons[f][XPOS] = rand();
				icons[f][YPOS] = 0;
				//icons[f][DELTAY] = random(5) + 1;
				//!icons[f][DELTAY] = rand() + 1;
			}
		}
	}
}

void testdrawchar(void) {
	setTextSize(1);
	setTextColor(WHITE);
	setCursor(0, 0);

	for (uint8_t i = 0; i < 168; i++) {
		if (i == '\n')
			continue;
		write(i);
		if ((i > 0) && (i % 21 == 0))
			;
		//!println();
	}
	display();
}

void testdrawcircle(void) {
	for (int16_t i = 0; i < height(); i += 2) {
		drawCircle(width() / 2, height() / 2, i, WHITE);
		display();
	}
}

void testfillrect(void) {
	uint8_t color = 1;
	for (int16_t i = 0; i < height() / 2; i += 3) {
		// alternate colors
		fillRect(i, i, width() - i * 2, height() - i * 2, color % 2);
		display();
		color++;
	}
}

void testdrawtriangle(void) {
	for (int16_t i = 0; i < min(width(),height()) / 2; i += 5) {
		drawTriangle(width() / 2, height() / 2 - i, width() / 2 - i,
				height() / 2 + i, width() / 2 + i, height() / 2 + i, WHITE);
		display();
	}
}

void testfilltriangle(void) {
	uint8_t color = WHITE;
	for (int16_t i = min(width(),height()) / 2; i > 0; i -= 5) {
		fillTriangle(width() / 2, height() / 2 - i, width() / 2 - i,
				height() / 2 + i, width() / 2 + i, height() / 2 + i, WHITE);
		if (color == WHITE)
			color = BLACK;
		else
			color = WHITE;
		display();
	}
}

void testdrawroundrect(void) {
	for (int16_t i = 0; i < height() / 2 - 2; i += 2) {
		drawRoundRect(i, i, width() - 2 * i, height() - 2 * i, height() / 4,
		WHITE);
		display();
	}
}

void testfillroundrect(void) {
	uint8_t color = WHITE;
	for (int16_t i = 0; i < height() / 2 - 2; i += 2) {
		fillRoundRect(i, i, width() - 2 * i, height() - 2 * i, height() / 4,
				color);
		if (color == WHITE)
			color = BLACK;
		else
			color = WHITE;
		display();
	}
}

void testdrawrect(void) {
	for (int16_t i = 0; i < height() / 2; i += 2) {
		drawRect(i, i, width() - 2 * i, height() - 2 * i, WHITE);
		display();
	}
}

void testdrawline() {
	for (int16_t i = 0; i < width(); i += 4) {
		drawLine(0, 0, i, height() - 1, WHITE);
		display();
	}
	for (int16_t i = 0; i < height(); i += 4) {
		drawLine(0, 0, width() - 1, i, WHITE);
		display();
	}
	delay(250);

	clearDisplay();
	for (int16_t i = 0; i < width(); i += 4) {
		drawLine(0, height() - 1, i, 0, WHITE);
		display();
	}
	for (int16_t i = height() - 1; i >= 0; i -= 4) {
		drawLine(0, height() - 1, width() - 1, i, WHITE);
		display();
	}
	delay(250);

	clearDisplay();
	for (int16_t i = width() - 1; i >= 0; i -= 4) {
		drawLine(width() - 1, height() - 1, i, 0, WHITE);
		display();
	}
	for (int16_t i = height() - 1; i >= 0; i -= 4) {
		drawLine(width() - 1, height() - 1, 0, i, WHITE);
		display();
	}
	delay(250);

	clearDisplay();
	for (int16_t i = 0; i < height(); i += 4) {
		drawLine(width() - 1, 0, 0, i, WHITE);
		display();
	}
	for (int16_t i = 0; i < width(); i += 4) {
		drawLine(width() - 1, 0, i, height() - 1, WHITE);
		display();
	}
	delay(250);
}

void testscrolltext(void) {
	setTextSize(2);
	setTextColor(WHITE);
	setCursor(10, 0);
	clearDisplay();
	//!println("scroll");
	display();

	startscrollright(0x00, 0x0F);
	delay(2000);
	stopscroll();
	delay(1000);
	startscrollleft(0x00, 0x0F);
	delay(2000);
	stopscroll();
	delay(1000);
	startscrolldiagright(0x00, 0x07);
	delay(2000);
	startscrolldiagleft(0x00, 0x07);
	delay(2000);
	stopscroll();
}

void digitalWrite(uint8_t pin, uint8_t dir) {
}

void pinMode(uint8_t pin, uint8_t dir) {

}

void println(uint8_t * array) {

}

