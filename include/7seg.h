//
// Created by alexis on 28/05/25.
//

#ifndef INC_7SEG_H
#define INC_7SEG_H

#ifndef SEVEN_SEG_H
#define SEVEN_SEG_H

int init_display(void);

void clear_display(int fd);

void display_digit(int fd, int pos, int digit);

void display_number(int fd, int number);

#endif // SEVEN_SEG_H

#endif //INC_7SEG_H
