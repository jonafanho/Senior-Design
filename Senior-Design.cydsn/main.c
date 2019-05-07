/* ========================================
 *
 * Copyright Jonathan Ho
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * ========================================
*/
#include "project.h"
#include "LCD_H.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

void resetPins(int resetX) {
    a1_Write(0);
    b1_Write(0);
    a2_Write(0);
    b2_Write(0);
    CyDelay(1);
    if(resetX) clkX_Write(1);
    clkY_Write(1);
    clkZ_Write(1);
    CyDelay(1);
    if(resetX) clkX_Write(0);
    clkY_Write(0);
    clkZ_Write(0);
}

int getSign(int a) {
    if(a == 0)
        return 0;
    return abs(a)/a;
}

const int stepA1[] = { 1, 0, 0, 0 };
const int stepB1[] = { 0, 1, 0, 0 };
const int stepA2[] = { 0, 0, 1, 0 };
const int stepB2[] = { 0, 0, 0, 1 };

void stepMotor(int stepsX, int stepsY, int stepsZ) {
    if(stepsX!=0) { // x motor
        int arrayStep = stepsX<0 ? 3-(abs(stepsX)%4) : stepsX%4;
        a1_Write(stepA1[arrayStep]);
        b1_Write(stepB1[arrayStep]);
        a2_Write(stepA2[arrayStep]);
        b2_Write(stepB2[arrayStep]);
        CyDelayUs(1);
        clkX_Write(1);
        CyDelayUs(1);
        clkX_Write(0);
        CyDelayUs(1);
    }
    if(stepsY!=0) { // y motor
        int arrayStep = stepsY<0 ? 3-(abs(stepsY)%4) : stepsY%4;
        a1_Write(stepA1[arrayStep]);
        b1_Write(stepB1[arrayStep]);
        a2_Write(stepA2[arrayStep]);
        b2_Write(stepB2[arrayStep]);
        CyDelayUs(1);
        clkY_Write(1);
        CyDelayUs(1);
        clkY_Write(0);
        CyDelayUs(1);
    }
    if(stepsZ!=0) { // z motor
        int arrayStep = stepsZ<0 ? 3-(abs(stepsZ)%4) : stepsZ%4;
        a1_Write(stepA1[arrayStep]);
        b1_Write(stepB1[arrayStep]);
        a2_Write(stepA2[arrayStep]);
        b2_Write(stepB2[arrayStep]);
        CyDelayUs(1);
        clkZ_Write(1);
        CyDelayUs(1);
        clkZ_Write(0);
        CyDelayUs(1);
    }
}

void delayExponential(int totalSteps, int startSpeed, int minSpeed, int currentStep) {
    int slope = 5;
    int ramp =(startSpeed-minSpeed)/slope;
    if((currentStep > ramp) &&(currentStep <(totalSteps - ramp)))
        CyDelayUs(minSpeed);
    else
        if(currentStep <= ramp)
            CyDelayUs(-currentStep*slope+startSpeed);
        else
            CyDelayUs((currentStep-(totalSteps-ramp))*slope+minSpeed);
}

int readButtons() {
    // 0  9  7  5 15 13  4  3  8 11  2
    // 0  1  2  3  4  5  6  7  8  9  10
    int map[16] = { 0, 0, 10, 7, 6, 3, 0, 2, 8, 1, 0, 9, 0, 5, 0, 4 };
    int button1 = 0, button2 = 0;
    if(Button0_Read()) button1 += 1;
    if(Button1_Read()) button1 += 2;
    if(Button2_Read()) button1 += 4;
    if(Button3_Read()) button1 += 8;
    CyDelay(100);
    if(Button0_Read()) button2 += 1;
    if(Button1_Read()) button2 += 2;
    if(Button2_Read()) button2 += 4;
    if(Button3_Read()) button2 += 8;
    if(button1 == button2) return map[button1];
    else return 0;
}

int readForce() {
    int i;
    int count=0;
    while(ForceData_Read());
    for(i=0;i<24;i++){
        ForceClock_Write(1);
        count = count<<1;
        ForceClock_Write(0);
        if(ForceData_Read()) count++;
    }
    ForceClock_Write(1);
    count = count^0x800000;
    ForceClock_Write(0);
    return round(-0.0000948 * count + 803.08026);
}

const int THRESHOLD = 650;

void moveMotor(int stepsX, int stepsY, int stepsZ, int maxSpeed, int sensorAction) {
    int steps = abs(stepsX);
    if(abs(stepsY) > steps)
        steps = abs(stepsY);
    if(abs(stepsZ) > steps)
        steps = abs(stepsZ);
    int i, j=0, irOffset=0;
    for(i=0;i<steps;i++) {
        stepMotor(i<abs(stepsX)*2?i*getSign(stepsX)/2:0,i<abs(stepsY)?i*getSign(stepsY):0,i<abs(stepsZ)?i*getSign(stepsZ):0);
//        while(IR_Read()) {
//            resetPins();
//            irOffset=i;
//            //CyDelay(2000);
//        }
        int photoresistor = ADC_GetResult16(0);
        if((sensorAction==-1&&photoresistor<THRESHOLD) ||
           (sensorAction==1&&photoresistor>THRESHOLD)) {
            j++;
            const int k = sensorAction==-1?2000:600;
            if(j >= k || sensorAction == -1) break;
        } else {
            j = 0;
        }
        delayExponential(steps-irOffset, 2000, maxSpeed, i-irOffset);
    }
    resetPins(0);
}

