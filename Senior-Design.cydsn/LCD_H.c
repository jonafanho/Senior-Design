#include "LCD.h"
#include "LCD_H.h"

void init() {
    begin();
    writeReg(RA8875_PWRR, RA8875_PWRR_NORMAL | RA8875_PWRR_DISPON); // display on
    writeReg(RA8875_GPIOX, 1); // GPIOX on
    writeReg(RA8875_P1CR, RA8875_P1CR_ENABLE |(RA8875_PWM_CLK_DIV1024 & 0xF)); // PWM1 config
    writeReg(RA8875_P1DCR, 255); // PWM1 out
    setScrollWindow(0, 0, RA8875_SCROLL_BOTH);    
}

void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t colour) {
    /* Set X */
    writeCommand(0x91);
    writeData(x0);
    writeCommand(0x92);
    writeData(x0 >> 8);
    /* Set Y */
    writeCommand(0x93);
    writeData(y0);
    writeCommand(0x94);
    writeData(y0 >> 8);
    /* Set X1 */
    writeCommand(0x95);
    writeData(x1);
    writeCommand(0x96);
    writeData((x1) >> 8);
    /* Set Y1 */
    writeCommand(0x97);
    writeData(y1);
    writeCommand(0x98);
    writeData((y1) >> 8);
    /* Set Color */
    writeCommand(0x63);
    writeData((colour & 0xf800) >> 11);
    writeCommand(0x64);
    writeData((colour & 0x07e0) >> 5);
    writeCommand(0x65);
    writeData((colour & 0x001f));
    /* Draw! */
    writeCommand(RA8875_DCR);
    writeData(0x80);
    /* Wait for the command to finish */
    waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t colour, int filled) {
    w = x + w - 1;
    h = y + h - 1;
    /* Set X */
    writeCommand(0x91);
    writeData(x);
    writeCommand(0x92);
    writeData(x >> 8);
    /* Set Y */
    writeCommand(0x93);
    writeData(y);
    writeCommand(0x94);
    writeData(y >> 8);
    /* Set X1 */
    writeCommand(0x95);
    writeData(w);
    writeCommand(0x96);
    writeData((w) >> 8);
    /* Set Y1 */
    writeCommand(0x97);
    writeData(h);
    writeCommand(0x98);
    writeData((h) >> 8);
    /* Set Colour */
    writeCommand(0x63);
    writeData((colour & 0xf800) >> 11);
    writeCommand(0x64);
    writeData((colour & 0x07e0) >> 5);
    writeCommand(0x65);
    writeData((colour & 0x001f));
    /* Draw! */
    writeCommand(RA8875_DCR);
    if(filled)
        writeData(0xB0);
    else
        writeData(0x90);
    /* Wait for the command to finish */
    waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
    //CyDelay(10);
}

