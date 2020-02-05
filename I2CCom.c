/* 
 * File:   I2CCom.c
 * Author: orlan
 *
 * Created on January 20, 2020, 4:22 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "I2CCom.h"

void I2CInit(char operation_mode, char address){
    if(operation_mode == 0){    //Slave mode
        TRISC3 = 1;     //SCL as input 
        TRISC4 = 1;     //SDA as input
        
        SSPSTATbits.SMP = 0;
        SSPSTATbits.CKE = 0;
        
        //There is no need to setup interrupts on start and stop bits because
        //an interrupt will be generated on an address match
        SSPCON1 = 0b00110110;   //reset everything and set I2C slave mode 7-bit address
        SSPCON2 = 0x00;
        
        SSPADD = address << 1; //SSPADD<0> is assumed to be 0, remember it is 7-bit addressing
        
        SSPIF = 0;
        SSPIE = 1;  //Interrupt enable bit
        SSPIP = 1;  //High priority interruption
    }
    else{
        
    }
}

void I2CSend(int input){
    char err = SSPBUF;      //BF bit must be cleared
    
    SSPBUF = input;         //load data
    SSPCON1bits.CKP =  1;   //SCL pin is enabled (clock is released)
    while(SSPSTATbits.BF);  //Wait for buffer to be empty
}

void I2CReceive(char *msg_input){
    
}

void I2CCheckError(void){
    if(SSPSTATbits.BF){
        char err = SSPBUF;      //clear BF bit by reading SSPBUF
    }
    if(SSPCON1bits.SSPOV){
        SSPCON1bits.SSPOV = 0;  //clear error by software
    }
    if(SSPCON1bits.WCOL){
        SSPCON1bits.WCOL = 0; //write collision occured
    }
}