const int X_SPEED = 900;
const int Z_SPEED = 360;
const int Z_STEPS = 43500;

void moveMotorManual() {
    int buttons;
    LED_R_Write(0);
    drawRect(0, 0, 800, 480, colour(0,0,4), 1);
    int i = 1;
    int speed = 900;
    int percentage = 100;
    while(1) {
        buttons = readButtons();
        if(i == 1 || buttons != 0)
            screen("Manual Mode", "For advanced users only!", "Press the emergency stop button to exit.",
                "X left", "X right", "Y up", "Z out", "Y down", "Z in",
                "Move more", "Faster", "Move less", "Slower",
                buttons, -1);
        if(buttons == 0) { CyDelay(10); resetPins(1); i = 0; }
        else i = 1;
        if(buttons == 1) moveMotor(2000,0,0,X_SPEED,0);
        if(buttons == 2) moveMotor(-2000,0,0,X_SPEED,0);
        if(buttons == 3) moveMotor(0,percentage==0?100:220*percentage,0,speed,0); // 500 no load
        if(buttons == 5) moveMotor(0,percentage==0?-100:-220*percentage,0,speed,0);
        if(buttons == 4) moveMotor(0,0,Z_STEPS*percentage/100,speed,0); // 320 for no load
        if(buttons == 6) moveMotor(0,0,-Z_STEPS*percentage/100,speed,0);
        if(buttons == 7) percentage += 5;
        if(buttons == 9 && percentage > 0) percentage -= 5;
        if(buttons == 8) speed -= 20;
        if(buttons == 10) speed += 20;
        char string[256];
        sprintf(string, " Movement: %d%% ", percentage);
        printText(400, 150, string, RA8875_WHITE, colour(0,0,4), 1, 1);
        sprintf(string, " Speed: %d ", speed);
        printText(400, 200, string, RA8875_WHITE, colour(0,0,4), 1, 1);
        int photoresistor = ADC_GetResult16(0);
        sprintf(string, " Photoresistor: %d ", photoresistor);
        printText(400, 250, string, RA8875_WHITE, colour(0,0,4), 1, 1);
        int forceSensor = readForce();
        sprintf(string, "     Weight: %d kg (%d lb)     ", (int)round(forceSensor*0.453592), forceSensor);
        printText(400, 300, string, RA8875_WHITE, colour(0,0,4), 1, 1);
    }
}

const int Y_UP[] = { 540, 700, 1000 };
const int Y_DOWN[] = { 580, 580, 580 };

void moveToBin(int start, int end) {
    const int y =(start-1)/2 -(end-1)/2;
    const int tempEnd = end%2==0&&y!=0 ? end-1 : end;
    const int x = start%2 - tempEnd%2;
    const int hasBin = (readForce() > -3) + (readForce() > 15);
    const int yUp = Y_UP[hasBin];
    const int yDown = Y_DOWN[hasBin];
    drawRect(0, 0, 800, 480, colour(4, 2, 0), 1);
    char string[256];
    sprintf(string, "Moving to bin %d", end);
    printText(400, 100, string, RA8875_WHITE, -1, 1, 1);
    printText(400, 150, "Beware of moving parts.", RA8875_WHITE, -1, 1, 1);
    sprintf(string, "Vertical speed: %d", y>0?yUp:yDown);
    printText(400, 250, string, RA8875_WHITE, -1, 1, 1);
    printText(400, 464, "Spam the emergency stop button to exit.", RA8875_WHITE, -1, 0, 1);
    // move short of target
    // offset x if only vertical movement, offset y if only horizontal movement
    moveMotor(x==0?-200:-x*1900,y==0?2000:y*22000-4000*getSign(y),0,y>=0?yUp:yDown,0);
    // move y to target until white
    moveMotor(tempEnd!=end?-1700:0,99999*getSign(y),0,y>0?yUp:yDown,1);
    // calibrate to white if no vertical movement
    if(y==0) moveMotor(0,-99999,0,yDown,1);
    // move x to target until black
    moveMotor(-9999*getSign(x),0,0,X_SPEED,-1);
    // calibrate to black
    if(x==0) {
        if(tempEnd!=end) moveMotor(-9999,0,0,1200,-1);
        else moveMotor(9999,0,0,1200,-1);
    }
}

const int stepsAbove[] = { 0, 700, 700, 500, 500, 600, 600, 400, 400, 400, 400 };
const int stepsBelow[] = { 0, 400, 400, 400, 400, 400, 400, 500, 500, 400, 400 };