void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t colour, int filled) {
    /* Set Point 0 */
    writeCommand(0x91);
    writeData(x0);
    writeCommand(0x92);
    writeData(x0 >> 8);
    writeCommand(0x93);
    writeData(y0);
    writeCommand(0x94);
    writeData(y0 >> 8);
    /* Set Point 1 */
    writeCommand(0x95);
    writeData(x1);
    writeCommand(0x96);
    writeData(x1 >> 8);
    writeCommand(0x97);
    writeData(y1);
    writeCommand(0x98);
    writeData(y1 >> 8);
    /* Set Point 2 */
    writeCommand(0xA9);
    writeData(x2);
    writeCommand(0xAA);
    writeData(x2 >> 8);
    writeCommand(0xAB);
    writeData(y2);
    writeCommand(0xAC);
    writeData(y2 >> 8);
    /* Set Color */
    writeCommand(0x63);
    writeData((colour & 0xf800) >> 11);
    writeCommand(0x64);
    writeData((colour & 0x07e0) >> 5);
    writeCommand(0x65);
    writeData((colour & 0x001f));
    /* Draw! */
    writeCommand(RA8875_DCR);
    if (filled)
        writeData(0xA1);
    else
        writeData(0x81);
    /* Wait for the command to finish */
    waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t colour, int filled) {
    /* Set X */
    writeCommand(0x99);
    writeData(x);
    writeCommand(0x9a);
    writeData(x >> 8);
    /* Set Y */
    writeCommand(0x9b);
    writeData(y);
    writeCommand(0x9c);
    writeData(y >> 8);
    /* Set Radius */
    writeCommand(0x9d);
    writeData(r);
    /* Set Color */
    writeCommand(0x63);
    writeData((colour & 0xf800) >> 11);
    writeCommand(0x64);
    writeData((colour & 0x07e0) >> 5);
    writeCommand(0x65);
    writeData((colour & 0x001f));
    /* Draw! */
    writeCommand(RA8875_DCR);
    if (filled)
        writeData(RA8875_DCR_CIRCLE_START | RA8875_DCR_FILL);
    else
        writeData(RA8875_DCR_CIRCLE_START | RA8875_DCR_NOFILL);
    /* Wait for the command to finish */
    waitPoll(RA8875_DCR, RA8875_DCR_CIRCLE_STATUS);
}

void printText(uint16_t x, uint16_t y, const char* buffer, uint16_t foreColour, int bgColour, uint8_t scale, uint8_t alignment) {
    textEnlarge(scale);
    switch (alignment) {
        case 1:
            x = x - strlen(buffer)*4*(scale+1);
            break;
        case 2:
            x = x - strlen(buffer)*8*(scale+1);
            break;
    }
    textSetCursor(x, y);
    if(bgColour < 0)
        textTransparent(foreColour);
    else
        textColour(foreColour, bgColour);
    textWrite(buffer);
}

uint16_t colour(uint16_t r, uint16_t g, uint16_t b) {
    g *= 2;
    if(r>=32) r = 31;
    if(g>=64) g = 63;
    if(b>=32) b = 31;
    return (r<<11) + (g<<5) + b;
}

void logo() {
    drawRect(0, 0, 800, 480, RA8875_WHITE, 1);
    int i;
    textMode();
    //char string1[] = "垂直自動儲存科技";
    char string2[] = "Vertical Automated Storage Technology";
    for(i=0;i<=70;i++) {
        int j = 32-32*i/70;
        uint16_t c = colour(j, j, j);
        drawRect(211, 3, 186, 186, c, 1);
        drawRect(403, 3, 186, 186, c, 1);
        drawRect(211, 195, 186, 186, c, 1);
        drawRect(403, 195, 186, 186, c, 1);
        printText(400, 480-i, string2, RA8875_BLACK, RA8875_WHITE, 1, 1);
        drawRect(0, 480-i+32, 800, 1, RA8875_WHITE, 1);
        CyDelay(1);
    }
    for(i=0;i<=144;i++)
        circleHelper(232+i/2, 24+i, 3, RA8875_RED, 1);
    for(i=0;i<=144;i++)
        circleHelper(304+i/2, 168-i, 3, RA8875_RED, 1);
    for(i=0;i<=144;i++)
        circleHelper(424+i/2, 168-i, 3, RA8875_GREEN, 1);
    for(i=0;i<=144;i++)
        circleHelper(496+i/2, 24+i, 3, RA8875_GREEN, 1);
    for(i=0;i<=144;i++)
        circleHelper(376-i, 216, 3, RA8875_WHITE, 1);
    for(i=0;i<=144;i++)
        circleHelper(232+i, 216+i, 3, RA8875_WHITE, 1);
    for(i=0;i<=144;i++)
        circleHelper(376-i, 360, 3, RA8875_WHITE, 1);
    for(i=0;i<=144;i++)
        circleHelper(568-i, 216, 3, RA8875_WHITE, 1);
    for(i=0;i<=144;i++)
        circleHelper(496, 216+i, 3, RA8875_WHITE, 1);
    printText(400, 464, "Copyright \xA9 2019 VAST, Inc. All rights reserved.", RA8875_BLACK, -1, 0, 1);
}

