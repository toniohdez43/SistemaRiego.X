/* 
 * File:   SysRiego.c
 * Author: 
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


// System messages
unsigned char GREETING[] = "Hello World";
unsigned char CASTELLO[] = "Castello <";

unsigned char automaticMessage[] = "Automatic mode";
unsigned char timerMessage[] = "Timer mode";
unsigned char manualMessage[] = "Manual mode";

unsigned char configAutoMessage[] = "Humidity: ";

unsigned char manualActiveMessage[] = "Active";
unsigned char manualInactiveMessage[] = "Inactive";

unsigned char columnSelected; 
unsigned char rowSelected;
unsigned char keyCode; 
unsigned char *zoneSelected; //Zone selected by the user to be configured
unsigned char modeSelected; // 0 = automatico, 1 = timer, 2 = manual

unsigned char getValueStatus; // When the getValueStatus is equal to 1 is that the system has gotten the input value from the user
unsigned char statusConfiguration; // When configuracionStatus is equal to 1 the system will enter to configuration mode
unsigned char oneShotFlag;          //  One-shot flag.

// Irrigation system variables
//===================================================

// Humidity zone values

unsigned char humidityZn1Val[] = "01"; 
unsigned char humidityZn2Val[] = "02";
unsigned char humidityZn3Val[] = "03";

// Timer zone values

unsigned char hourConfiguration = 0;
unsigned char dayName[] = "000";

unsigned char counterSEC='0';//Overflow counter
unsigned char counterMIN[]="00";
unsigned char counterHOUR[]="00";
unsigned char counterDAYS='0';


unsigned char time1Zn1[] = "0001";
unsigned char time2Zn1[] = "0004";
unsigned char time3Zn1[] = "0007";

unsigned char time1Zn2[] = "0003";
unsigned char time2Zn2[] = "$$$$";
unsigned char time3Zn2[] = "$$$$";

unsigned char time1Zn3[] = "0000";
unsigned char time2Zn3[] = "$$$$";
unsigned char time3Zn3[] = "$$$$";


unsigned char *zoneTimes[] = {
        time1Zn1,
        time2Zn1,
        time3Zn1,
        time1Zn2,
        time2Zn2,
        time3Zn2,
        time1Zn3,
        time2Zn3,
        time3Zn3
};

unsigned char endTimeZn1[] = "$$$$";
unsigned char endTimeZn2[] = "$$$$";
unsigned char endTimeZn3[] = "$$$$";

unsigned char *zoneEndTimes[] = {
        endTimeZn1,
        endTimeZn2,
        endTimeZn3
};

unsigned char irrigateTimeZn1[] = "02";
unsigned char irrigateTimeZn2[] = "01";
unsigned char irrigateTimeZn3[] = "01";

// Manual zone values. When '1' is selected zone will be active, when '0' zone will be inactive.

unsigned char manualZn1Val = '1';
unsigned char manualZn2Val = '1';
unsigned char manualZn3Val = '1';

unsigned char irrigateManualStatus = 0;

// Functions
//===================================================================================
void modeSelection();
void inicioManual();
void inicioAuto();
void inicioTimer();


void oneShot();
void resetOneShot();
void delay();

void groundAllRows();
void determinePressedRow();
unsigned char getKeyCode();
void pressedKey();
void pressedKeyAction(unsigned char x);

// LCD Functions
//===================================================================================

void portDAsInput();
void portDAsOutput();
void LCDSetup();
void shortDelay();
void longDelay();
void command (unsigned char x);
void isReady();
void displayCharOnLCD(unsigned char character);
void displayLineOnLCD(unsigned char array[], unsigned char sizeOfArray);
void clearDisplay();
void moveCurR();
void moveCurR();
void moveCurL();

//Interruption Functions
//===================================================================================

void interrupt low_priority ISRL();
void interrupt ISRH();
unsigned char getInput();
void turnOnAllRows();
void getValue();
void getKey();

void readInput();
void readKey();
void resetReadInput();

void disableIntRBIE();

void enableIntRBIE();

// Functions that configure each of the modes and its respective zone
//===================================================================================
void automaticConfiguration();
void timerConfiguration();
void manualConfiguration();


// Functions that have the business logic from the selected mode.
//===================================================================================

void workingManual();
void workingAutomatic();
void workingTimer();



void displayTime();
void getDayName();
void startIrrigation(unsigned char zone);
void updateEndTime(unsigned char *endTimeZn, unsigned char *zoneTime, unsigned char *irrigateTimeZn);
void stopIrrigation(unsigned char zone);
void stopIrrigateManual();

void irrigateManual();
void stopIrrigateManual();

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
    
    PORTEbits.RE0 = 0; // Initialize PORTE
    PORTEbits.RE1 = 0;
    PORTEbits.RE2 = 0;
    
    LATEbits.LATE0 = 0; // Clear PORTE
    LATEbits.LATE1 = 0;
    LATEbits.LATE2 = 0;
    
    TRISEbits.RE0 = 0;
    TRISEbits.RE1 = 0;
    TRISEbits.RE2 = 0;

    
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
    
    //Default inition of the system
    
    //columnSelected = 0; 
    //rowSelected = 0;
    //keyCode = ' '; 
    
    //zoneSelected = 0; 
    modeSelected = 0; // Automatic mode by default

    getValueStatus = 0; // When the getValueStatus is equal to 1 is that the system has gotten the input value from the user
    statusConfiguration = 0; // When configuracionStatus is equal to 1 the system will enter to configuration mode
    oneShotFlag = 0;          //  One-shot flag.
    
    
    //Timer#0 Interrupt as High Priority
    INTCON2bits.TMR0IP = 1;
    
    //Enable Timer #0 Interrupt
    INTCONbits.TMR0IE = 1;
    
    //Internal instruction cycle,
    //Timer DesActivated,16 bits, Prescaler 256    
    T0CON = 0x07;
    
    //Accumulator, starting from 18,660 since we want to count 12 seconds
    //TMR0 = 0x48E4;
    TMR0 = 0x3A98;
    //Enable Timer#0
    T0CONbits.TMR0ON = 1;

    
    
    //infinite loop to constantly be waiting for user input on the keyboard matrix
    while(1){
        
        if(statusConfiguration == 0) {
            switch(modeSelected) {

                case 0:
                    workingAutomatic();
                break;

                case 1:
                    workingTimer();
                break;

                case 2:
                    workingManual();
                break;

            }
        }
        else {
            
            switch(modeSelected) {

                case 0:
                    automaticConfiguration();
                break;

                case 1:
                    timerConfiguration();
                break;

                case 2:
                    manualConfiguration();
                break;

            }
            
        }
            
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

void displayLineOnLCD(unsigned char array[], unsigned char sizeOfArray){
    unsigned char i;
    for(i=0; i<sizeOfArray-1; i++){
        displayCharOnLCD(array[i]);
    }
}

void clearDisplay() {
    command(0x01);
    shortDelay();
    isReady();
}

void moveCurH() {
    command(0xA8);
    //shortDelay();
    //isReady();
}

void moveCurR() {
    command(0x1C);
    shortDelay();
    isReady();
}

void moveCurL() {
    command(0x18);
    shortDelay();
    isReady();
}

//Program functions
//========================================================================================






// Functions that configure each of the modes and its respective zone
//======================================================================================

void automaticConfiguration() {
    
    
    disableIntRBIE();
    
    unsigned char input = ' ';
    unsigned char counter = 0;
    
    clearDisplay();
    displayLineOnLCD(configAutoMessage, sizeof(configAutoMessage) / sizeof(configAutoMessage[0]));
    displayCharOnLCD(zoneSelected[0]);
    displayCharOnLCD(zoneSelected[1]);
 
    
    while(input != '#')  {  
        
        
        input = getInput();
        
        if(input != '#') {
            zoneSelected[counter%2] = input;
            clearDisplay();
            displayCharOnLCD(zoneSelected[0]);
            displayCharOnLCD(zoneSelected[1]);
            displayCharOnLCD('%');
            counter++;
        }
        
    }
    
    enableIntRBIE();
    
    statusConfiguration = 0;
    
}

void timerConfiguration() {

    disableIntRBIE();
    
    unsigned char input = ' ';
    
    clearDisplay();
    
    if (hourConfiguration == 0) {
        
        //configuration for timer
        
        
    }
    else {
        
        //configuration for hour
        
        input = getInput();
        
        
        hourConfiguration = 0;
    }
        
    enableIntRBIE();
    
    statusConfiguration = 0;    

}


void manualConfiguration() {
    
    disableIntRBIE();
    
    unsigned char input = ' ';
    
    clearDisplay();
    
    if(*zoneSelected == '1') {
        displayLineOnLCD(manualActiveMessage, sizeof(manualActiveMessage) / sizeof(manualActiveMessage[0]));
    }
    
    else {
        displayLineOnLCD(manualInactiveMessage, sizeof(manualInactiveMessage) / sizeof(manualInactiveMessage[0]));
    }
 
    
    while(input != '#')  {  
        
        input = getInput();

        clearDisplay();
        
        if(input == '1') {
            *zoneSelected = '1';
            displayLineOnLCD(manualActiveMessage, sizeof(manualActiveMessage) / sizeof(manualActiveMessage[0]));
        }
        
        else if (input == '0') {
            *zoneSelected = '0';
            displayLineOnLCD(manualInactiveMessage, sizeof(manualInactiveMessage) / sizeof(manualInactiveMessage[0]));
        }
        
        else {
            if (*zoneSelected == '1') {
                displayLineOnLCD(manualActiveMessage, sizeof (manualActiveMessage) / sizeof (manualActiveMessage[0]));
            }
            else {
                displayLineOnLCD(manualInactiveMessage, sizeof (manualInactiveMessage) / sizeof (manualInactiveMessage[0]));
            }
        }
        
    }
    
    enableIntRBIE();
    
    statusConfiguration = 0;
    
}

// Functions that have the business logic from the selected mode.
//======================================================================================
void workingAutomatic() {
    
    
    clearDisplay();
    displayLineOnLCD(automaticMessage, sizeof(automaticMessage) / sizeof(automaticMessage[0]));
    
    while(modeSelected == 0 && statusConfiguration == 0) {
        
        
        
    }

}

void workingTimer() {

    clearDisplay();
    displayLineOnLCD(timerMessage, sizeof(timerMessage) / sizeof(timerMessage[0]));
    
    unsigned char *zoneTime;
    
    while(modeSelected == 1 && statusConfiguration == 0) {
        command(0xC0);
        displayLineOnLCD(counterHOUR, 3);
        displayLineOnLCD(counterMIN, 3);
        
        for(unsigned char i = 0; i < 9; i++) {
            
            zoneTime = zoneTimes[i];
            
            if (zoneTime[0] != '$') {
                
                if (zoneTime[0] == counterHOUR[0]) {
                    
                    if (zoneTime[1] == counterHOUR[1]) {
                        
                        if (zoneTime[2] == counterMIN[0]) {
                            
                            if (zoneTime[3] == counterMIN[1]) {
                                
                                if (i < 3) {
                                    updateEndTime(endTimeZn1, zoneTime, irrigateTimeZn1);
                                    startIrrigation(1);
                                }
                                else if (i < 6) {
                                    updateEndTime(endTimeZn2, zoneTime, irrigateTimeZn2);
                                    startIrrigation(2);
                                }
                                else if (i < 9) {
                                    updateEndTime(endTimeZn3, zoneTime, irrigateTimeZn3);
                                    startIrrigation(3);
                                }
                                
                                /*
                                clearDisplay();
                                displayLineOnLCD("Regando: ", 10);
                                displayLineOnLCD(selectedTime, 5);
                                */
                            }
                            
                        }
                    
                    }
                    
                }
            
            }
            
        }
        
        for(unsigned char i = 0; i < 3; i++) {
            
            unsigned char *endTimeZn = zoneEndTimes[i];
            
            if(endTimeZn[0] != '$') {
                if(endTimeZn[0] == counterHOUR[0]) {
                    if(endTimeZn[1] == counterHOUR[1]) {
                        if(endTimeZn[2] == counterMIN[0]) {
                            if(endTimeZn[3] == counterMIN[1]) {
                                stopIrrigation(i + 1);
                                for(unsigned char j = 0; j < 4; j ++)
                                    endTimeZn[j] = '$';
                            }
                        }
                    }
                }
            }
        
        }
        
    }
    
}

