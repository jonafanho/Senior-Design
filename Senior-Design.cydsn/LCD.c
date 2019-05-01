#include "project.h"
#include "LCD.h"
#include "LCD_H.h"

void writeCommand(int command) {
    CS_Write(0);
    SPI_WriteTxData(RA8875_CMDWRITE);
    while(!(SPI_ReadTxStatus() & SPI_STS_SPI_DONE)) {}
    SPI_WriteTxData(command);
    while(!(SPI_ReadTxStatus() & SPI_STS_SPI_DONE)) {}
    CS_Write(1);
}

void writeData(int data) {
    CS_Write(0);
    SPI_WriteTxData(RA8875_DATAWRITE);
    while(!(SPI_ReadTxStatus() & SPI_STS_SPI_DONE)) {}
    SPI_WriteTxData(data);
    while(!(SPI_ReadTxStatus() & SPI_STS_SPI_DONE)) {}
    CS_Write(1);
}

void writeReg(int reg, int val) {
    writeCommand(reg);
    writeData(val);
}

uint8 readData() {
    CS_Write(0);
    SPI_WriteTxData(RA8875_DATAREAD);
    while(!(SPI_ReadTxStatus() & SPI_STS_SPI_DONE)) {}
    SPI_ClearRxBuffer();
    SPI_WriteTxData(0);
    while(!(SPI_ReadTxStatus() & SPI_STS_SPI_DONE)) {}
    uint8 x = SPI_ReadRxData();
    CS_Write(1);
    return x;
}

uint8_t readReg(uint8_t reg) {
  writeCommand(reg);
  return readData();
}

void waitPoll(uint8_t regname, uint8_t waitflag) {
    /* Wait for the command to finish */
    while(1) {
        uint8_t temp = readReg(regname);
        if(!(temp & waitflag))
            return;
    }
}

void PLLinit() {
    writeReg(RA8875_PLLC1, RA8875_PLLC1_PLLDIV1 + 11);
    CyDelay(1);
    writeReg(RA8875_PLLC2, RA8875_PLLC2_DIV4);
    CyDelay(1);
}

void initialize() {
    PLLinit();
    writeReg(RA8875_SYSR, RA8875_SYSR_16BPP | RA8875_SYSR_MCU8);
    /* Timing values */
    uint8_t pixclk = RA8875_PCSR_PDATL | RA8875_PCSR_2CLK;
    uint8_t hsync_start = 32;
    uint8_t hsync_pw = 96;
    uint8_t hsync_finetune = 0;
    uint8_t hsync_nondisp = 26;
    uint8_t vsync_pw = 2;
    uint16_t vsync_nondisp = 32;
    uint16_t vsync_start = 23;
    writeReg(RA8875_PCSR, pixclk);
    CyDelay(1);
    int _height = 480, _width = 800;
    /* Horizontal settings registers */
    writeReg(RA8875_HDWR,(_width / 8) - 1);                          // H width:(HDWR + 1) * 8 = 480
    writeReg(RA8875_HNDFTR, RA8875_HNDFTR_DE_HIGH + hsync_finetune);
    writeReg(RA8875_HNDR,(hsync_nondisp - hsync_finetune - 2)/8);    // H non-display: HNDR * 8 + HNDFTR + 2 = 10
    writeReg(RA8875_HSTR, hsync_start/8 - 1);                         // Hsync start:(HSTR + 1)*8
    writeReg(RA8875_HPWR, RA8875_HPWR_LOW +(hsync_pw/8 - 1));        // HSync pulse width =(HPWR+1) * 8
    /* Vertical settings registers */
    writeReg(RA8875_VDHR0,(uint16_t)(_height - 1) & 0xFF);
    writeReg(RA8875_VDHR1,(uint16_t)(_height - 1) >> 8);
    writeReg(RA8875_VNDR0, vsync_nondisp-1);                          // V non-display period = VNDR + 1
    writeReg(RA8875_VNDR1, vsync_nondisp >> 8);
    writeReg(RA8875_VSTR0, vsync_start-1);                            // Vsync start position = VSTR + 1
    writeReg(RA8875_VSTR1, vsync_start >> 8);
    writeReg(RA8875_VPWR, RA8875_VPWR_LOW + vsync_pw - 1);            // Vsync pulse width = VPWR + 1
    /* Set active window X */
    writeReg(RA8875_HSAW0, 0);                                        // horizontal start point
    writeReg(RA8875_HSAW1, 0);
    writeReg(RA8875_HEAW0,(uint16_t)(_width - 1) & 0xFF);            // horizontal end point
    writeReg(RA8875_HEAW1,(uint16_t)(_width - 1) >> 8);
    /* Set active window Y */
    writeReg(RA8875_VSAW0, 0);                              // vertical start point
    writeReg(RA8875_VSAW1, 0);
    writeReg(RA8875_VEAW0,(uint16_t)(_height - 1) & 0xFF); // vertical end point
    writeReg(RA8875_VEAW1,(uint16_t)(_height - 1) >> 8);
    /* Clear the entire window */
    writeReg(RA8875_MCLR, RA8875_MCLR_START | RA8875_MCLR_FULL);
    CyDelay(500);
}

