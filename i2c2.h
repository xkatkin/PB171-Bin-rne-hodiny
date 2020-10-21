#ifndef _I2C_H
#define _I2C_H

#include "avr/io.h"
#include "shift.h"

void I2C_Set_Address(uint8_t, uint8_t);		/* Set I2C low address and type of operation */
void I2C_Start(void);				/* Generate start condition */
void I2C_Stop(void);				/* Generate stop condition */
void I2C_Write_B (uint8_t);			/* Write one byte to temporary register */
uint8_t I2C_Read_B (uint8_t);			/* Read one byte from I2C */
uint8_t I2C_Ack_In(void);			/* Generate ACK pulse for slave present testing */
void I2C_NoAck_Out(void);			/* Generate NON ACK for slave to stop next reading */
void I2C_Ack_Out(void);				/* Generate ACK for slave for next reading */ 
void I2C_Write_Block_W(uint16_t *);		/* Write block of word to I2C */
void I2C_Read_Block(uint8_t , uint8_t *);	/* Read block from I2C */
void I2C_Wait(void);                            /* Wait for I2C */
#endif