void workingManual() {

    clearDisplay();
    displayLineOnLCD(manualMessage, sizeof(manualMessage) / sizeof(manualMessage[0]));
    
    while(modeSelected == 2 && statusConfiguration == 0) {
        
        
        
    }
   
}

//Interruption functions
//======================================================================================

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
    
    //Check if it is TMR0 Overflow ISR
    if(TMR0IE && TMR0IF)
    {
        counterSEC++;
        if(counterSEC=='5')
        {
            counterSEC='0';
            counterMIN[1]++;
            if(counterMIN[1]==':')
            {
                counterMIN[1]='0';
                counterMIN[0]++;
                if(counterMIN[0]=='6')
                {
                    
                    if(counterHOUR[0] < '2') {
                    
                        counterMIN[0]='0';
                        counterHOUR[1]++;
                        
                        if(counterHOUR[1] == ':') {
                            counterHOUR[1] = '0';
                            counterHOUR[0]++;
                        }
                    
                    } 
                    
                    else if( counterHOUR[1] < '3')
                    {                     
                        counterMIN[0]='0';
                        counterHOUR[1]++;                       
                    }
                    
                    else {
                        
                        counterSEC = '0'; //Overflow counter
                        counterMIN[0] = '0';
                        counterMIN[1] = '0';
                        counterHOUR[0] = '0';
                        counterHOUR[1] = '0';
                        counterDAYS++;
                       
                        if (counterDAYS == '7') {
                            
                            counterSEC = '0';//Overflow counter
                            counterMIN[0] = '0';
                            counterMIN[1] = '0';
                            counterHOUR[0] = '0';
                            counterHOUR[1] = '0';
                            counterDAYS = '0';
                        }                   
                    }                 
                }               
            }
        }
        
        INTCONbits.TMR0IF = 0;
        TMR0 = 0x3A98;
        
    }

}

