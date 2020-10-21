/* 
 * File:   yunimain.c
 * Author: Slavomír Katkin
 *
 * Created on June 24, 2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xc.h>
#include "simdelay.h"
#include "display.h"
#include "shift.h"
#include "i2c2.h"

#pragma config WDTEN = OFF
#pragma config FOSC = INTIO7
#pragma config MCLRE = EXTMCLR
#pragma config FCMEN = ON

/**
 * This structure represents first 5 registers inside RTC as they are defined in 
 * datasheet, starting from address 0x00 to 0x04. These registers contain all the 
 * data we need for a regular clock.
 */
typedef struct {
	uint8_t controlReg;
	uint8_t milisecReg;              
	uint8_t secondsReg;                   
	uint8_t minutesReg;               
	uint8_t hoursReg;                                              
} _RTC;

_RTC RTC;

/*
 * Values used for the first set up of the clock
 * T = tens  
 * D = digits 
 * so time is interpreted as TD:TD:TD in HH:MM:SS format.
 */
uint8_t hoursT   = 1;
uint8_t hoursD   = 2;
uint8_t minutesT = 0;
uint8_t minutesD = 0;
uint8_t secondsT = 0;
uint8_t secondsD = 0;

/*
 * Mode which specifies display format of the clock (see function display()).
 */
uint8_t mode = 0;

void displayInit() {
    TRISC = 0;
    TRISEbits.RE2 = 0;
    LATEbits.LE2 = 1;
    lcd_init();
}

void rtcInit() {
    TRISDbits.RD0 = 0;  /* serial clock -> output pin */
    TRISDbits.RD1 = 0;  /* serial data  -> output pin */

    LATDbits.LATD0 = 1; /* P_SCL_ON */
    LATDbits.LATD0 = 1; /* P_SDA_ON */

    RTC.controlReg = 0x80;                          /* Set control 32.768kHz */
    RTC.milisecReg = 0;                             /* Set begin time: ms */   
    RTC.secondsReg = (secondsT << 4) | (secondsD);  /* Set begin time: sec */            
    RTC.minutesReg = (minutesT << 4) | (minutesD);  /* Set begin time: min */ 
    RTC.hoursReg   = (hoursT   << 4) | (hoursD);    /* Set begin time: hr */ 
}

void init(){
    OSCCON = (OSCCON & 0b10001111) | 0b01110000;    /* internal oscillator at full speed (16 MHz) */

    TRISB = 0b11111111; /* five buttons in + unused + PGC, PGD */
    LATB  = 0xff;       /* pull-up by default */
    ANSELB = 0;         /* no ADC inputs */
    
    TRISD = 0b00000000; 
    LATD  = 0b00000000;
    ANSELD = 0;
    
    RCONbits.IPEN = 1; //Allow interrupts 
    INTCONbits.GIEL = 1; //Allow low priority interrups 
    
    displayInit();
    rtcInit();
}

/*
 * Function for getting time data from RTC unit
 */
void getTime() {
    I2C_Stop();                           /* Generate stop condition */
    I2C_Set_Address(0,0);                 /* Set RTC address to 0, func. write */
    I2C_Start();
    I2C_Set_Address(0,1);	              /* Set RTC address to 0, func. read */ 
    I2C_Read_Block(5, &RTC.controlReg);   /* Read first 5 byte from RTC and store data */
    I2C_Stop();                           /* Generate stop condition */
}

/*
 * Function for setting time data to RTC unit
 */
void setTime() {
    uint8_t i;
    uint8_t *ptr = &RTC.controlReg;       /* Set temporary pointer */
    I2C_Stop();                           /* Generate stop */
    I2C_Set_Address(0,0);                 /* Set low address and func, write */
    for (i = 0; i < 5; i++) {
	 I2C_Write_B(*ptr);                   /* Write 5 byte to RTC */
	 ptr++;
    }
    I2C_Stop();       
}

/* 
 * This function prints number in binary representation, where
 * 0 -> 'o'
 * 1 -> '*'
 * in 4 bit format, e.g 3 -> oo**
 */
void printBinary(uint8_t number) {
    uint8_t i;
    
    for(i = 0x8; i!= 0; i >>= 1)
        lcd_putchar((number & i) ? '*':'o');
}

/*
 * Display time in selected mode.
 * mode 0 = regular clock; HH:MM:SS
 * mode 1 = binary print; bin minutesT | bin minutesD MM
 *                        bin secondsT | bin secondsD SS
 */
