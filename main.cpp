/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Button-controlled counter with 7-segment display and logging.
 * @details This program can count (plus, subtract, reset) via button press.
 * The value range is from 0 to 299, he is strictly enforced by design.
 * All success and error states are logged via printf for debugging.
 * You can read the logs via hterm (baudrate: 9600, Port: COM_3, Data 8, Stop 1, Parity None, Newline at: CR+LF)
 * The logs help identify most errors that may occur during operation.
 *
 * @author  Mike Mayer
 * @date    01/10/2025
 * @since   v1.0.0.0
 * @version V1.0.1.0
 * @warning Please note that this program is designed for a NUCLEO_L152RE with extension board,
 * other MC & boards may work but this is not granted 
 * 
 * Copyright (c) 2025 dercrafter & all contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#if 0

#include "mbed.h"

Ticker seg_switch;
Timeout debounceTimeout;

PortOut seg_display(PortC, 0xFF);

DigitalOut seg_left(PC_11);
DigitalOut seg_right(PC_12);
DigitalOut seg_led(PA_5);

/*
* The interrupts in the program.
* The plusButton will be used to count +1 on the old counter value each PA_1 is pressed.
* The subtractButton will be used to count -1 on the old counter value each PA_6 is pressed.
* The subtractButton will be used to reset counter value to 0 each PA_6 is pressed.
* To locate the Pins we will promote to look in to the Datasheet of the Nucleo_L152RE and the extensionboard.
*/
InterruptIn plusButton(PA_1);       //add 1 to the counter
InterruptIn subtractButton(PA_6);   //withdraw 1 from the counter
InterruptIn resetButton(PA_10);     //reset the counter

/*
* decimal patterns from 0 to 9 for the seven segment display.
* to understand the lofic behind each pattern you should look into the datasheet of the extensionboard
*/
constexpr int seg_numbers[10] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111  // 9
};

// some general limits for the program operation
constexpr int MIN_COUNT = 0;
constexpr int MAX_COUNT = 299;
constexpr int SEG_LIMIT_1 = 99;
constexpr int SEG_LIMIT_2 = 199;

// two waiting times for some operations in the program
constexpr std::chrono::milliseconds WAITING_TIME(2);        //waituing time for seven segment display (status change) and the while slave in the main method
constexpr std::chrono::milliseconds DEBOUNCE_TIME(150);     //waitung time for the debounce part of the program

bool status;    //requiered to switch between the left and right segment
volatile int counter = MIN_COUNT;   //the global atribute for the counter to count

/*
* A printer mnethod to outsource the printf cmd from the regular methods because it has errors with ISR and ticker.
* It's also a good idea to centralize the debug / logging part to structure the sourcecode.
* The method use a PrinterCode and a ExtensionCode for printing,
* the first one will be used to decided which printer job is the right and the extension code
* is required to send extra data to the printer job normal it is not required so the executer send 0 as extensioncode.
* This is enforced because otherwise the program can't run
*/
void printer(int pPrinterCode, int pExtensionCode){
    printf("[printer] starting printer() method \n");

    if(pPrinterCode==0){
        printf("[main] starting main() method \n");
    }else if(pPrinterCode==1){
        printf("[hw_init] starting hw_init() method \n");
    }else if(pPrinterCode==2){
        printf("[main] Counter: %d \n", counter);
    }else if(pPrinterCode==3){
        printf("[isr_plus] starting isr_plus() method \n");
    }else if (pPrinterCode==4){
        printf("[isr_subtract] starting isr_subtract() method \n");
    }else if (pPrinterCode==5){
        printf("[isr_reset] starting isr_reset() method \n");
    }else if(pPrinterCode==6){
        printf("[isr_plus] this action is not allowed \n");
        printf("Reason: Value border reached, the following operation will produce an error out of bones exception \n");
    }else if(pPrinterCode==7){
        printf("[isr_subtract] this action is not allowed \n");
        printf("Reason: Value border reached, the following operation will produce an error out of bones exception \n");
    }else if(pPrinterCode==8){
        printf("[enable_isr] starting enable_isr() method \n");
    }else if(pPrinterCode==9){
        printf("[enable_isr] %d is not a valid ISR_Code \n", pExtensionCode);
    }else {
        printf("[printer] %d is not a valid PrinterCode \n", pPrinterCode);
    }

}