// Await for input from the user Functions(Configuration)
//=======================================================================================

unsigned char getInput() {
    
    getValueStatus = 0; //Status that determines when the user has entered a value to the keypad
    
    oneShotFlag = 0;
    
    while(getValueStatus == 0) {
        
        groundAllRows();  //ground all rows so we can detect if a button is pressed

        if (PORTB != 0xF0) { //if PORTB is different from 0xF0, a button is being pressed, we must implement oneShot
            readInput();
        }
        
        if (oneShotFlag == 1) { //if the oneShotFlag == 1, reset the oneShotFlag
            resetReadInput();   
        }
        
    }
    
    return keyCode;

}



// Keypad Logic and Functions for Interruptions
//========================================================================================

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

//10 ms delay
void delay(){
    unsigned char L1REG = 0;
    unsigned char L2REG;
    while(L1REG++ < 13){
        L2REG = 0;
        while(L2REG++ < 255);
    }
}

void resetOneShot(){
    if (PORTB  == 0xF0)      //IF RB0 OFF THEN Eliminate Bouncing
        delay();
    else                    //ELSE EXIT
        return;
    if (PORTB  == 0XF0){      //IF RB0 OFF THEN reset one-shot flag and Interruption change flag
        INTCONbits.RBIF=0;
        oneShotFlag = 0;
    }
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
    
    switch(keyCode){
        
        case '0': 
            
        break;
        
        
        case '1': 
            
            if (modeSelected == 0) {
                zoneSelected = humidityZn1Val;
            }
            
            if (modeSelected == 1) {
                //zoneSelected = humidityZn1Val;
            }
            
            if (modeSelected == 2) {
                zoneSelected = &manualZn1Val;
            }
            
            statusConfiguration = 1;
            
        break;
        
        
        case '2': 
            
            if (modeSelected == 0) {
                zoneSelected = humidityZn2Val;
            }
            
            if (modeSelected == 1) {
                //zoneSelected = humidityZn1Val;
            }
            
            if (modeSelected == 2) {
                zoneSelected = &manualZn2Val;
            }
            
            statusConfiguration = 1;
            
        break;
        
        
        case '3': 
            
            if (modeSelected == 0) {
                zoneSelected = humidityZn3Val;
            }
            
            if (modeSelected == 1) {
                zoneSelected = humidityZn1Val;
            }
            
            if (modeSelected == 2) {
                zoneSelected = &manualZn3Val;
            }
            
            statusConfiguration = 1;
            
        break;
        
        
        case '4': 
         
        break;
        
        
        case '5': 
            
            
        break;
        
        
        case '6': 
            
        break;
        
        
        case '7': 
            
        break;
        
        
        case '8': 
            
        break;
        
        
        case '9': 
            
        break;
        
        
        case 'A': 
            
            modeSelected = 0; //Automatico
            
        break;
        
        
        case 'B': 
            
            modeSelected = 1; //Timer
            
        break;
        
        
        case 'C': 
        
            modeSelected = 2; //Manual
            
        break;
        
        
        case 'D': 
        
        break;
        
        
        case '*': 
            
        break;
        
        
        case '#': 
            
            if(modeSelected == 1) {
                hourConfiguration = 1;
                statusConfiguration = 1;
            }
            
            if(modeSelected == 2) {
                
                if(irrigateManualStatus == 0) {
                    irrigateManual();
                }
                
                else {
                    stopIrrigateManual();
                }
                
            }
            
        break;
        
        
        default: ;
        
       /*switch(keyCode){
        case '*': displayLineOnLCD(GREETING,sizeof(GREETING) / sizeof(GREETING[0])); break;
        case '#': displayLineOnLCD(CASTELLO,sizeof(CASTELLO) / sizeof(CASTELLO[0])); break;
        default: displayCharOnLCD(keyCode);
    

        }*/
    }
}

