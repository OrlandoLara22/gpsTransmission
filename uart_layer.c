/*
 * File:   uart.c
 * Author: Orlando Lara Guzman
 *
 * Created on January 14, 2020, 5:29 PM
 * This is a source file for use of UART communication on PIC18F25K80
 */


#include <xc.h>
#include "uart_layer.h"
#include "pic18f25k80.h"

void uartInit(unsigned int brg_reg, unsigned char sync, unsigned char brg16, unsigned char brgh){
    //UART interruption enable
    PIE1bits.TX1IE = 0;
    TX1IP = 0;
    RC1IE = 1;
    RC1IP = 1;
    
    SPBRGH1 = (brg_reg & 0xFF00) >> 8;
    SPBRG1 = brg_reg & 0x00FF;
    
    TXSTA1bits.TX9 = 0;     //8-bit transmission
    TXSTA1bits.TXEN = 1;    //enable transmission
    TXSTA1bits.SYNC = sync; 
    TXSTA1bits.BRGH = brgh;
    
    RCSTA1bits.SPEN = 1;    //serial port is enabled
    RCSTA1bits.RX9 = 0;     //8-bit reception
    RCSTA1bits.CREN = 1;    //enable receiver
    
    BAUDCON1bits.BRG16 = brg16;
}

void uartSend(unsigned char c){
    while(PIR1bits.TX1IF == 0){};
    TXREG1 = c;
}

void uartSendArray(unsigned char *c, unsigned int length){
    for(char n = 0; n < length; n++){
        uartSend(c[n]);
    }
}

void uartSendString(unsigned char *c){
    char n = 0;
    while(c[n] != 0){    //continue until null character (evaluates to 0 in decimal)
        uartSend(c[n]);
        n++;
    }
}

void uartReceive(unsigned char *c, bool *no_errors){
    if(RCSTA1bits.FERR){
        unsigned char error = RCREG1;     //framing error, read register to clear error
    }
    else{
        if(RCSTA1bits.OERR){
            RCSTA1bits.CREN = 0;    //over run error, clear CREN to clear error
            RCSTA1bits.CREN = 1;    //enable receiver
        }
        else{
            *c = RCREG1;
            *no_errors = true;
        }
    }
}