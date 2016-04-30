/* 
 * File:   SysRiego.c
 * Author: antoniohernandez
 *
 * Created on April 23, 2016, 4:31 PM
 */
#include <xc.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * 
 */


#pragma config	OSC = IRCIO67       //CONFIG1H (0-3) = 0010: INTIO2 oscillator, Internal oscillator block, port function on RA6 and RA7.
#pragma config	MCLRE = ON          // CONFIG3H.7 = 1: Pin de RESET habilitado y Entrada RE3 desactivado.
#pragma config	PBADEN = OFF        // CONFIG3H.1 = 0: PORTB.0 -- PORTB.4 as Digital I/O.
#pragma config	LVP = OFF           // CONFIG3H.2 = 0: Single-Supply ICSP disabled  so that PORTB.5 works as Digital I/O.

//PICIT-3 DEBUGGER SETUP CONFIGURATION BITS
//=========================================
// declaracion de los metodos
#pragma config WDT = OFF        // CONFIG2H (0) = 0: Watchdog Timer Disabled.

unsigned char oneShotFlag = 0;          //  One-shot flag.

//rows of the keyboard matrix
#define row1 PORTBbits.RB0
#define row2 PORTBbits.RB1
#define row3 PORTBbits.RB2
#define row4 PORTBbits.RB3

//columns of the keyboard matrix
#define col1 PORTBbits.RB4
#define col2 PORTBbits.RB5
#define col3 PORTBbits.RB6
#define col4 PORTBbits.RB7

//special bits for the LCD
#define registerSelectBit PORTCbits.RC0
#define readWriteBit PORTCbits.RC1
#define enableBit PORTCbits.RC2

//portD bits used for transferring data between LCD and microcontroller
#define DB0 PORTDbits.RD0
#define DB1 PORTDbits.RD1
#define DB2 PORTDbits.RD2
#define DB3 PORTDbits.RD3
#define DB4 PORTDbits.RD4
#define DB5 PORTDbits.RD5
#define DB6 PORTDbits.RD6
#define DB7 PORTDbits.RD7

#define LED1 PORTCbits.RC0
#define LED2 PORTCbits.RC1
#define LED3 PORTCbits.RC2
#define LED4 PORTCbits.RC3
#define LED5 PORTCbits.RC4
#define LED6 PORTCbits.RC5
#define LED7 PORTCbits.RC6
#define LED8 PORTCbits.RC7

//get the length of an array
#define arrayLength(array) (unsigned char) (sizeof(array) / sizeof((array)[0]))

//constants
unsigned char GREETING[] = "Hello World";
unsigned char CASTELLO[] = "Castello <3";

unsigned char columnSelected;
unsigned char rowSelected;
unsigned char keyCode;


void modeSelection();
void inicioManual();
void inicioAuto();
void inicioTimer();
void configTimer();
void workingManual(int zona);
void workingAuto(int zona);
void workingTimer(int zona);
void oneShot();
void resetOneShot();
void delay();
void turnOffAllLeds();
void groundAllRows();
void determinePressedRow();
unsigned char getKeyCode();
void pressedKey();
void pressedKeyAction(unsigned char x);
void portDAsInput();
void portDAsOutput();
void LCDSetup();
void shortDelay();
void longDelay();
void command (unsigned char x);
void isReady();
void displayCharOnLCD(unsigned char character);
void displayLineOnLCD(unsigned char array[], unsigned char sizeOfArray);
void interrupt low_priority ISRL();
void interrupt ISRH();


 
void main() {
    //CLOCK FREQUENCY CONFIGURATION
    //============================
    OSCCON = 0x60;             // 4 MHz internal oscillator

    //DISABLE PORT's ANALOG FUNCTIONS
    //===============================
    CMCON = 0x07;              // Comparators OFF, to use PORT_Ds LSN
    CVRCONbits.CVREN = 0;      // Comparator Voltge Reference Module OFF
    ADCON1 = 0x0F;             // All ports as DIGITAL I/O




    PORTB = 0X00;   //Clear PORTB
    LATB = 0X00;    //Clear LATB
    TRISB = 0XFF;   //Set PORTB as input 

    PORTC = 0X00;   //Clear PORTC
    LATC = 0X00;    //Clear LATC
    TRISC = 0X00;   //Set PORTC as output

    //LSb of PORTB as outputs
    TRISBbits.RB0 = 0;
    TRISBbits.RB1 = 0;
    TRISBbits.RB2 = 0;
    TRISBbits.RB3 = 0;
    
    //Interrupt Priority Enable Bit
    RCONbits.IPEN = 1;
    
    //Global Interrupt Enable Bit for High Interrupts
    INTCONbits.GIEH = 1;
    
    //Global Interrupt Enable Bit for Low Interrupts
    INTCONbits.GIEL = 1;
    
    //Activate PORTB's pull up resistors
    INTCON2bits.RBPU = 0;
    
    //Change State Interrupt Enable Bit
    INTCONbits.RBIE = 1;
    
    //Change Interrupt Flag Bit
    INTCONbits.RBIF = 0;
    
    //Change State Interrupt Priority Bit. When set to 1 is High priority
    INTCON2bits.RBIP = 1;
    //set the portD as output
    portDAsOutput();
    
    //initial LCD Setup
    LCDSetup();
    
    //infinite loop to constantly be waiting for user input on the keyboard matrix
    while(1){
//      groundAllRows();  //ground all rows so we can detect if a button is pressed
//  
//      if(PORTB != 0xF0){    //if PORTB is different from 0xF0, a button is being pressed, we must implement oneShot
//          oneShot();
//      }
//      if(oneShotFlag == 1){ //if the oneShotFlag == 1, reset the oneShotFlag
//          resetOneShot();
//      }
    }

 
}
void portDAsInput(){
    TRISD = 0xFF;
}

