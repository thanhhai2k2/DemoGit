/*
 * eeprom.c
 *
 *  Created on: 22 sty 2022
 *      Author: pawel
 */

#include "stm32f401xe_i2c.h"
#include "delay.h"
#include "eeprom.h"

I2c_Handle_t *p_eeprom_i2c;

void Eeprom_SendData(uint8_t first_page, uint8_t *data_buffer, uint32_t data_size)
{
	uint8_t write_pages = data_size / EEPROM_PAGE_SIZE;

	if (write_pages > EEPROM_NO_PAGES)
	{
		//error
		return;
	}

	// save data in eeprom pages
	for (uint8_t transmit_count = 0; transmit_count < write_pages; transmit_count++)
	{
		I2c_Transmit(p_eeprom_i2c, (EEPROM_CONTORLBYTE << 1), first_page + (transmit_count * EEPROM_PAGE_SIZE),
				&(data_buffer[transmit_count * EEPROM_PAGE_SIZE]),
				EEPROM_PAGE_SIZE);

		// need a delay between writing or a repeat sending
		delay(40000);
	}


}

void Eeprom_ReadData(uint8_t first_page, uint8_t *data_buffer, uint32_t data_size)
{

	// transmit info that we are reading
	I2c_Transmit(p_eeprom_i2c, (EEPROM_CONTORLBYTE << 1),first_page,0,0);
	I2c_Recieve(p_eeprom_i2c, (EEPROM_CONTORLBYTE << 1), data_buffer, data_size);

}

void Eeprom_Init(I2c_Handle_t *p_i2c)
{
	p_eeprom_i2c = p_i2c;
}