void enable_isr(int pISR_Code){
    printer(8, 0);

    if(pISR_Code==0){
        plusButton.enable_irq();
    }else if(pISR_Code==1){
        subtractButton.enable_irq();
    }else if(pISR_Code==2){
        resetButton.enable_irq();
    }else{
        printer(9, pISR_Code);
    }

}

void isr_plus(){
    plusButton.disable_irq();
    printer(3, 0);

    if(counter >= MIN_COUNT && counter < MAX_COUNT){
        counter = counter + 1;
    }else{
        printer(6, 0);
    }
    
    printer(2, 0);
    debounceTimeout.attach(callback([] { enable_isr(0); }), DEBOUNCE_TIME);
}

void isr_subtract(){
    subtractButton.disable_irq();
    printer(4, 0);
    
    if(counter > MIN_COUNT && counter <= MAX_COUNT){
        counter = counter - 1;
    }else{
        printer(7, 0);
    }

    printer(2, 0);
    debounceTimeout.attach(callback([] { enable_isr(1); }), DEBOUNCE_TIME);
}

void isr_reset(){
    resetButton.disable_irq();
    printer(5, 0);
    counter = 0;
    printer(2, 0);
    debounceTimeout.attach(callback([] { enable_isr(2); }), DEBOUNCE_TIME);
}

void hw_init(){
    printer(1, 0);
    seg_display = 0b00000000;
    status = true;
    seg_left = 0;
    seg_right = 0;
    seg_led = 0;
    printer(2, 0);
}

void isr_Display(){
    int value = counter;
    if (value >= MIN_COUNT && value <= SEG_LIMIT_1) {
        if (status == 0) {
            seg_left = 1;
            seg_right = 0;
            seg_led = 0;
            seg_display = seg_numbers[value % 10];
        } else {
            seg_left = 0;
            seg_right = 1;
            seg_led = 0;
            seg_display = seg_numbers[(value / 10) % 10];
        }
    }else if (value > SEG_LIMIT_1 && value <= SEG_LIMIT_2) {
        if (status == 0) {
            seg_left = 1;
            seg_right = 0;
            seg_led = 0;
            seg_display = seg_numbers[value % 10] | (1 << 7); 
        } else {
            seg_left = 0;
            seg_right = 1;
            seg_led = 0;              
            seg_display = seg_numbers[(value / 10) % 10] | (1 << 7);
        }
    }else if (value > SEG_LIMIT_2 && value <= MAX_COUNT) {
        if (status == 0) {
            seg_left = 1;
            seg_right = 0;
            seg_led = 1;          
            seg_display = seg_numbers[value % 10] | (1 << 7);  
        } else {
            seg_left = 0;
            seg_right = 1;
            seg_led = 1;           
            seg_display = seg_numbers[(value / 10) % 10] | (1 << 7); 
        }
    }else {
        if (status == 0) {
            seg_left = 1;
            seg_right = 0;
            seg_led = 1;          
            seg_display = 0b01111001; 
        } else {
            seg_left = 0;
            seg_right = 1;
            seg_led = 1;           
            seg_display = 0b01111001;
        }
    }
    status = !status;
}

int main(){
    printer(0, 0);
    hw_init();
    seg_switch.attach(&isr_Display, WAITING_TIME);

    plusButton.mode(PullDown);
    plusButton.rise(&isr_plus);

    subtractButton.mode(PullDown);
    subtractButton.rise(&isr_subtract);

    resetButton.mode(PullDown);
    resetButton.rise(&isr_reset);

    for(int i = 0; i<3; i++){
        enable_isr(i);
    }

    __enable_irq();

    while (true) {
        ThisThread::sleep_for(WAITING_TIME);
    }

}

# endif