void turnOnAllRows(){
    row1 = 1;
    row2 = 1;
    row3 = 1;
    row4 = 1;
}

// Keypad Logic and Functions for getting input from user
//========================================================================================

void readInput(){     
    if (oneShotFlag == 1)    //IF One-Shot has been triggered THEN EXIT.
        return;
    delay();                 //Eliminate Bouncing.
    if (PORTB  != 0xF0){     //IF a button is still pressed
        readKey();       //a key was pressed, determine which key and act accordingly
        oneShotFlag = 1;            //Turn-ON one-shot flag.
    }
}

void readKey(){
    
    switch(PORTB){      //obtain the selected column
        case 0xE0: columnSelected = 1; break;
        case 0xD0: columnSelected = 2; break;
        case 0xB0: columnSelected = 3; break;
        case 0x70: columnSelected = 4; break;
        default: columnSelected = 100;
    }
        determinePressedRow();
        keyCode = getKeyCode();
}

void resetReadInput(){
    if (PORTB  == 0xF0)      //IF RB0 OFF THEN Eliminate Bouncing
        delay();
    else                    //ELSE EXIT
        return;
    if (PORTB  == 0XF0){      //IF RB0 OFF THEN reset one-shot flag and Interruption change flag
        oneShotFlag = 0;
        getValueStatus = 1;
    }
}

