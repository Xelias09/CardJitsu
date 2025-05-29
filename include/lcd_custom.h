//
// Created by alexis on 29/05/25.
//

#ifndef LCD_CUSTOM_H
#define LCD_CUSTOM_H

int lcd_setup(int i2c_addr);

void lcd_display_message(int lcd_fd, const char *message);

void lcd_scroll_text(int lcd_fd, const char *text, int row, int delay_ms);

void lcd_clear(int lcd_fd);

#endif //LCD_CUSTOM_H
