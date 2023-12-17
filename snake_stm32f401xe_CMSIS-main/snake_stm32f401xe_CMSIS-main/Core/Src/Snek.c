/*
 * Snek.c
 *
 *  Created on: 17 sty 2022
 *      Author: ROJEK
 */

#include <stm32f401xe_gpio.h>
#include "SSD1306_OLED.h"
#include "eeprom.h"
#include "GFX_BW.h"
#include "bitmap.h"
#include "string.h"
#include "delay.h"
#include "stdint.h"
#include "stdlib.h"
#include "stdio.h"
#include "Snek.h"

//  snek_ui - functions used to draw user interface
static void snek_ui_modify_speed(snek_game_t *p_snek_game)
{
	p_snek_game->p_game_tick_tim = TIM9;
	p_snek_game->p_game_tick_tim->ARR = 9999 - (p_snek_game->game_config.speed * 1000);
	p_snek_game->p_game_tick_tim->CNT = 0;
}

static void snek_ui_draw_mainmenu_button(uint8_t *p_text, uint8_t pos, uint8_t select)
{
	// lcd width - char size - spaces between chars
	uint8_t string_start_pos = ((SSD1306_LCDWIDTH - (strlen((char*) p_text) * 5) - strlen((char*) p_text))) / 2;

	GFX_DrawFillRectangle(0, pos * SNEK_UI_BUTTON_OFFSET, SNEK_UI_BUTTON_WIDTH, SNEK_UI_BUTTON_HEIGHT, BLACK);
	//draw selected button
	if (select)
	{
		GFX_DrawFillRectangle(0, pos * SNEK_UI_BUTTON_OFFSET, SNEK_UI_BUTTON_WIDTH, SNEK_UI_BUTTON_HEIGHT, YELLOWBLUE);
		GFX_DrawString(string_start_pos, (pos * SNEK_UI_BUTTON_OFFSET) + SNEK_UI_CHAR_OFFSET, (char*) p_text, BLACK,
		YELLOWBLUE);
	}
	else // draw unselected button
	{
		GFX_DrawRectangle(0, pos * SNEK_UI_BUTTON_OFFSET, SNEK_UI_BUTTON_WIDTH, SNEK_UI_BUTTON_HEIGHT, YELLOWBLUE);
		GFX_DrawString(string_start_pos, (pos * SNEK_UI_BUTTON_OFFSET) + SNEK_UI_CHAR_OFFSET, (char*) p_text, YELLOWBLUE,
		BLACK);
	}

	return;
}

static void snek_ui_draw_ok_button(uint8_t highlight)
{
	if (!highlight)
	{
		// button highlight off
		GFX_DrawFillRectangle(SNEK_UI_OK_BUTTON_POS_X, SNEK_UI_OK_BUTTON_POS_Y, SNEK_UI_OK_BUTTON_WIDTH, SNEK_UI_OK_BUTTON_HEIGHT, BLACK);
		GFX_DrawRectangle(SNEK_UI_OK_BUTTON_POS_X, SNEK_UI_OK_BUTTON_POS_Y, SNEK_UI_OK_BUTTON_WIDTH, SNEK_UI_OK_BUTTON_HEIGHT, YELLOWBLUE);
		GFX_DrawString(SNEK_UI_OK_BUTTON_POS_X + 4, SNEK_UI_OK_BUTTON_POS_Y + 1, "OK", YELLOWBLUE, BLACK);
	}
	else
	{
		// highlight ok button
		GFX_DrawFillRectangle(SNEK_UI_OK_BUTTON_POS_X, SNEK_UI_OK_BUTTON_POS_Y, SNEK_UI_OK_BUTTON_WIDTH, SNEK_UI_OK_BUTTON_HEIGHT, YELLOWBLUE);
		GFX_DrawRectangle(SNEK_UI_OK_BUTTON_POS_X, SNEK_UI_OK_BUTTON_POS_Y, SNEK_UI_OK_BUTTON_WIDTH, SNEK_UI_OK_BUTTON_HEIGHT, BLACK);
		GFX_DrawString(SNEK_UI_OK_BUTTON_POS_X + 4, SNEK_UI_OK_BUTTON_POS_Y + 1, "OK", BLACK, YELLOWBLUE);
	}
}

static void snek_ui_updatescore(snek_game_t *p_snek_game)
{
	uint8_t temp_msg[8];
	sprintf((char*) temp_msg, "%d", (p_snek_game->snek_lenght - 3));
	GFX_DrawString(102, 0 + SNEK_UI_CHAR_OFFSET, (char*) temp_msg, YELLOWBLUE, BLACK);
}

static void snek_ui_gameover_animation(void)
{
	for (uint8_t count = 0; count < SSD1306_LCDWIDTH; count++)
	{
		GFX_DrawFillRectangle((count % 16) * 8, (count / 16) * 8, SNEK_UI_NODE_WIDTH, SNEK_UI_NODE_HEIGHT, YELLOWBLUE);

		SSD1306_Display();
	}
}

