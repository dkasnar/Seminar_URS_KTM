#include "LCD_i2c.h"

extern I2C_HandleTypeDef hi2c1;

static uint8_t dpBacklight = 1;
//slanje 4 bita u LCD
static void lcd_send_nibble(uint8_t nibble, uint8_t mode) {
    uint8_t data = nibble | mode;
    	if(dpBacklight){
    		data |= LCD_BACKLIGHT;
    	}
    HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, &data, 1, HAL_MAX_DELAY);
    data |= LCD_ENABLE;
    HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, &data, 1, HAL_MAX_DELAY);
    HAL_Delay(1);
    if (data & LCD_ENABLE) {
    	data -= LCD_ENABLE;
    }
    HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, &data, 1, HAL_MAX_DELAY);
    HAL_Delay(1);
}

//Slanje jednog bajta kroz 2 nibble-a od 4 bita
static void lcd_send_byte(uint8_t byte, uint8_t mode) {
    lcd_send_nibble(byte & 0xF0, mode);
    lcd_send_nibble((byte & 0x0F) * 16, mode);
}


void HD44780_Init(uint8_t rows) {
    HAL_Delay(50);
    //Inicijalizacija sekvence za 4-bitni nacin rada
    lcd_send_nibble(0x30, 0);
    HAL_Delay(5);
    lcd_send_nibble(0x30, 0);
    HAL_Delay(5);
    lcd_send_nibble(0x20, 0);
    HAL_Delay(5);
    //funkcije
    uint8_t fn = LCD_4BITMODE | LCD_5x8DOTS;
      if (rows > 1) {
        fn |= LCD_2LINE;
      }
    lcd_send_byte(LCD_FUNCTIONSET | fn, 0);
    lcd_send_byte(LCD_DISPLAYCONTROL | LCD_DISPLAYON, 0);
    HD44780_Clear();
    lcd_send_byte(LCD_ENTRYMODESET | LCD_ENTRYLEFT, 0);
}

void HD44780_Clear(void) {
    lcd_send_byte(LCD_CLEARDISPLAY, 0);
    HAL_Delay(2);
}

void HD44780_SetCursor(uint8_t col, uint8_t row) {
    static const uint8_t row_offset[2] = { 0x00, 0x40 };
    if (row > 1){
        row = 1;
    }
    uint8_t ddram_addr = row_offset[row] + col;  //izracun konacne adrese, pocetak retka + stupac
    lcd_send_byte(LCD_SETDDRAMADDR | ddram_addr, 0);
}


void HD44780_PrintStr(const char str[])  //funkcija za ispis znakova na ekranu
{
    for(size_t i = 0; str[i] != '\0'; i++){
        lcd_send_byte((uint8_t)str[i], LCD_RS);
    }
}

void HD44780_Backlight(void) {
    dpBacklight = 1;

}

void HD44780_NoBacklight(void) {
    dpBacklight = 0;

}
//komande za paljenje/gasenje lcd ekrana
void HD44780_Display(void) {
    lcd_send_byte(LCD_DISPLAYCONTROL | LCD_DISPLAYON, 0);
}

void HD44780_NoDisplay(void) {
    lcd_send_byte(LCD_DISPLAYCONTROL | LCD_DISPLAYOFF, 0);
}
