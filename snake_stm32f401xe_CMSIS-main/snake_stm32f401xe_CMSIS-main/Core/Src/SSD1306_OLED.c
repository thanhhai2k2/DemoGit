/*
 * SSD1306_OLED.c
 *
 *  Created on: Sep 13, 2021
 *      Author: pawel
 */


#include <stm32f401xe_i2c.h>
#include "SSD1306_OLED.h"
#include <string.h>

I2c_Handle_t *p_oled_i2c;

static uint8_t buffer [SSD1306_BUFFERSIZE];


void SSD1306_Command(uint8_t Command)
{
	I2c_Transmit(p_oled_i2c, (SSD1306_ADDRESS << 1), SSD1306_REG_COMMAND, &Command, 1);

}

void SSD1306_Data(uint8_t *Data, uint16_t Size)
{
	I2c_Transmit(p_oled_i2c, (SSD1306_ADDRESS << 1), SSD1306_REG_DATA, Data, Size);

}

void SSD1306_Clear(uint8_t Color)
{
	switch(Color)
	{
	case BLACK:
		memset(buffer,0x00,SSD1306_BUFFERSIZE);
		break;

	case YELLOWBLUE:
		memset(buffer,0xFF,SSD1306_BUFFERSIZE);
		break;
	}
}

void SSD1306_DrawPixel(int16_t x, int16_t y, uint8_t color) {

	if ((x < 0) || (x >= SSD1306_LCDWIDTH) || (y < 0) || (y >= SSD1306_LCDWIDTH))
		return;
    switch (color) {
    case SSD1306_COLOR_YELLOWBLUE:
    //buffer[x + (y / 8) * SSD1306_LCDWIDTH] |= (1 << (y & 7)); // byte 0 on the left side near GND
    buffer[(y/8) * SSD1306_LCDWIDTH + (SSD1306_LCDWIDTH - 1 - x)] |= (1 << (y & 7)); //  byte 0 on the right side near SDA

      break;
    case SSD1306_COLOR_BLACK:
      //buffer[x + (y / 8) * SSD1306_LCDWIDTH] &= ~(1 << (y & 7));
    	buffer[(y/8) * SSD1306_LCDWIDTH + (SSD1306_LCDWIDTH - 1 - x)] &= ~(1 << (y & 7));
      break;
    case SSD1306_INVERSE:
      //buffer[x + (y / 8) * SSD1306_LCDWIDTH] ^= (1 << (y & 7));
    	buffer[(y/8) * SSD1306_LCDWIDTH + (SSD1306_LCDWIDTH - 1 - x)] ^= (1 << (y & 7));
      break;
    }
  }


void SSD1306_Display(void)
{

	  SSD1306_Command(SSD1306_PAGEADDR);
	  SSD1306_Command(0);				// Page start address
	  SSD1306_Command(0xFF);				// Page end (not really, but works here)
	  SSD1306_Command(SSD1306_COLUMNADDR);
	  SSD1306_Command(0);				// Column start address
	  SSD1306_Command(SSD1306_LCDWIDTH - 1); 		// Column end address

	  SSD1306_Data(buffer,SSD1306_BUFFERSIZE);


}
void SSD1306_Init(I2c_Handle_t *i2c)
{
	p_oled_i2c = i2c;

	SSD1306_Command(SSD1306_DISPLAYOFF);
	SSD1306_Command(SSD1306_SETDISPLAYCLOCKDIV);
	SSD1306_Command(0x80);
	SSD1306_Command(SSD1306_SETMULTIPLEX);
	SSD1306_Command(SSD1306_LCDHEIGHT - 1);

	SSD1306_Command(SSD1306_SETDISPLAYOFFSET);
	SSD1306_Command(0x00);
	SSD1306_Command(0x80);
	SSD1306_Command(SSD1306_CHARGEPUMP);
	SSD1306_Command(0x14);

	SSD1306_Command(SSD1306_MEMORYMODE);
	SSD1306_Command(0x00);
	SSD1306_Command(SSD1306_SEGREMAP);
	SSD1306_Command(SSD1306_COMSCANDEC);

	SSD1306_Command(SSD1306_SETCOMPINS);
	SSD1306_Command(0x12);
	SSD1306_Command(SSD1306_SETCONTRAST);
	SSD1306_Command(0xFF);

	SSD1306_Command(SSD1306_SETPRECHARGE); // 0xd9
	SSD1306_Command(0xF1);

	SSD1306_Command(SSD1306_SETVCOMDETECT);
	SSD1306_Command(0x40);
	SSD1306_Command(SSD1306_DISPLAYALLON_RESUME);
	SSD1306_Command(SSD1306_NORMALDISPLAY);
	SSD1306_Command(SSD1306_DEACTIVATE_SCROLL);
	SSD1306_Command(SSD1306_DISPLAYON);


}
void SSD1306_testrun(void)
{
	buffer[127] = 0xFF;
	SSD1306_Display();
}
