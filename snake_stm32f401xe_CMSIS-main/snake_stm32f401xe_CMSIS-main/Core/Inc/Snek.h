/*
 * Snek.h
 *
 *  Created on: 17 sty 2022
 *      Author: ROJEK
 */

#ifndef INC_SNEK_H_
#define INC_SNEK_H_


typedef enum
{
	GAMESTATE_MENU,
	GAMESTATE_SCORES,
	GAMESTATE_ABOUT,
	GAMESTATE_SETTINGS,
	GAMESTATE_GAME,
	GAMESTATE_OVER,
	GAMESTATE_SAVE
} game_state_t;

typedef enum
{
	BUTTON_0_START_SPEED,
	BUTTON_1_SCORES_COLOR,
	BUTTON_2_SETTINGS_ERASE,
	BUTTON_3_ABOUT_OK
} menu_buttons_t;

#define SNEK_SET_BIT(ControlRegister,Bit)		(ControlRegister |= (1U << Bit))
#define SNEK_RESET_BIT(ControlRegister,Bit)		(ControlRegister &= ~(1U << Bit))
#define SNEK_CHECK_BIT(ControlRegister,Bit)		((ControlRegister >> Bit) & 1U)

// control register
#define SNEK_CR1_BUTTON_DOWN					0U		// button down clicked
#define SNEK_CR1_BUTTON_UP						1U		// button up clicked
#define SNEK_CR1_BUTTON_LEFT					2U		// button left clicked
#define SNEK_CR1_BUTTON_RIGHT					3U		// button right clicked
#define SNEK_CR1_BUTTON_ENTER					4U		// button enter clicked
#define SNEK_CR1_GAME_TICK						5U		// game tick
#define SNEK_CR1_DRAW_OLED						8U		// refresh screen with new values


// status register
#define SNEK_SR1_ACTIVE_BUTTON_0				0U		// main menu cursor on button start
#define SNEK_SR1_ACTIVE_BUTTON_1				1U		// main menu cursor on button scores
#define SNEK_SR1_ACTIVE_BUTTON_2				2U		// main menu cursor on button settings
#define SNEK_SR1_ACTIVE_BUTTON_3				3U		// main menu cursor on button about
#define SNEK_SR1_SNEKMOVE_DOWN					4U		// snek is moving down
#define SNEK_SR1_SNEKMOVE_UP					5U		// snek is moving up
#define SNEK_SR1_SNEKMOVE_LEFT					6U		// snek is moving left
#define SNEK_SR1_SNEKMOVE_RIGHT					7U		// snek is moving right
#define SNEK_SR1_ERROR_NODE						8U		// error creating or deleting node
#define SNEK_SR1_NO_FRUITY						9U		// fruity is required on the map


#define SNEK_BUTTON_DOWN						GPIO_PIN_8
#define SNEK_BUTTON_UP							GPIO_PIN_9
#define SNEK_BUTTON_LEFT						GPIO_PIN_6
#define SNEK_BUTTON_RIGHT						GPIO_PIN_12
#define SNEK_BUTTON_ENTER						GPIO_PIN_5

#define SNEK_BUTTON_DOWN_PORT					GPIOC
#define SNEK_BUTTON_UP_PORT						GPIOC
#define SNEK_BUTTON_LEFT_PORT					GPIOC
#define SNEK_BUTTON_RIGHT_PORT					GPIOA
#define SNEK_BUTTON_ENTER_PORT					GPIOC

#define SNEK_UI_BUTTON_WIDTH					128U
#define SNEK_UI_BUTTON_HEIGHT					15U
#define SNEK_UI_BUTTON_OFFSET					16U		// number of pixels from 1 button start to next one
#define SNEK_UI_CHAR_OFFSET						4U		// number of pixels from button border to char

//we take 48 x 128 game screen and divide it to 8x8 squares that will be fruity or snek parts
#define SNEK_UI_GAME_HEIGHT						48U
#define SNEK_UI_GAME_WIDTH						128U
#define SNEK_UI_NODE_HEIGHT						8U
#define SNEK_UI_NODE_WIDTH						8U
#define SNEK_UI_NO_NODES						((SNEK_UI_GAME_HEIGHT / SNEK_UI_NODE_HEIGHT) * (SNEK_UI_GAME_WIDTH / SNEK_UI_NODE_WIDTH))
#define SNEK_UI_MAX_NAME_LENGHT					8U
#define SNEK_UI_TXT_EDITOR_OFFSET_LEFT			40U
#define SNEK_UI_TXT_EDITOR_OFFSET_BOTTOM		12U
#define SNEK_UI_OK_BUTTON_POS_X					106U
#define SNEK_UI_OK_BUTTON_POS_Y					50U
#define SNEK_UI_OK_BUTTON_WIDTH					20U
#define SNEK_UI_OK_BUTTON_HEIGHT				11U

#define SNEK_GAME_HEAD_STARTPOS					8U + (3U * 16U)
#define SNEK_GAME_START_LENGHT					3U
#define SNEK_NULL_NODE							100U
#define SNEK_CHECK_NODE_LIMITS(NODE)			((NODE >= 0U) && (NODE < 96))
#define SCREEN_ACTIVE							1U


#define EEPROM_SIZE								128U
#define ON										1U
#define OFF										0U

// linked list to save snek shape and fruity
typedef struct node_t
{

	uint8_t node_taken; // node taken flag

	uint8_t last_node_pos; // last node position in array

	uint8_t next_node_pos; // next node position in array

} node_t;

typedef struct txt_edit_t
{

	uint8_t pos; // node taken flag

	uint8_t val; // last node position in array

	uint8_t name[SNEK_UI_MAX_NAME_LENGHT]; // next node position in array

} txt_edit_t;

typedef struct game_config_t
{
	uint8_t speed;

	uint8_t color;

}game_config_t;


typedef struct snek_game_t
{

	game_state_t game_state;			// enum state machine

	menu_buttons_t menu_buttons;		// enum buttons names

	volatile uint16_t CR1;				// control register 1

	volatile uint16_t SR1;				// status register 1

	uint8_t fruity_node;				// fruity node

	node_t game_map[SNEK_UI_NO_NODES];	// game map that will contain snake shape and fruit

	uint8_t head_address;				// snek first node 'address'

	uint8_t tail_address;				// snek last node

	uint16_t snek_lenght;				// snek lenght

	txt_edit_t txt_edit;				// text editor struct

	game_config_t game_config;			// game options

	TIM_TypeDef *p_game_tick_tim;		// game tick timer


} snek_game_t;


void snek(snek_game_t *snek_game);
void snek_button_callback(uint8_t GPIO_Pin, snek_game_t *snek_game);
void snek_gametick_callback(snek_game_t *snek_game);

#endif /* INC_SNEK_H_ */