static void snek_eeprom_erase(void)
{
	uint8_t temp_data[EEPROM_SIZE] = {'0'};
	uint8_t dummy[8] =
			{ 'd', 'u', 'm', 'm', 'y', '0', '0', '0' };

	//create 10 dummy names
	for (uint8_t i = 0; i < 10; i++)
	{
		//fill names
		dummy[7] = i + 48;
		memcpy(&temp_data[i * EEPROM_PAGE_SIZE], dummy, 8);

		//fill scores
		temp_data[EEPROM_PAGE_SIZE * 10 + i] = 10 - i;
	}

	// earse game configuration to 0
	temp_data[96] = 0; // speed
	temp_data[97] = 0; // color

	Eeprom_SendData(0, temp_data, EEPROM_SIZE);

	delay(5000);

	Eeprom_ReadData(0, temp_data, EEPROM_SIZE);

}

static void snek_eeprom_getconfig(snek_game_t *p_snek_game)
{
	uint8_t temp_array[8] = {0};
	Eeprom_ReadData(96, temp_array, 8);

	p_snek_game->game_config.speed = temp_array[0];
	p_snek_game->game_config.color = temp_array[1];

	return;
}

static void snek_eeprom_setconfig(snek_game_t *p_snek_game)
{
	uint8_t temp_array[8];

	temp_array[0] = p_snek_game->game_config.speed;
	temp_array[1] = p_snek_game->game_config.color;

	Eeprom_SendData(96, temp_array, 8);

	return;
}

static uint8_t snek_ui_mainmenu_check_update(snek_game_t *p_snek_game)
{
	// check if button down is clicked
	if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_DOWN))
	{
		// move button cursor down
		p_snek_game->menu_buttons = (p_snek_game->menu_buttons + 1) % 4;
		SNEK_SET_BIT(p_snek_game->CR1, SNEK_CR1_DRAW_OLED);
		SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_DOWN);
	}

	// check if button up is clicked
	if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_UP))
	{
		// move button up
		p_snek_game->menu_buttons = (p_snek_game->menu_buttons + 3) % 4;
		SNEK_SET_BIT(p_snek_game->CR1, SNEK_CR1_DRAW_OLED);
		SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_UP);
	}

	// check if enter is clicked
	if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_ENTER))
	{
		// change menu screen
		if (SNEK_CHECK_BIT(p_snek_game->SR1, SNEK_SR1_ACTIVE_BUTTON_0))
		{
			p_snek_game->game_state = GAMESTATE_GAME;
		}
		else if (SNEK_CHECK_BIT(p_snek_game->SR1, SNEK_SR1_ACTIVE_BUTTON_1))
		{
			p_snek_game->game_state = GAMESTATE_SCORES;
		}
		else if (SNEK_CHECK_BIT(p_snek_game->SR1, SNEK_SR1_ACTIVE_BUTTON_2))
		{
			p_snek_game->game_state = GAMESTATE_SETTINGS;
		}
		else if (SNEK_CHECK_BIT(p_snek_game->SR1, SNEK_SR1_ACTIVE_BUTTON_3))
		{
			p_snek_game->game_state = GAMESTATE_ABOUT;
		}

		//debounce delay
		delay(840000);
		SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_ENTER);

		return 1;

	}

	return 0;
}

static uint8_t snek_ui_settingsmenu_check_update(snek_game_t *p_snek_game)
{
	// check if button down is clicked
	if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_DOWN))
	{
		// move button cursor down
		p_snek_game->menu_buttons = (p_snek_game->menu_buttons + 1) % 4;
		SNEK_SET_BIT(p_snek_game->CR1, SNEK_CR1_DRAW_OLED);
		SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_DOWN);
	}

	// check if button up is clicked
	if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_UP))
	{
		// move button up
		p_snek_game->menu_buttons = (p_snek_game->menu_buttons + 3) % 4;
		SNEK_SET_BIT(p_snek_game->CR1, SNEK_CR1_DRAW_OLED);
		SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_UP);
	}

	if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_LEFT))
	{
		// switch setting
		if (SNEK_CHECK_BIT(p_snek_game->SR1, SNEK_SR1_ACTIVE_BUTTON_0))
		{
			p_snek_game->game_config.speed = (p_snek_game->game_config.speed + 9) % 10;
		}
		else if (SNEK_CHECK_BIT(p_snek_game->SR1, SNEK_SR1_ACTIVE_BUTTON_1))
		{
			p_snek_game->game_config.color ^= 0x01;
		}

		SNEK_SET_BIT(p_snek_game->CR1, SNEK_CR1_DRAW_OLED);
		SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_LEFT);
	}

	if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_RIGHT))
	{
		// switch setting
		if (SNEK_CHECK_BIT(p_snek_game->SR1, SNEK_SR1_ACTIVE_BUTTON_0))
		{
			p_snek_game->game_config.speed = (p_snek_game->game_config.speed + 1) % 10;
		}
		else if (SNEK_CHECK_BIT(p_snek_game->SR1, SNEK_SR1_ACTIVE_BUTTON_1))
		{
			p_snek_game->game_config.color ^= 0x01;
		}

		SNEK_SET_BIT(p_snek_game->CR1, SNEK_CR1_DRAW_OLED);
		SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_RIGHT);
	}

	// check if enter is clicked
	if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_ENTER))
	{
		// change menu screen
		if (SNEK_CHECK_BIT(p_snek_game->SR1, SNEK_SR1_ACTIVE_BUTTON_2))
		{
			//erase eeprom
			snek_eeprom_erase();
			snek_eeprom_getconfig(p_snek_game);
		}
		else if (SNEK_CHECK_BIT(p_snek_game->SR1, SNEK_SR1_ACTIVE_BUTTON_3))
		{
			//go back
			p_snek_game->game_state = GAMESTATE_MENU;
			snek_eeprom_setconfig(p_snek_game);

		}

		//debounce delay
		delay(840000);
		SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_ENTER);

		return 1;

	}

	return 0;
}
// snek_map - functions used during game to draw move snek, update fruity positions etc.