void disableIntRBIE() {
    
    LATB = 0X00; 
    INTCONbits.RBIF = 0;
    INTCONbits.RBIE = 0; //Deactivate the interruptions when the user is asked for an input
    
}

void enableIntRBIE() {
    
    LATB = 0X00; 
    INTCONbits.RBIF = 0;
    INTCONbits.RBIE = 1; //Deactivate the interruptions when the user is asked for an input
    
}

void displayTime() {

    getDayName();
    displayLineOnLCD(dayName, 4);
    displayCharOnLCD(' ');
    displayLineOnLCD(counterHOUR, 3);
    displayCharOnLCD(':');
    displayLineOnLCD(counterMIN, 3);
    displayCharOnLCD(':');
    displayCharOnLCD(counterSEC);
    
}

void getDayName() {

    switch(counterDAYS) {
        case '0':
            dayName[0] = 'M';
            dayName[1] = 'o';
            dayName[2] = 'n';
        break;
        
        case '1':
            dayName[0] = 'T';
            dayName[1] = 'u';
            dayName[2] = 'e';
        break;
        
        case '2':
            dayName[0] = 'W';
            dayName[1] = 'e';
            dayName[2] = 'n';
        break;
        
        case '3':
            dayName[0] = 'T';
            dayName[1] = 'h';
            dayName[2] = 'u';
        break;
        
        case '4':
            dayName[0] = 'F';
            dayName[1] = 'r';
            dayName[2] = 'i';
        break;
        
        case '5':
            dayName[0] = 'S';
            dayName[1] = 'a';
            dayName[2] = 't';
        break;
        
        case '6':
            dayName[0] = 'S';
            dayName[1] = 'u';
            dayName[2] = 'n';
        break;
    
    }

}