int main() {
    CyGlobalIntEnable;
    LED_R_Write(1);
    ForceClock_Write(0);
    resetPins(1);
    ADC_Start();
    ADC_StartConvert();
    init();
    logo();
    CyDelay(100);
    screen("Which bin are you at?", "The system needs to be initalized.",
        "Copyright \xA9 2019 VAST, Inc. All rights reserved.",
        "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", 0, colour(4,0,0));
    int bin = 0;
    while(bin == 0) {
        bin = readButtons();
        if(!Button_Read()) moveMotorManual();
    }
    screen("Which bin are you at?", "The system needs to be initalized.",
        "Copyright \xA9 2019 VAST, Inc. All rights reserved.",
        "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", bin, -1);
    while(readButtons() != 0) {}
    while(1) {
        // no bin
        screen("Vertical Automated Storage Technology", "Tyler, you thought the screen didn't work, did you?",
            "Copyright \xA9 2019 VAST, Inc. All rights reserved.",
            "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", 0, RA8875_BLACK);
        char string[256];
        sprintf(string, "    Currently at bin %d    ", bin);
        printText(400, 200, string, RA8875_GREEN, RA8875_BLACK, 1, 1);
        printText(400, 150, "Select a bin to retrieve.", RA8875_RED, RA8875_BLACK, 1, 1);
        int button = 0;
        resetPins(1);
        // wait for button press
        while(button == 0) {
            button = readButtons();
            if(!Button_Read()) moveMotorManual();
        }
        // check that there is no bin
        while(readForce() > -3) {
            drawRect(0, 0, 800, 480, RA8875_RED, 1);
            CyDelay(100);
            printText(400, 100, "Retrieving bin!", RA8875_WHITE, -1, 1, 1);
            printText(400, 200, "Please remove all items from the platform.", RA8875_WHITE, -1, 1, 1);
            CyDelay(400);
        }
        // move to selected bin and retrieve bin
        moveToBin(bin, button);
        // move down a little
        moveMotor(0,-stepsBelow[button],0,Y_DOWN[0],0);
        // extend z
        moveMotor(0,0,Z_STEPS,Z_SPEED,0);
        // move back up a little
        moveMotor(0,stepsAbove[button]+stepsBelow[button],0,Y_UP[2],0);
        int weight = readForce() > 10;
        if(weight)
            moveMotor(0,300,0,Y_UP[2],0);
        // retract z
        moveMotor(0,0,-Z_STEPS,Z_SPEED,0);
        // move down a little
        moveMotor(0,-stepsAbove[button],0,Y_DOWN[0],0);
        if(weight)
            moveMotor(0,-300,0,Y_UP[2],0);
        moveToBin(button, 7);
        moveMotor(-200,0,0,X_SPEED,0);
        // with bin
        sprintf(string, "%d", button);
        screen("Vertical Automated Storage Technology", "Tyler, you thought the screen didn't work, did you?",
            "Copyright \xA9 2019 VAST, Inc. All rights reserved.",
            string, string, string, string, string, string, string, string, string, string, 0, RA8875_BLACK);
        sprintf(string, "Press any button to replace bin %d", button);
        printText(400, 200, string, RA8875_GREEN, -1, 1, 1);
        resetPins(1);
        // wait for button
        while(readButtons() == 0) {
            if(!Button_Read()) moveMotorManual();
            sprintf(string, "             No load             ");
            int forceSensor = readForce();
            if(forceSensor >= 0)
                sprintf(string, "     Weight: %d kg (%d lb)     ", (int)round(forceSensor*0.453592), forceSensor);
            printText(400, 150, string, RA8875_RED, RA8875_BLACK, 1, 1);
            if(forceSensor > 40)
                printText(400, 100, "Overloaded!", RA8875_RED, RA8875_BLACK, 1, 1);
            else
                printText(400, 100, "             ", RA8875_RED, RA8875_BLACK, 1, 1);
        }
        // check for overload and bin presence
        while(readForce() > 40 || readForce() < -2) {
            drawRect(0, 0, 800, 480, RA8875_RED, 1);
            CyDelay(100);
            printText(400, 100, readForce()>40?"Overloaded!":"Replacing bin!",
                RA8875_WHITE, -1, 1, 1);
            printText(400, 200, readForce()>40?"Please take stuff out of the bin.":"Please place a bin on the platform.",
                RA8875_WHITE, -1, 1, 1);
            CyDelay(400);
        }
        moveMotor(200,0,0,X_SPEED,0);
        // move to selected bin and replace bin
        moveToBin(7, button);
        // move up a little
        moveMotor(0,stepsAbove[button],0,Y_UP[2],0);
        weight = readForce() > 20;
        // extend z
        moveMotor(0,0,Z_STEPS,Z_SPEED,0);
        // move back down a little
        moveMotor(0,-stepsAbove[button]-stepsBelow[button],0,Y_DOWN[0],0);
        // retract z
        moveMotor(0,0,-Z_STEPS,Z_SPEED,0);
        // move up a little
        moveMotor(0,stepsBelow[button],0,Y_UP[2],0);
        moveToBin(button, 5);
        bin = 5;
    }
}