static void snek_map_clearmap(snek_game_t *p_snek_game)
{
	memset(p_snek_game->game_map, 0, sizeof(node_t) * SNEK_UI_NO_NODES);
}

static void snek_map_createnode(snek_game_t *p_snek_game, uint8_t new_node_number)
{

	if (!(SNEK_CHECK_NODE_LIMITS(new_node_number)))
	{
		SNEK_SET_BIT(p_snek_game->SR1, SNEK_SR1_ERROR_NODE);
		return;
	}

	p_snek_game->game_map[new_node_number].node_taken = 1;

	// if this first node made
	if (p_snek_game->snek_lenght == 0)
	{
		// to first node assign null to last position
		p_snek_game->game_map[new_node_number].last_node_pos = SNEK_NULL_NODE;

		p_snek_game->tail_address = new_node_number;
	}
	else
	{
		// to the next node assign position of last head
		p_snek_game->game_map[new_node_number].last_node_pos = p_snek_game->head_address;
		// to the last head assign node that we are creating
		p_snek_game->game_map[p_snek_game->head_address].next_node_pos = new_node_number;

	}

	// we are creating head to next node is NULL
	p_snek_game->game_map[new_node_number].next_node_pos = SNEK_NULL_NODE;
	// node that we are creating is new head
	p_snek_game->head_address = new_node_number;
	p_snek_game->snek_lenght++;

}

static void snek_map_deletenode(snek_game_t *p_snek_game, uint8_t last_node_number)
{
	uint8_t temp_node_pos;

	if (!(SNEK_CHECK_NODE_LIMITS(last_node_number)))
	{
		SNEK_SET_BIT(p_snek_game->SR1, SNEK_SR1_ERROR_NODE);
		return;
	}

	// save new tail to temporary buffer
	temp_node_pos = p_snek_game->game_map[p_snek_game->tail_address].next_node_pos;

	// clear node
	p_snek_game->game_map[p_snek_game->tail_address].last_node_pos = 0;
	p_snek_game->game_map[p_snek_game->tail_address].next_node_pos = 0;
	p_snek_game->game_map[p_snek_game->tail_address].node_taken = 0;

	// assign new tail
	p_snek_game->tail_address = temp_node_pos;
	p_snek_game->game_map[temp_node_pos].last_node_pos = SNEK_NULL_NODE;
	p_snek_game->snek_lenght--;

}

static void snek_map_drawnode(snek_game_t *p_snek_game, uint8_t new_node_number)
{
	snek_map_createnode(p_snek_game, new_node_number);
	// draw node square
	uint8_t temp_x_pos = ((new_node_number % 16) * 8);
	uint8_t temp_y_pos = 16 + ((new_node_number / 16) * 8);
	GFX_DrawFillRectangle(temp_x_pos, temp_y_pos, SNEK_UI_NODE_HEIGHT, SNEK_UI_NODE_WIDTH, YELLOWBLUE);
}

static void snek_map_erasenode(snek_game_t *p_snek_game, uint8_t last_node_number)
{
	snek_map_deletenode(p_snek_game, last_node_number);
	// draw node square
	uint8_t temp_x_pos = ((last_node_number % 16) * 8);
	uint8_t temp_y_pos = 16 + ((last_node_number / 16) * 8);
	GFX_DrawFillRectangle(temp_x_pos, temp_y_pos, SNEK_UI_NODE_HEIGHT, SNEK_UI_NODE_WIDTH, BLACK);
}