void updateEndTime(unsigned char *endTimeZn, unsigned char *zoneTime, unsigned char *irrigateTimeZn) {
    
    endTimeZn[0] = zoneTime[0];
    endTimeZn[1] = zoneTime[1];
    endTimeZn[2] = zoneTime[2];
    endTimeZn[3] = zoneTime[3];
    
    endTimeZn[3] = endTimeZn[3] + (irrigateTimeZn[1] - 48);
    
    if(endTimeZn[3] > 57) {
        endTimeZn[3] = endTimeZn[3]%58 + 48;
        endTimeZn[2]++;
    }
    
    endTimeZn[2] = endTimeZn[2] + (irrigateTimeZn[0] - 48);
    
    if(endTimeZn[2] > 53) {
        endTimeZn[2] = endTimeZn[2]%54 + 48;
        endTimeZn[1]++;
    }
    
    if(endTimeZn[0] < '2') {
        
        if(endTimeZn[1] > 57) {
            endTimeZn[1] = endTimeZn[1]%58 + 48;
            endTimeZn[0]++;
        }
    
    }
    
    else {
        
        if(endTimeZn[1] > 51) {
            endTimeZn[1] = endTimeZn[1]%52 + 48;
            endTimeZn[0]++;
        }
        
        if (endTimeZn[0] > 50) {
            endTimeZn[0] = endTimeZn[0]%51 + 48;
        }
    
    }

}

void startIrrigation(unsigned char zone) {
    
    switch(zone) {
        
        case 1:
            PORTEbits.RE0 = 1;
        break;
        
        case 2:
            PORTEbits.RE1 = 1;
        break;
        
        case 3:
            PORTEbits.RE2 = 1;
        break;
    
    }

}

void stopIrrigation(unsigned char zone) {
    
    switch(zone) {
        
        case 1:
            PORTEbits.RE0 = 0;
        break;
        
        case 2:
            PORTEbits.RE1 = 0;
        break;
        
        case 3:
            PORTEbits.RE2 = 0;
        break;
    
    }

}

void irrigateManual() {
    
    if(manualZn1Val == '1') {
        startIrrigation(1);
    }
    
    if(manualZn2Val == '1') {
        startIrrigation(2);
    }
    
    if(manualZn3Val == '1') {
        startIrrigation(3);
    }
    
    irrigateManualStatus = 1;
    
}

void stopIrrigateManual(){
    
    stopIrrigation(1);
    stopIrrigation(2);
    stopIrrigation(3);
    
    irrigateManualStatus = 0;
    
}