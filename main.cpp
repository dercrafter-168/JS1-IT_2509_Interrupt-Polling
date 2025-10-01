/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"

Ticker seg_switch;      //ticker

PortOut seg_display(PortC, 0xFF);
DigitalOut left = PC_11;
DigitalOut right = PC_12;
DigitalOut led = PA_5;

constexpr int seg_numbers[10] = {       //segemt pattern from 0 to 9 in decimal
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

constexpr std::chrono::milliseconds WAITING_TIME(5);        //wartezeit
bool status;         //status welche segemt anzeige als nächstes dran kommnt
volatile int value = 0;

void hw_init(){
    InterruptIn plus(PA_0);
    InterruptIn subtract(PA_6);
    InterruptIn reset(PA_10);

    seg_display = 0b00000000;       //segement wert ist alles aus
    status = true;
    left = 0;
    right = 0;
    led = 0;
}

void isr_Display() {        //ISR
    if (value >= 0 && value <= 99) {
        if (status == 0) {
            left = 1;
            right = 0;
            led = 0;
            seg_display = seg_numbers[value % 10];
        } else {
            left = 0;
            right = 1;
            led = 0;
            seg_display = seg_numbers[(value / 10) % 10];
        }
    }else if (value >= 100 && value <= 199) {
        if (status == 0) {
            left = 1;
            right = 0;
            led = 0;
            seg_display = seg_numbers[value % 10] | (1 << 7); 
        } else {
            left = 0;
            right = 1;
            led = 0;              
            seg_display = seg_numbers[(value / 10) % 10] | (1 << 7);
        }
    }else if (value >= 200 && value <= 299) {
        if (status == 0) {
            left = 1;
            right = 0;
            led = 1;          
            seg_display = seg_numbers[value % 10] | (1 << 7);  
        } else {
            left = 0;
            right = 1;
            led = 1;           
            seg_display = seg_numbers[(value / 10) % 10] | (1 << 7); 
        }
    }
    status = !status;       //kehre status um
}

int main(){
    hw_init();      // Initialisierung der Hardwarekomponenten
    seg_switch.attach(&isr_Display, 5ms);

    while (true) {
        ThisThread::sleep_for(10ms);
    } // Endlosschleife, die den Hauptthread am Leben hält

}
