/* 
 * File:   I2CCom.h
 * Author: Orlando Lara Guzman
 *
 * Created on January 20, 2020, 4:22 PM
 */

#ifndef I2CCOM_H
#define	I2CCOM_H

#include <xc.h>
#include "pic18f25k80.h"

void I2CInit(char operation_mode, char address);
void I2CSend(int input);
void I2CReceive(char *msg_input);
void I2CCheckError();

#endif	/* I2CCOM_H */

