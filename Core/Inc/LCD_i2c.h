#ifndef LCD_I2C_H_
#define LCD_I2C_H_

#include "stm32f4xx_hal.h"

#define LCD_ADDR       (0x27 << 1)  //I2C adresa

#define LCD_CLEARDISPLAY   0x01   //ocisti zaslon
#define LCD_ENTRYMODESET   0x04   //nacin unosa znakova
#define LCD_DISPLAYCONTROL 0x08   //ukljucivanje/iskljucivanje zaslona
#define LCD_FUNCTIONSET    0x20
#define LCD_SETDDRAMADDR   0x80

#define LCD_ENTRYLEFT           0x02  //Unos s lijeva na desno


#define LCD_DISPLAYON    0x04   //ukljucivanje zaslona
#define LCD_DISPLAYOFF   0x00   //iskljucivanje zaslna

#define LCD_4BITMODE  0x00    //4-bitni nacin rada
#define LCD_2LINE      0x08
#define LCD_5x8DOTS    0x00

#define LCD_BACKLIGHT    0x08
#define LCD_NOBACKLIGHT  0x00

#define LCD_ENABLE    0x04
#define LCD_RS        0x01


void HD44780_Init(uint8_t rows);
void HD44780_Clear(void);
void HD44780_SetCursor(uint8_t col, uint8_t row);
void HD44780_PrintStr(const char *str);
void HD44780_Backlight(void);
void HD44780_NoBacklight(void);
void HD44780_Display(void);
void HD44780_NoDisplay(void);

#endif /* LCD_I2C_H_ */