static void snek_map_drawfruity(snek_game_t *p_snek_game, uint8_t fruity_node)
{
	// draw node square
	uint8_t temp_x_pos = ((fruity_node % 16) * 8);
	uint8_t temp_y_pos = 16 + ((fruity_node / 16) * 8);

	GFX_DrawFillRoundRectangle(temp_x_pos, temp_y_pos, SNEK_UI_NODE_HEIGHT, SNEK_UI_NODE_WIDTH, 1, YELLOWBLUE);
	//GFX_DrawFillRectangle(temp_x_pos, temp_y_pos, SNEK_UI_NODE_HEIGHT, SNEK_UI_NODE_WIDTH, YELLOWBLUE);
}

static void snek_map_check_direction(snek_game_t *p_snek_game)
{
	// if we are moving vertically
	if ((SNEK_CHECK_BIT(p_snek_game->SR1, SNEK_SR1_SNEKMOVE_DOWN)) || (SNEK_CHECK_BIT(p_snek_game->SR1, SNEK_SR1_SNEKMOVE_UP)))
	{
		// ignore up/down buttons and change snek new movement direction
		SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_DOWN);
		SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_UP);

		if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_LEFT))
		{
			// set new direction, reset old direction and button flag
			SNEK_SET_BIT(p_snek_game->SR1, SNEK_SR1_SNEKMOVE_LEFT);
			SNEK_RESET_BIT(p_snek_game->SR1, SNEK_CR1_BUTTON_LEFT);
			SNEK_RESET_BIT(p_snek_game->SR1, SNEK_SR1_SNEKMOVE_DOWN);
			SNEK_RESET_BIT(p_snek_game->SR1, SNEK_SR1_SNEKMOVE_UP);
		}
		else if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_RIGHT))
		{
			SNEK_SET_BIT(p_snek_game->SR1, SNEK_SR1_SNEKMOVE_RIGHT);
			SNEK_RESET_BIT(p_snek_game->SR1, SNEK_CR1_BUTTON_RIGHT);
			SNEK_RESET_BIT(p_snek_game->SR1, SNEK_SR1_SNEKMOVE_DOWN);
			SNEK_RESET_BIT(p_snek_game->SR1, SNEK_SR1_SNEKMOVE_UP);
		}
	}
	else if ((SNEK_CHECK_BIT(p_snek_game->SR1, SNEK_SR1_SNEKMOVE_LEFT)) || (SNEK_CHECK_BIT(p_snek_game->SR1, SNEK_SR1_SNEKMOVE_RIGHT)))
	{
		// same if we are in horizontal movement
		SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_LEFT);
		SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_RIGHT);

		if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_UP))
		{
			// set new direction, reset old direction and button flag
			SNEK_SET_BIT(p_snek_game->SR1, SNEK_SR1_SNEKMOVE_UP);
			SNEK_RESET_BIT(p_snek_game->SR1, SNEK_CR1_BUTTON_UP);
			SNEK_RESET_BIT(p_snek_game->SR1, SNEK_SR1_SNEKMOVE_LEFT);
			SNEK_RESET_BIT(p_snek_game->SR1, SNEK_SR1_SNEKMOVE_RIGHT);
		}
		else if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_DOWN))
		{
			SNEK_SET_BIT(p_snek_game->SR1, SNEK_SR1_SNEKMOVE_DOWN);
			SNEK_RESET_BIT(p_snek_game->SR1, SNEK_CR1_BUTTON_DOWN);
			SNEK_RESET_BIT(p_snek_game->SR1, SNEK_SR1_SNEKMOVE_LEFT);
			SNEK_RESET_BIT(p_snek_game->SR1, SNEK_SR1_SNEKMOVE_RIGHT);
		}
	}
}

static void snek_map_check_fruity(snek_game_t *p_snek_game)
{
	if (SNEK_CHECK_BIT(p_snek_game->SR1, SNEK_SR1_NO_FRUITY))
	{
		// look for a place for a fruity node
		do
		{
			p_snek_game->fruity_node = rand() % 96;
		} while (p_snek_game->game_map[p_snek_game->fruity_node].node_taken);

		snek_map_drawfruity(p_snek_game, p_snek_game->fruity_node);
		SNEK_RESET_BIT(p_snek_game->SR1, SNEK_SR1_NO_FRUITY);
	}
}

