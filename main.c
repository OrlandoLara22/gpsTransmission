/*
 * File:   main.c
 * Author: Orlando Lara Guzman
 *
 * Created on January 29, 2020, 10:07 AM
 * This is the main file to read information from a GPS sensor via UART and 
 * communicate it to a Jetson via I2C
 */

#pragma config FOSC = HS1       // Oscillator (HS oscillator (Medium power, 4 MHz - 16 MHz))
#pragma config XINST = OFF      // Extended instruction set disabled
#pragma config MCLRE = ON       // Master Clear Enable (MCLR Disabled, RG5 Enabled)
#pragma config SOSCSEL = DIG    // SOSC Power Selection and mode Configuration bits (Digital (SCLKI) mode)

#include <xc.h>
#include <string.h>
#include <stdio.h>
#include "I2CCom.h"
#include "uart_layer.h"
#include "pic18f25k80.h"

#define PIC1_ADDR 0x1B
#define PIC2_ADDR 0x1C
#define PIC3_ADDR 0x1D

typedef struct{
    unsigned char status            : 1;
    unsigned char north_not_south   : 1;
    unsigned char east_not_west     : 1;
    unsigned char unused_bits       : 5;

    unsigned char hour;
    unsigned char minute;
    unsigned char second;
    unsigned int mili_second;
    
    unsigned char day;
    unsigned char month;
    unsigned char year;
    
    unsigned char lat_deg;
    unsigned char lat_minutes;
    unsigned int lat_dec_minutes;   //decimal minutes
    
    unsigned char lon_deg;
    unsigned char lon_minutes;
    unsigned int lon_dec_minutes;   //decimal minutes
}gps_data_read;

gps_data_read gps_data;
unsigned char gps_buffer[17] = {0};
unsigned char input_message[256] = {0};

unsigned char PMTK_SET_RMC_ONLY[] = "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";     //null character is automatically added
unsigned char PMTK_SET_NMEA_UPDATE_10HZ[] = "$PMTK220,100*2F\r\n";  //Set GPS transmission rate to 10hz
unsigned char PMTK_SET_NMEA_UPDATE_5HZ[] = "$PMTK220,200*2C\r\n";   //Set GPS transmission rate to 5hz
unsigned char PMTK_SET_NMEA_UPDATE_2HZ[] = "$PMTK220,500*2B\r\n";   //Set GPS transmission rate to 5hz
unsigned char PMTK_SET_NMEA_UPDATE_1HZ[] = "$PMTK220,1000*1F\r\n";  //Set GPS transmission rate to 1hz

void portSetup(void){
    //Make sure that all unused IO pins are set to output and cleared
    TRISA = 0x00;
    TRISB = 0x01;   //RB0 is input
    TRISC = 0x00;
    
    PORTA = 0x00;
    PORTB = 0x00;
    PORTC = 0x00;
    
    ANSEL10 = 0;    //set RB0 to digital not analog
}