int begin() {
    CS_Write(1);
    RST_Write(0);
    CyDelay(100);
    RST_Write(1);
    CyDelay(100);
    SPI_Start();
    uint8_t x = readReg(0);
    if(x != 0x75)
        return 0;
    initialize();
    return 1;
}

void setScrollWindow(int16_t x, int16_t y, uint8_t mode) {
    int16_t w = 800, h = 480;
    // Horizontal Start point of Scroll Window
    writeCommand(0x38);
    writeData(x);
    writeCommand(0x39);
    writeData(x>>8);
    // Vertical Start Point of Scroll Window
    writeCommand(0x3a);
    writeData(y);
    writeCommand(0x3b);
    writeData(y>>8);
    // Horizontal End Point of Scroll Window
    writeCommand(0x3c);
    writeData(x+w);
    writeCommand(0x3d);
    writeData((x+w)>>8);
    // Vertical End Point of Scroll Window
    writeCommand(0x3e);
    writeData(y+h);
    writeCommand(0x3f);
    writeData((y+h)>>8);
    // Scroll function setting
    writeCommand(0x52);
    writeData(mode);
}

void circleHelper(int16_t x, int16_t y, int16_t r, uint16_t colour, int filled) {
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
        writeData(RA8875_DCR_CIRCLE_START | RA8875_DCR_FILL);
    else
        writeData(RA8875_DCR_CIRCLE_START | RA8875_DCR_NOFILL);
    /* Wait for the command to finish */
    waitPoll(RA8875_DCR, RA8875_DCR_CIRCLE_STATUS);
}
void graphicsMode() {
    writeCommand(RA8875_MWCR0);
    uint8_t temp = readData();
    temp &= ~RA8875_MWCR0_TXTMODE; // bit #7
    writeData(temp);
}

void textMode() {
    /* Set text mode */
    writeCommand(RA8875_MWCR0);
    uint8_t temp = readData();
    temp |= RA8875_MWCR0_TXTMODE; // Set bit 7
    writeData(temp);
    /* Select the internal(ROM) font */
    writeCommand(0x21);
    temp = readData();
    temp &= ~((1<<7) |(1<<5)); // Clear bits 7 and 5
    writeData(temp);
}

void textSetCursor(uint16_t x, uint16_t y) {
    /* Set cursor location */
    writeCommand(0x2A);
    writeData(x & 0xFF);
    writeCommand(0x2B);
    writeData(x >> 8);
    writeCommand(0x2C);
    writeData(y & 0xFF);
    writeCommand(0x2D);
    writeData(y >> 8);
}

void textColour(uint16_t foreColour, uint16_t bgColour) {
    /* Set Fore Colour */
    writeCommand(0x63);
    writeData((foreColour & 0xf800) >> 11);
    writeCommand(0x64);
    writeData((foreColour & 0x07e0) >> 5);
    writeCommand(0x65);
    writeData((foreColour & 0x001f));
    /* Set Background Colour */
    writeCommand(0x60);
    writeData((bgColour & 0xf800) >> 11);
    writeCommand(0x61);
    writeData((bgColour & 0x07e0) >> 5);
    writeCommand(0x62);
    writeData((bgColour & 0x001f));
    /* Clear transparency flag */
    writeCommand(0x22);
    uint8_t temp = readData();
    temp &= ~(1<<6); // Clear bit 6
    writeData(temp);
}

void textTransparent(uint16_t foreColour) {
    /* Set Fore Colour */
    writeCommand(0x63);
    writeData((foreColour & 0xf800) >> 11);
    writeCommand(0x64);
    writeData((foreColour & 0x07e0) >> 5);
    writeCommand(0x65);
    writeData((foreColour & 0x001f));
    /* Set transparency flag */
    writeCommand(0x22);
    uint8_t temp = readData();
    temp |=(1<<6); // Set bit 6
    writeData(temp);
}

void textWrite(const char* buffer)
{
    uint16_t len = strlen(buffer);
    writeCommand(RA8875_MRWC);
    uint16_t i;
    for(i=0;i<len;i++)
        writeData(buffer[i]);
}

uint8_t SCALE = 0;

void textEnlarge(uint8_t scale) {
    if(scale == SCALE) return;
    if(scale > 3) scale = 3; // highest setting is 3
    SCALE = scale;
    /* Set font size flags */
    writeCommand(0x22);
    uint8_t temp = readData();
    temp &= ~(0xF); // Clears bits 0..3
    temp |= scale << 2;
    temp |= scale;
    writeData(temp);
}