/*
 * eeprom.h
 *
 *  Created on: 22 sty 2022
 *      Author: pawel
 */

#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_

#define EEPROM_CONTORLBYTE			0x50
#define EEPROM_PAGE_SIZE			8U
#define EEPROM_NO_PAGES				128U
#define EEPROM_PAGE_CONFIG			EEPROM_PAGE_SIZE * 12U
#define EEPROM_SCORES_SIZE			96U


void Eeprom_SendData(uint8_t first_page, uint8_t *data_buffer, uint32_t data_size);
void Eeprom_ReadData(uint8_t first_page, uint8_t *data_buffer, uint32_t data_size);
void Eeprom_Init(I2c_Handle_t *p_i2c);

#endif /* INC_EEPROM_H_ */