static uint8_t snek_map_compute_newposition(snek_game_t *p_snek_game)
{
	uint8_t temp_new_head;

	if (SNEK_CHECK_BIT(p_snek_game->SR1, SNEK_SR1_SNEKMOVE_DOWN))
	{
		// head = same column + new row
		temp_new_head = (p_snek_game->head_address) % 16 + ((((p_snek_game->head_address / 16) + 1) % 6 * 16));
	}
	else if (SNEK_CHECK_BIT(p_snek_game->SR1, SNEK_SR1_SNEKMOVE_UP))
	{
		// head = same column + new row
		temp_new_head = (p_snek_game->head_address) % 16 + ((((p_snek_game->head_address / 16) + 5) % 6 * 16));
	}
	else if (SNEK_CHECK_BIT(p_snek_game->SR1, SNEK_SR1_SNEKMOVE_LEFT))
	{
		//head = new column + same row
		temp_new_head = (p_snek_game->head_address + 15) % 16 + ((p_snek_game->head_address / 16) * 16);
	}
	else if (SNEK_CHECK_BIT(p_snek_game->SR1, SNEK_SR1_SNEKMOVE_RIGHT))
	{
		//head = new column + same row
		temp_new_head = (p_snek_game->head_address + 1) % 16 + ((p_snek_game->head_address / 16) * 16);
	}

	return temp_new_head;
}
// snek_gamestate - main state machine functions

static void snek_gamestate_menu(snek_game_t *p_snek_game)
{

	// *** INIT SCREEN *** //
	SSD1306_Clear(BLACK);
	p_snek_game->menu_buttons = BUTTON_0_START_SPEED;
	SNEK_SET_BIT(p_snek_game->SR1, SNEK_SR1_ACTIVE_BUTTON_0);
	// set bit to draw menu for the first time
	snek_eeprom_getconfig(p_snek_game);
	SNEK_SET_BIT(p_snek_game->CR1, SNEK_CR1_DRAW_OLED);
	SNEK_RESET_BIT(p_snek_game->SR1, SNEK_SR1_ERROR_NODE);

	// *** ACTIVE SCREEN *** //
	while (SCREEN_ACTIVE)
	{
		// update
		if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_DRAW_OLED))
		{

			snek_ui_draw_mainmenu_button((uint8_t*) "Start", 0, 0);
			snek_ui_draw_mainmenu_button((uint8_t*) "High scores", 1, 0);
			snek_ui_draw_mainmenu_button((uint8_t*) "Settings", 2, 0);
			snek_ui_draw_mainmenu_button((uint8_t*) "About", 3, 0);
			// clear active buttons bits
			p_snek_game->SR1 &= ~(15U << SNEK_SR1_ACTIVE_BUTTON_0);

			//draw selected button
			//set active window bit
			switch (p_snek_game->menu_buttons)
			{
			case (BUTTON_0_START_SPEED):
				snek_ui_draw_mainmenu_button((uint8_t*) "Start", 0, 1);
				SNEK_SET_BIT(p_snek_game->SR1, SNEK_SR1_ACTIVE_BUTTON_0);
				break;

			case (BUTTON_1_SCORES_COLOR):
				snek_ui_draw_mainmenu_button((uint8_t*) "High scores", 1, 1);
				SNEK_SET_BIT(p_snek_game->SR1, SNEK_SR1_ACTIVE_BUTTON_1);
				break;

			case (BUTTON_2_SETTINGS_ERASE):
				snek_ui_draw_mainmenu_button((uint8_t*) "Settings", 2, 1);
				SNEK_SET_BIT(p_snek_game->SR1, SNEK_SR1_ACTIVE_BUTTON_2);
				break;

			case (BUTTON_3_ABOUT_OK):
				snek_ui_draw_mainmenu_button((uint8_t*) "About", 3, 1);
				SNEK_SET_BIT(p_snek_game->SR1, SNEK_SR1_ACTIVE_BUTTON_3);
				break;
			}

			// sent data to OLED
			SSD1306_Display();
			SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_DRAW_OLED);
		}

		// check for new update
		if (snek_ui_mainmenu_check_update(p_snek_game) == 1)
			return;
	}
}

static void snek_gamestate_about(snek_game_t *p_snek_game)
{

	// *** INIT SCREEN *** //
	SSD1306_Clear(BLACK);
	snek_ui_draw_mainmenu_button((uint8_t*) "Snek loves fruity", 0, 1);
	snek_ui_draw_mainmenu_button((uint8_t*) "a lot", 1, 1);
	GFX_Image(30, 26, gImage_bitmap, 85, 48, YELLOWBLUE);
	snek_map_drawfruity(p_snek_game, 48);
	snek_map_drawfruity(p_snek_game, 66);
	snek_map_drawfruity(p_snek_game, 81);
	SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_ENTER);
	SSD1306_Display();

	while (SCREEN_ACTIVE)
	{
		if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_ENTER))
		{
			delay(840000);
			p_snek_game->game_state = GAMESTATE_MENU;
			SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_ENTER);
			break;
		}
	}
}

