# snake_stm32f401xe_CMSIS

I created this project to dive into CMSIS and bare-metal programming on STM32.
HAL was ok but i had a feeling that i do not understand whats going on even when i am using some peripherals.
So i worked mainly with reference manual to create this little project.

What hardware i used :
- Nucleo stm32f401re dev board  
- EEPROM 1kb I2C - 24AA01-I/P  
- SSD1306 OLED  
- 5 push buttons  
- 5x 10k pull up resistors  
- 5x 1k resitor + 5x 100nF capacitor as RC filter 
  
![alt text](https://github.com/Roju667/snake_stm32f401xe_CMSIS/blob/main/Schematic_snake_2022-01-27.png)
  
What libraries i used :  
- stm32f401xe.h - CMSIS lib
- SSD1306_OLED + GFX_BW + fonts - that i took from https://msalamon.pl/ HAL programming course  

All other drivers for peripherals i wrote with reference manual. Those are not perfect but are enough for this project.
I am planning to develop those along with further learning and new projects.  

Snek UI that i created :  
- First menu with 4 push buttons Start/Settings/High Scores/About  
- Settings menu with 3 options:  
*Speed configuration (that configure timer tick that controls game calculations)  
*Color inverse (i wanted to make color inversion for all the UI but i didnt finish this option)  
*Erase memory - earse eeprom - write dummies to all highscores and reset speed to 0  
- About screen - with snek drawing and history of the game in 5 words  
- High scores - displaying 4 scores that are saved in eeprom memory with option to scroll down between 1st and 10th place  
- Game screen - just snek game with updateing score on the top (snek can pass walls)  
- Game over screen - displaying score with option to write your name (8 chars in big ASCII letters) - that is saved to eeprom if its high enough  


