/*
 * File:   uart.h
 * Author: Orlando Lara Guzman
 *
 * Created on January 14, 2020, 5:22 PM
 * This is a header file for use of UART communication on PIC18F25K80
 */

#ifndef UART_LAYER_H
#define UART_LAYER_H

#include <xc.h>
#include "pic18f25k80.h"
#include <stdint.h>
#include <stdbool.h>

void uartInit(unsigned int brg_reg, unsigned char sync, unsigned char brg16, unsigned char brgh);
void uartSend(unsigned char c);
void uartSendArray(unsigned char *c, unsigned int length);
void uartSendString(unsigned char *c);
void uartReceive(unsigned char *c, bool *no_errors);

#endif