static void snek_gamestate_settings(snek_game_t *p_snek_game)
{
	// *** INIT SCREEN *** //
	SSD1306_Clear(BLACK);
	SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_ENTER);
	snek_eeprom_getconfig(p_snek_game);
	p_snek_game->menu_buttons = BUTTON_0_START_SPEED;
	SNEK_SET_BIT(p_snek_game->SR1, SNEK_SR1_ACTIVE_BUTTON_0);
	SNEK_SET_BIT(p_snek_game->CR1, SNEK_CR1_DRAW_OLED);

	// *** ACTIVE SCREEN *** //

	while (SCREEN_ACTIVE)
	{
		// update
		if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_DRAW_OLED))
		{

			uint8_t text[16];
			sprintf((char*) text, "Speed <%d>", p_snek_game->game_config.speed);
			snek_ui_draw_mainmenu_button((uint8_t*) text, 0, 0);
			sprintf((char*) text, "Color <%d>", p_snek_game->game_config.color);
			snek_ui_draw_mainmenu_button((uint8_t*) text, 1, 0);
			snek_ui_draw_mainmenu_button((uint8_t*) "Erase scores", 2, 0);
			snek_ui_draw_ok_button(OFF);
			// clear active buttons bits
			p_snek_game->SR1 &= ~(15U << SNEK_SR1_ACTIVE_BUTTON_0);

			//draw selected button
			//set active window bit
			switch (p_snek_game->menu_buttons)
			{
			case (BUTTON_0_START_SPEED):
				sprintf((char*) text, "Speed <%d>", p_snek_game->game_config.speed);
				snek_ui_draw_mainmenu_button((uint8_t*) text, 0, 1);
				SNEK_SET_BIT(p_snek_game->SR1, SNEK_SR1_ACTIVE_BUTTON_0);
				break;

			case (BUTTON_1_SCORES_COLOR):
				sprintf((char*) text, "Color <%d>", p_snek_game->game_config.color);
				snek_ui_draw_mainmenu_button((uint8_t*) text, 1, 1);
				SNEK_SET_BIT(p_snek_game->SR1, SNEK_SR1_ACTIVE_BUTTON_1);
				break;

			case (BUTTON_2_SETTINGS_ERASE):
				snek_ui_draw_mainmenu_button((uint8_t*) "Erase scores", 2, 1);
				SNEK_SET_BIT(p_snek_game->SR1, SNEK_SR1_ACTIVE_BUTTON_2);
				break;

			case (BUTTON_3_ABOUT_OK):
				snek_ui_draw_ok_button(ON);
				SNEK_SET_BIT(p_snek_game->SR1, SNEK_SR1_ACTIVE_BUTTON_3);
				break;
			}

			// sent data to OLED
			SSD1306_Display();
			SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_DRAW_OLED);
		}

		// check for new update
		if (snek_ui_settingsmenu_check_update(p_snek_game) == 1)
			return;
	}
}

static void snek_gamestate_scores(snek_game_t *p_snek_game)
{
	// *** INIT SCREEN *** //
	static uint8_t temp_array[128];
	uint8_t temp_name[16] =
			{ 0 };
	uint8_t temp_score;
	uint8_t display_text[24];
	uint8_t offset = 0;

	SSD1306_Clear(YELLOWBLUE);
	SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_ENTER);
	Eeprom_ReadData(0, temp_array, 128);

	// *** ACTIVE SCREEN *** //
	while (SCREEN_ACTIVE)
	{

		// check if buttons are clicked
		if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_UP))
		{
			(offset == 0) ? (offset = 0) : (offset--);
			SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_UP);
		}

		if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_DOWN))
		{
			(offset == 6) ? (offset = 6) : (offset++);
			SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_DOWN);
		}

		if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_ENTER))
		{
			//debounce delay
			delay(840000);
			SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_ENTER);
			p_snek_game->game_state = GAMESTATE_MENU;
			break;
		}

		// display 4 scores
		for (uint8_t i = 0; i < 4; i++)
		{
			memcpy(temp_name, &(temp_array[(i + offset) * 8]), 8);
			temp_score = temp_array[80 + i + offset];
			sprintf((char*) display_text, "%d. %8s %d", (i + offset + 1), temp_name, temp_score);
			snek_ui_draw_mainmenu_button(display_text, i, 1);
		}

		SSD1306_Display();

	}

}