void screen(const char* title, const char* subtitle, const char* bottom, const char* t1, const char* t2, const char* t3, const char* t4, const char* t5, const char* t6, const char* t7, const char* t8, const char* t9, const char* t10, uint16_t selected, int16_t backgroundColour) {
    if(backgroundColour >= 0)
        drawRect(0, 0, 800, 480, backgroundColour, 1);
    printText(400, 0, title, RA8875_WHITE, -1, 1, 1);
    printText(400, 30, subtitle, RA8875_WHITE, -1, 0, 1);
    if(strlen(t1) > 0) {
        drawTriangle(0, 20, 17, 10, 17, 30, selected==1?RA8875_YELLOW:RA8875_WHITE, 1);
        printText(30, 20-17, t1, selected==1?RA8875_YELLOW:RA8875_WHITE, -1, 1, 0);
    }
    if(strlen(t2) > 0) {
        drawTriangle(799, 20, 782, 10, 782, 30, selected==2?RA8875_YELLOW:RA8875_WHITE, 1);
        printText(769, 20-17, t2, selected==2?RA8875_YELLOW:RA8875_WHITE, -1, 1, 2);
    }
    if(strlen(t3) > 0) {
        drawTriangle(0, 130, 17, 120, 17, 140, selected==3?RA8875_YELLOW:RA8875_WHITE, 1);
        printText(30, 130-17, t3, selected==3?RA8875_YELLOW:RA8875_WHITE, -1, 1, 0);
    }
    if(strlen(t4) > 0) {
        drawTriangle(799, 130, 782, 120, 782, 140, selected==4?RA8875_YELLOW:RA8875_WHITE, 1);
        printText(769, 130-17, t4, selected==4?RA8875_YELLOW:RA8875_WHITE, -1, 1, 2);
    }
    if(strlen(t5) > 0) {
        drawTriangle(0, 240, 17, 230, 17, 250, selected==5?RA8875_YELLOW:RA8875_WHITE, 1);
        printText(30, 240-17, t5, selected==5?RA8875_YELLOW:RA8875_WHITE, -1, 1, 0);
    }
    if(strlen(t6) > 0) {
        drawTriangle(799, 240, 782, 230, 782, 250, selected==6?RA8875_YELLOW:RA8875_WHITE, 1);
        printText(769, 240-17, t6, selected==6?RA8875_YELLOW:RA8875_WHITE, -1, 1, 2);
    }
    if(strlen(t7) > 0) {
        drawTriangle(0, 350, 17, 340, 17, 360, selected==7?RA8875_YELLOW:RA8875_WHITE, 1);
        printText(30, 350-17, t7, selected==7?RA8875_YELLOW:RA8875_WHITE, -1, 1, 0);
    }
    if(strlen(t8) > 0) {
        drawTriangle(799, 350, 782, 340, 782, 360, selected==8?RA8875_YELLOW:RA8875_WHITE, 1);
        printText(769, 350-17, t8, selected==8?RA8875_YELLOW:RA8875_WHITE, -1, 1, 2);
    }
    if(strlen(t9) > 0) {
        drawTriangle(0, 460, 17, 450, 17, 470, selected==9?RA8875_YELLOW:RA8875_WHITE, 1);
        printText(30, 460-17, t9, selected==9?RA8875_YELLOW:RA8875_WHITE, -1, 1, 0);
    }
    if(strlen(t10) > 0) {
        drawTriangle(799, 460, 782, 450, 782, 470, selected==10?RA8875_YELLOW:RA8875_WHITE, 1);
        printText(769, 460-17, t10, selected==10?RA8875_YELLOW:RA8875_WHITE, -1, 1, 2);
    }
    printText(400, 464, bottom, RA8875_WHITE, -1, 0, 1);
}