void portDAsOutput(){
    TRISD = 0x00;
}

void LCDSetup(){
    enableBit = 0;
    longDelay();
    command(0x38);
    longDelay();
    command(0x0F);
    isReady();
    command(0x01);
    isReady();
}

//450 ns delay
void shortDelay(){
    unsigned char L1REG = 2;
    while(L1REG-- > 0){}
}

//250 ms delay
void longDelay(){
    unsigned char L1REG = 2;
    unsigned char L2REG = 162;
    unsigned char L3REG = 255;
    
    while(L1REG-- > 0){
        L2REG = 162;
        while(L2REG-- > 0){
            L3REG = 255;
            while(L3REG-- > 0){}
        }
    }
}

void command(unsigned char x){
    PORTD = x;
    registerSelectBit = 0;
    readWriteBit = 0;
    enableBit = 1;
    shortDelay();
    enableBit = 0;
}
void isReady(){
    portDAsInput();
    registerSelectBit = 0;
    readWriteBit = 1;
    
    while(DB7 == 1){
        enableBit = 1;
        shortDelay();
        enableBit = 0;
    }
    portDAsOutput();   
}

void displayCharOnLCD(unsigned char character){
    PORTD = character;
    registerSelectBit = 1;
    readWriteBit = 0;
    enableBit = 1;
    shortDelay();
    enableBit = 0;
}

void groundAllRows(){    
    row1 = 0;
    row2 = 0;
    row3 = 0;
    row4 = 0;
}

void oneShot(){     
    if (oneShotFlag == 1)    //IF One-Shot has been triggered THEN EXIT.
        return;
    delay();                 //Eliminate Bouncing.
    if (PORTB  != 0xF0){     //IF a button is still pressed
        pressedKey();       //a key was pressed, determine which key and act accordingly
        oneShotFlag = 1;            //Turn-ON one-shot flag.
    }
}

void resetOneShot(){
    if (PORTB  == 0xF0)      //IF RB0 OFF THEN Eliminate Bouncing
        delay();
    else                    //ELSE EXIT
        return;
    if (PORTB  == 0XF0){      //IF RB0 OFF THEN reset one-shot flag
        //turnOffAllLeds();
        INTCONbits.RBIF=0;
        oneShotFlag = 0;
    }
        
}

//10 ms delay
void delay(){
    unsigned char L1REG = 0;
    unsigned char L2REG;
    while(L1REG++ < 13){
        L2REG = 0;
        while(L2REG++ < 255);
    }
}

void turnOnAllRows(){
    row1 = 1;
    row2 = 1;
    row3 = 1;
    row4 = 1;
}


void determinePressedRow(){
    char i;
    for(i=0; i<4; i++){
        turnOnAllRows();
        switch(i){
            case 0: {
                
                row1 = 0;
                
                if( (columnSelected == 1) && (col1 == 0)){
                    rowSelected = 1;
                    return;
                }
                if( (columnSelected == 2) && (col2 == 0)){
                    rowSelected = 1;
                    return;
                }
                if( (columnSelected == 3) && (col3 == 0)){
                    rowSelected = 1;
                    return;
                }
                if( (columnSelected == 4) && (col4 == 0)){
                    rowSelected = 1;
                    return;
                }
            }break;
            case 1: {
                row2 = 0;
                if( (columnSelected == 1) && (col1 == 0)){
                    rowSelected = 2;
                    return;
                }
                if( (columnSelected == 2) && (col2 == 0)){
                    rowSelected = 2;
                    return;
                }
                if( (columnSelected == 3) && (col3 == 0)){
                    rowSelected = 2;
                    return;
                }
                if( (columnSelected == 4) && (col4 == 0)){
                    rowSelected = 2;
                    return;
                }
            }break;
            case 2: {
                row3 = 0;
                if( (columnSelected == 1) && (col1 == 0)){
                    rowSelected = 3;
                    return;
                }
                if( (columnSelected == 2) && (col2 == 0)){
                    rowSelected = 3;
                    return;
                }
                if( (columnSelected == 3) && (col3 == 0)){
                    rowSelected = 3;
                    return;
                }
                if( (columnSelected == 4) && (col4 == 0)){
                    rowSelected = 3;
                    return;
                }
            }break;
            case 3: {
                
                row4 = 0;
                if( (columnSelected == 1) && (col1 == 0)){
                    rowSelected = 4;
                    return;
                }
                if( (columnSelected == 2) && (col2 == 0)){
                    rowSelected = 4;
                    return;
                }
                if( (columnSelected == 3) && (col3 == 0)){
                    rowSelected = 4;
                    return;
                }
                if( (columnSelected == 4) && (col4 == 0)){
                    rowSelected = 4;
                    return;
                }
            }break;
        }
        
    }    
}