static void snek_gamestate_game(snek_game_t *p_snek_game)
{

	// init parameters
	p_snek_game->snek_lenght = 0;
	snek_map_clearmap(p_snek_game);
	SSD1306_Clear(BLACK);

	// set speed
	snek_ui_modify_speed(p_snek_game);
	// draw scorebox as a top button
	GFX_DrawRectangle(0, 0, SNEK_UI_BUTTON_WIDTH, SNEK_UI_BUTTON_HEIGHT, YELLOWBLUE);
	GFX_DrawString(5, 0 + SNEK_UI_CHAR_OFFSET, "SNEK", YELLOWBLUE, BLACK);
	GFX_DrawString(60, 0 + SNEK_UI_CHAR_OFFSET, "SCORE: 0", YELLOWBLUE, BLACK);

	// create snake in game map
	snek_map_drawnode(p_snek_game, 40);
	snek_map_drawnode(p_snek_game, 41);
	snek_map_drawnode(p_snek_game, 42);

	// set first movement as right
	SNEK_SET_BIT(p_snek_game->SR1, SNEK_SR1_SNEKMOVE_RIGHT);
	SNEK_SET_BIT(p_snek_game->SR1, SNEK_SR1_NO_FRUITY);
	SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_GAME_TICK);

	SSD1306_Display();

	while (SCREEN_ACTIVE)

	{
		// game tick
		if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_GAME_TICK))
		{
			// check if direction change is required
			snek_map_check_direction(p_snek_game);

			// check if new fruity is required
			snek_map_check_fruity(p_snek_game);

			// calculate next node position in array
			uint8_t temp_new_head = snek_map_compute_newposition(p_snek_game);

			// check if new node is taken -> if yes then collision
			if (p_snek_game->game_map[temp_new_head].node_taken)
			{
				p_snek_game->game_state = GAMESTATE_OVER;
				return;
			}

			// check if next node is fruity node
			if (p_snek_game->fruity_node == temp_new_head)
			{
				SNEK_SET_BIT(p_snek_game->SR1, SNEK_SR1_NO_FRUITY);

			}
			else
			{
				// erase tail
				snek_map_erasenode(p_snek_game, p_snek_game->tail_address);
			}

			// draw new head
			snek_map_drawnode(p_snek_game, temp_new_head);

			snek_ui_updatescore(p_snek_game);

			SSD1306_Display();

			SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_GAME_TICK);
		}
	}

}

static void snek_gamestate_over(snek_game_t *p_snek_game)
{

	// *** INIT SCREEN *** //
	snek_ui_gameover_animation();

	uint8_t msg[16];
	sprintf((char*) msg, "Score: %d", (p_snek_game->snek_lenght - SNEK_GAME_START_LENGHT + p_snek_game->game_config.speed));

	// draw buttons
	snek_ui_draw_mainmenu_button((uint8_t*) "Game over", 0, 1);
	snek_ui_draw_mainmenu_button((uint8_t*) msg, 1, 1);
	snek_ui_draw_mainmenu_button((uint8_t*) "Name:", 2, 1);
	snek_ui_draw_mainmenu_button((uint8_t*) "", 3, 0);

	// draw ok button
	snek_ui_draw_ok_button(OFF);

	// init text editor values
	p_snek_game->txt_edit.pos = 0;
	memset(p_snek_game->txt_edit.name, ' ', 8);
	p_snek_game->txt_edit.val = 'A';

	SSD1306_Display();

	// *** ACTIVE SCREEN *** //
	while (SCREEN_ACTIVE)
	{
		delay(840000);

		// check if there is a letter to change
		if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_UP))
		{
			delay(840000);
			SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_UP);

			if (p_snek_game->txt_edit.val >= 'Z' || p_snek_game->txt_edit.val < 'A')
			{
				p_snek_game->txt_edit.val = 'A';
			}
			else
			{
				p_snek_game->txt_edit.val++;
			}
		}
		else if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_DOWN))
		{
			delay(840000);
			SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_DOWN);

			if (p_snek_game->txt_edit.val > 'Z' || p_snek_game->txt_edit.val <= 'A')
			{
				p_snek_game->txt_edit.val = 'Z';
			}
			else
			{
				p_snek_game->txt_edit.val--;
			}
		}

		// check if there is position to change
		if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_RIGHT))
		{
			delay(840000);
			SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_RIGHT);
			(p_snek_game->txt_edit.pos == SNEK_UI_MAX_NAME_LENGHT) ? p_snek_game->txt_edit.pos = 0 : p_snek_game->txt_edit.pos++;
			p_snek_game->txt_edit.val = p_snek_game->txt_edit.name[p_snek_game->txt_edit.pos];
		}
		else if (SNEK_CHECK_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_LEFT))
		{
			delay(840000);
			SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_LEFT);
			(p_snek_game->txt_edit.pos == 0) ? p_snek_game->txt_edit.pos = SNEK_UI_MAX_NAME_LENGHT : p_snek_game->txt_edit.pos--;
			p_snek_game->txt_edit.val = p_snek_game->txt_edit.name[p_snek_game->txt_edit.pos];
		}

		GFX_DrawLine(40, 61, 48 + 45, 61, BLACK);
		// jump between letters
		if (p_snek_game->txt_edit.pos != SNEK_UI_MAX_NAME_LENGHT)
		{
			GFX_DrawLine((p_snek_game->txt_edit.pos * 6) + 40, 61, (p_snek_game->txt_edit.pos * 6) + 45, 61, YELLOWBLUE);
			GFX_DrawChar((p_snek_game->txt_edit.pos * 6) + SNEK_UI_TXT_EDITOR_OFFSET_LEFT, SSD1306_LCDHEIGHT - SNEK_UI_TXT_EDITOR_OFFSET_BOTTOM,
					p_snek_game->txt_edit.val, YELLOWBLUE,
					BLACK);
			SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_ENTER);
			snek_ui_draw_ok_button(OFF);
		}
		else
		{
			snek_ui_draw_ok_button(ON);
		}

		p_snek_game->txt_edit.name[p_snek_game->txt_edit.pos] = p_snek_game->txt_edit.val;

		SSD1306_Display();

		// check if button enter is clicked and go back to main menu
		if (SNEK_CHECK_BIT(p_snek_game->CR1,SNEK_CR1_BUTTON_ENTER) && (p_snek_game->txt_edit.pos == SNEK_UI_MAX_NAME_LENGHT))
		{

			//debounce delay
			delay(840000);
			// save data on eeprom
			p_snek_game->game_state = GAMESTATE_SAVE;
			SNEK_RESET_BIT(p_snek_game->CR1, SNEK_CR1_BUTTON_ENTER);
			break;
		}

	}
}