void parseData(unsigned char *str){
    int n = 0, group = 0;
    unsigned char hundreds = 0, tens = 0, ones = 0;
    
    
    while(str[n] != 0)
    {
        if(str[n++] == ','){    //if "," then move to the next group with group++
            switch(++group){
                case 1:
                    tens = (int)str[n++] - 48;
                    ones = (int)str[n++] - 48;
                    gps_data.hour = tens*10 + ones;

                    tens = (int)str[n++] - 48;
                    ones = (int)str[n++] - 48;
                    gps_data.minute = tens*10 + ones;

                    tens = (int)str[n++] - 48;
                    ones = (int)str[n++] - 48;
                    gps_data.second = tens*10 + ones;
                    
                    n++;    //Ignore the decimal point
                    
                    hundreds = (int)str[n++] - 48;
                    tens = (int)str[n++] - 48;
                    ones = (int)str[n++] - 48;
                    
                    gps_data.mili_second = hundreds*100 + tens*10 + ones;
                    break;
                case 2:
                    if(str[n++] == 'A'){
                        gps_data.status = 1;
                    }
                    else
                        gps_data.status = 0;
                    break;
                case 3:
                    tens = (int)str[n++] - 48;
                    ones = (int)str[n++] - 48;
                    gps_data.lat_deg = tens*10 + ones;

                    tens = (int)str[n++] - 48;
                    ones = (int)str[n++] - 48;
                    gps_data.lat_minutes = tens*10 + ones;

                    n++; //Ignore the decimal point

                    tens = (int)str[n++] - 48;
                    ones = (int)str[n++] - 48;
                    gps_data.lat_dec_minutes = (int)(tens*10 + ones)*100;   // This is the hundreds and thousands place
                    tens = (int)str[n++] - 48;
                    ones = (int)str[n++] - 48;
                    gps_data.lat_dec_minutes += (int)(tens*10 + ones);
                    break;
                case 4:
                    if(str[n++] == 'N'){
                        gps_data.north_not_south = 1;
                    }
                    else
                        gps_data.north_not_south = 0;
                    break;
                case 5:
                    hundreds = (int)str[n++] - 48;
                    tens = (int)str[n++] - 48;
                    ones = (int)str[n++] - 48;
                    gps_data.lon_deg = hundreds*100 + tens*10 + ones;

                    tens = (int)str[n++] - 48;
                    ones = (int)str[n++] - 48;
                    gps_data.lon_minutes = tens*10 + ones;

                    n++; // Ignore the decimal point

                    tens = (int)str[n++] - 48;
                    ones = (int)str[n++] - 48;
                    gps_data.lon_dec_minutes = (int)(tens*10 + ones)*100;   // This is the hundreds and thousands place
                    tens = (int)str[n++] - 48;
                    ones = (int)str[n++] - 48;
                    gps_data.lon_dec_minutes += (int)(tens*10 + ones);
                    break;
                case 6:
                    if(str[n++] == 'E'){
                        gps_data.east_not_west = 1;
                    }
                    else
                        gps_data.east_not_west = 0;
                    break;
                case 9:
                    tens = (int)str[n++] - 48;
                    ones = (int)str[n++] - 48;
                    gps_data.day = tens*10 + ones;

                    tens = (int)str[n++] - 48;
                    ones = (int)str[n++] - 48;
                    gps_data.month = tens*10 + ones;

                    tens = (int)str[n++] - 48;
                    ones = (int)str[n++] - 48;
                    gps_data.year = tens*10 + ones;
                    break;
            }
        }
    }
    
    if(SSPSTATbits.P){      //I2C, stop bit was detected last, Last message has finished sending
        memcpy(gps_buffer,&gps_data,sizeof(gps_data));
        memset(&gps_data,0,sizeof(gps_data));   //Clear all information in data structure
    }
}

void __interrupt(high_priority) isr_high(void){  
    
    if(PIE1bits.RC1IE && PIR1bits.RC1IF){
        static unsigned char n;     //initialized with 0 even if not explicitly
        static bool save_input = false;
        unsigned char uart_data = 0;
        bool no_errors = false;
        
        uartReceive(&uart_data, &no_errors);
        
        if(no_errors){
            switch(uart_data){
                case '$':
                    if(!save_input){
                        save_input = true;
                        n = 0;
                        input_message[n++] = uart_data;
                    }
                    break;
                case '\n':
                    if(save_input){
                        input_message[n++] = uart_data;
                        parseData(input_message);

                        memset(input_message, 0, sizeof(input_message));    //clear UART buffer
                        save_input = false;
                    }
                    break;
                default:
                    if(save_input){
                        input_message[n++] = uart_data;
                    }
                    break;
            }
        }
    }
    
    if(SSPIE && SSPIF){
        static unsigned char n;
        static unsigned char i;
        SSPIF = 0;  //reset flag
        I2CCheckError();
        
        if(!SSPSTATbits.D_NOT_A && SSPSTATbits.R_NOT_W){                        //If last byte was address and read
            n = 0;
            I2CSend(gps_buffer[n++]);
        }
        else if(SSPSTATbits.D_NOT_A && SSPSTATbits.R_NOT_W && SSPSTATbits.S){   //if last byte was data and read, and start bit was detected last    
            I2CSend(gps_buffer[n++]);
        }
    }
}

void main(void) {
    unsigned char message = 0;
    bool button_held = false;
    IPEN = 1;       //Enable interrupt priorities
    GIEH = 1;       //Enable High priority interruptions
    
    portSetup();
    I2CInit(0, PIC1_ADDR);
    uartInit(25,0,0,0);
    
    memset(&gps_data,0,sizeof(gps_data));   //Clear all information in data structure
    while(1){
         if(PORTBbits.RB0 == 1 && !button_held){
             button_held = true;
             switch(message++){
                 case 0:
                     uartSendString(PMTK_SET_NMEA_UPDATE_10HZ);
                     //uartSendString(PMTK_SET_RMC_ONLY);
                     break;
                 case 1:
                     uartSendString(PMTK_SET_NMEA_UPDATE_5HZ);
                     //uartSendString(PMTK_SET_RMC_ONLY);
                     break;
                 case 2:
                     uartSendString(PMTK_SET_NMEA_UPDATE_1HZ);
                     //uartSendString(PMTK_SET_RMC_ONLY);
                     break;
             }
             if(message >= 3){
                 message = 0;
             }
        }
         else if(PORTBbits.RB0 == 0){
             button_held = false;
         }
         if(button_held){
             RC2 = 1;   //LED OFF
         }
         else
             RC2 = 0;   //LED ON
    }
}