unsigned char getKeyCode(){
    if(rowSelected == 1){
        switch(columnSelected){
            case 1: return '1';
            case 2: return '2';
            case 3: return '3';
            case 4: return 'A';
        }
    }
    else if(rowSelected == 2){
        switch(columnSelected){
            case 1: return '4';
            case 2: return '5';
            case 3: return '6';
            case 4: return 'B';
        }
    }
    else if(rowSelected == 3){
        switch(columnSelected){
            case 1: return '7';
            case 2: return '8';
            case 3: return '9';
            case 4: return 'C';
        }
    }
    else if(rowSelected == 4){
        switch(columnSelected){
            case 1: return '*';
            case 2: return '0';
            case 3: return '#';
            case 4: return 'D';
        }
    }
    return -1;
}

void pressedKeyAction(unsigned char keyCode){
    /*turnOffAllLeds();
    switch(keyCode){
        case '0': LED5 = 1;break;
        case '1': LED1 = 1; break;
        case '2': LED2 = 1; break;
        case '3': LED2 = 1;LED1 = 1; break;
        case '4': LED3 = 1; break;
        case '5': LED1 = 1;LED3 = 1; break;
        case '6': LED3 = 1;LED2 = 1; break;
        case '7': LED3 = 1;LED2 = 1;LED1 = 1; break;
        case '8': LED4 = 1; break;
        case '9': LED4 = 1;LED1 = 1;break;
        case 'A': LED4 = 1;LED2 = 1; break;
        case 'B': LED4 = 1;LED1 = 1;LED2 = 1; break;
        case 'C': LED4 = 1;LED3 = 1;break;
        case 'D': LED4 = 1;LED1 = 1;LED3 = 1; break;
        case '*': LED4 = 1; LED2 = 1; LED3 = 1;break;
        case '#': LED4 = 1; LED1 = 1;LED2 = 1; LED3 = 1;break;
        default: turnOffAllLeds();*/
       switch(keyCode){
        case '*': displayLineOnLCD(GREETING,sizeof(GREETING) / sizeof(GREETING[0])); break;
        case '#': displayLineOnLCD(CASTELLO,sizeof(CASTELLO) / sizeof(CASTELLO[0])); break;
        default: displayCharOnLCD(keyCode);
    

    }
}
void turnOffAllLeds(){
    LED1 = 0;
    LED2 = 0;
    LED3 = 0;
    LED4 = 0;
    LED5 = 0;
    LED6 = 0;
    LED7 = 0;
    LED8 = 0;
}

void pressedKey(){
    switch(PORTB){      //obtain the selected column
            case 0xE0: columnSelected = 1; break;
            case 0xD0: columnSelected = 2; break;
            case 0xB0: columnSelected = 3; break;
            case 0x70: columnSelected = 4; break;
        default: columnSelected = 100;
        }
        determinePressedRow();
        keyCode = getKeyCode();
        pressedKeyAction(keyCode);
    //pressedKeyAction(columnSelected);
}

void displayLineOnLCD(unsigned char array[], unsigned char sizeOfArray){
    unsigned char i;
    for(i=0; i<sizeOfArray-1; i++){
        displayCharOnLCD(array[i]);
    }
}


void modeSelection()
{
    

}
void inicioManual()
{

}
void inicioAuto()
{

}
void inicioTimer()
{

}
void configTimer()
{

}
void workingManual(int zona)
{

}
void workingAuto(int zona)
{

}
void workingTimer(int zona)
{

}

void interrupt low_priority ISRL()
{

}
void interrupt  ISRH()
{
    if(INTCONbits.RBIF==1)
    {
        while(INTCONbits.RBIF == 1) {
            
            groundAllRows();  //ground all rows so we can detect if a button is pressed

            if(PORTB != 0xF0){    //if PORTB is different from 0xF0, a button is being pressed, we must implement oneShot
                oneShot();
            }
            if(oneShotFlag == 1){ //if the oneShotFlag == 1, reset the oneShotFlag
                resetOneShot();
            }
            
        }
        return;
    }

}