static void snek_gamestate_save(snek_game_t *p_snek_game)
{
	uint8_t save_name[SNEK_UI_MAX_NAME_LENGHT] =
			{ 0 };
	uint8_t save_score = p_snek_game->snek_lenght - SNEK_GAME_START_LENGHT + p_snek_game->game_config.speed;
	uint8_t new_position = 99;
	static uint8_t temp_buffer[EEPROM_SCORES_SIZE];

	// fill name
	for (uint8_t arr_count = 0; arr_count < SNEK_UI_MAX_NAME_LENGHT; arr_count++)
	{
		save_name[arr_count] = p_snek_game->txt_edit.name[arr_count];
	}

	// read scores from eeprom

	Eeprom_ReadData(0, temp_buffer, EEPROM_SCORES_SIZE);
	// check if score is qualifying to top
	for (uint8_t i = 0; i < 10; i++)
	{
		if (save_score >= temp_buffer[80 + i])
		{
			new_position = i;
			break;
		}
	}

	// if new score is outside high scores
	if (new_position > 10)
	{
		p_snek_game->game_state = GAMESTATE_MENU;
		return;

	}

	// write scores in new order
	for (uint8_t i = 1; i < (10 - new_position); i++)
	{
		//write names in new order
		memcpy(&temp_buffer[8 * (10 - i)], &temp_buffer[8 * (10 - i - 1)], EEPROM_PAGE_SIZE);
		//write scores in new order
		temp_buffer[80 + (10 - i)] = temp_buffer[80 + (10 - i - 1)];
	}

	//add new score
	memcpy(&temp_buffer[new_position * 8], save_name, EEPROM_PAGE_SIZE);
	temp_buffer[80 + new_position] = save_score;

	// send new scores to eeprom

	Eeprom_SendData(0, temp_buffer, EEPROM_PAGE_SIZE * 12);

	p_snek_game->game_state = GAMESTATE_MENU;
}

// main function

void snek(snek_game_t *snek_game)
{
	switch (snek_game->game_state)
	{

	case GAMESTATE_MENU:
		snek_gamestate_menu(snek_game);
		break;

	case GAMESTATE_SCORES:
		snek_gamestate_scores(snek_game);
		break;

	case GAMESTATE_ABOUT:
		snek_gamestate_about(snek_game);
		break;

	case GAMESTATE_SETTINGS:
		snek_gamestate_settings(snek_game);
		break;

	case GAMESTATE_GAME:
		snek_gamestate_game(snek_game);
		break;

	case GAMESTATE_OVER:
		snek_gamestate_over(snek_game);
		break;

	case GAMESTATE_SAVE:
		snek_gamestate_save(snek_game);
		break;

	default:
		break;
	}
}

// callback to connect hardware buttons with snek game

void snek_button_callback(uint8_t GPIO_Pin, snek_game_t *snek_game)
{

// set button clicked bit in CR1
	switch (GPIO_Pin)
	{
	case SNEK_BUTTON_DOWN:
		SNEK_SET_BIT(snek_game->CR1, SNEK_CR1_BUTTON_DOWN);
		break;

	case SNEK_BUTTON_UP:
		SNEK_SET_BIT(snek_game->CR1, SNEK_CR1_BUTTON_UP);
		break;

	case SNEK_BUTTON_LEFT:
		SNEK_SET_BIT(snek_game->CR1, SNEK_CR1_BUTTON_LEFT);
		break;

	case SNEK_BUTTON_RIGHT:
		SNEK_SET_BIT(snek_game->CR1, SNEK_CR1_BUTTON_RIGHT);
		break;

	case SNEK_BUTTON_ENTER:
		SNEK_SET_BIT(snek_game->CR1, SNEK_CR1_BUTTON_ENTER);
		break;
	}

}

// gametick callback

void snek_gametick_callback(snek_game_t *snek_game)
{
	SNEK_SET_BIT(snek_game->CR1, SNEK_CR1_GAME_TICK);
}