void display() {
    uint8_t tmp = secondsD;
    /* interpret register values as HH:MM:SS */
    hoursT   = (RTC.hoursReg & 0b11110000) >> 4;
    hoursD   = RTC.hoursReg & 0b00001111;
    minutesT = (RTC.minutesReg & 0b11110000) >> 4;
    minutesD = RTC.minutesReg & 0b00001111;
    secondsT = (RTC.secondsReg & 0b11110000) >> 4;
    secondsD = RTC.secondsReg & 0b00001111;

    if(tmp != secondsD) { 
        if(mode) {  
        /* binary mode print */
            /* minutes */
            lcd_clear();
            lcd_goto(0);
            printBinary(minutesT);
            lcd_putchar('|');
            printBinary(minutesD);
            lcd_putchar(' ');
            lcd_putchar('0' + minutesT);
            lcd_putchar('0' + minutesD);
            
            /* seconds */
            lcd_goto(40);
            printBinary(secondsT);
            lcd_putchar('|');
            printBinary(secondsD);
            lcd_putchar(' ');
            lcd_putchar('0' + secondsT);
            lcd_putchar('0' + secondsD);
        } else {
        /* regular clock print */
            lcd_clear();
            lcd_goto(0);
            lcd_putchar('0' + hoursT);
            lcd_putchar('0' + hoursD);
            lcd_putchar(':');
            lcd_putchar('0' + minutesT);
            lcd_putchar('0' + minutesD);
            lcd_putchar(':');
            lcd_putchar('0' + secondsT); 
            lcd_putchar('0' + secondsD);
        }
    }
}

/*
 * Help function for registering button presses
 */
int pressedButton() {
    if(!PORTBbits.RB0) {    
        DelayMs(50);
        if(!PORTBbits.RB0) return 0; /* BTN1 */
    }
    else if(!PORTBbits.RB1) {
        DelayMs(50);
        if(!PORTBbits.RB1) return 1; /* BTN2 */
    }
    else if(!PORTBbits.RB2) {
        DelayMs(50);
        if(!PORTBbits.RB2) return 2; /* BTN3 */
    }
    return -1;                       /* no button */
}

/*
 * Function for setting time on selected position (int pos), with time slot 
 * in register (uint8_t* valueReg) from 0 to maximum value (int max)
 * BTN1 to change value
 * BTN2 to confirm and move to next position
 */
void readAndSet(int pos, uint8_t* valueReg, int max) {
    lcd_goto(pos);
    while(1) {
        DelayMs(100);
        switch(pressedButton()) {
            case 0 :
                *valueReg = (*valueReg + 1) % (max + 1);
                lcd_goto(pos);
                lcd_putchar('0' + *valueReg);
                break;
            case 1 :
                return;
            default :
                break;
        }
    }
}

/*
 * Function for setting time defined by user.
 * BTN2 to start function (only in regular time mode!), control described in
 * function readAndSet.
 */
void clockSetup() {
    if(!mode) {
        int pos = 0;
        while(pos < 8) {
            switch(pos) {
                case 0: 
                    readAndSet(pos, &hoursT, 2);
                    break;
                case 1: 
                    if(hoursT == 2) readAndSet(pos, &hoursD, 3);
                    else            readAndSet(pos, &hoursD, 9);
                    break;
                case 3:
                    readAndSet(pos, &minutesT, 5);
                    break;
                case 4:
                    readAndSet(pos, &minutesD, 9);
                    break;
                case 6:
                    readAndSet(pos, &secondsT, 5);
                    break;    
                case 7:
                    readAndSet(pos, &secondsD, 9);
                    break;
                default:
                    break;
            }
            pos++;
        }
        RTC.secondsReg = (secondsT << 4) | (secondsD);
        RTC.minutesReg = (minutesT << 4) | (minutesD);
        RTC.hoursReg   = (hoursT   << 4) | (hoursD);
        setTime();
        DelayMs(100); 
    } else {
        lcd_clear();
        lcd_goto(0);
        lcd_puts("Cannot set time");
        lcd_goto(40);
        lcd_puts("in binary mode");
        DelayMs(2000); 
    }
}

/*
 * Temporary clock stop.
 * BTN1 to stop time, after pressing BTN1 again time continues from when 
 * it stopped.
 */
void clockStop() {
    while(pressedButton() != 0);
    setTime();
}

void main() {    
    /* PIC, RTC and LCD initialization */
    init();
    /* start the clock */
    setTime();
    RTC.controlReg = 0;
    setTime();
    
    while(1) {
        getTime();
        display();
        switch(pressedButton()) {
            case 0 : /* BTN1 */
                clockStop();
                break;
            case 1 : /* BTN2 */
                clockSetup();
                break;
            case 2 : /* BTN3 */
                mode = ~mode;
                break;
            default :
                break;
        }
    }
}
