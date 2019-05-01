#ifndef LCD_H
#define LCD_H

#include "project.h"

void writeCommand(int command);
void writeData(int data);
void writeReg(int reg, int val);
uint8 readData();
uint8_t readReg(uint8_t reg);
void waitPoll(uint8_t regname, uint8_t waitflag);
void PLLinit();
void initialize();
int begin();
void setScrollWindow(int16_t x, int16_t y, uint8_t mode);
void circleHelper(int16_t x, int16_t y, int16_t r, uint16_t colour, int filled);
void graphicsMode();
void textMode();
void textSetCursor(uint16_t x, uint16_t y);
void textColour(uint16_t foreColour, uint16_t bgColour);
void textTransparent(uint16_t foreColour);
void textWrite(const char* buffer);
void textEnlarge(uint8_t scale);

#endif