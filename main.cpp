//dev branch

/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"

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

EventQueue queue;       //event queue (busout & printf fix)
Ticker seg_switch;      //ticker

PortOut seg_display(PortC, 0xFF);
BusOut seg_select(PC_11, PC_12, PA_5);   //busout für beide segmente & LED

void hw_init(){
    InterruptIn plus(PA_0);
    InterruptIn subtract(PA_6);
    InterruptIn reset(PA_10);

    seg_select = 0b000;     //alle leds & segemte aus
    seg_display = 0b00000000;       //segement wert ist alles aus
    status = true;
}

void isr_Display() {        //ISR
    int value = 0;
    if (value >= 0 && value <= 99) {
        if (status == 0) {
            seg_select = 0b001;
            seg_display = seg_numbers[value % 10];
        } else {
            seg_select = 0b010;
            seg_display = seg_numbers[(value / 10) % 10];
        }
    }else if (value >= 100 && value <= 199) {
        if (status == 0) {
            seg_select = 0b001; 
            seg_display = seg_numbers[value % 10] | (1 << 7); 
        } else {
            seg_select = 0b010;              
            seg_display = seg_numbers[(value / 10) % 10] | (1 << 7);
        }
    }else if (value >= 200 && value <= 299) {
        if (status == 0) {
            seg_select = 0b101;            
            seg_display = seg_numbers[value % 10] | (1 << 7);  
        } else {
            seg_select = 0b110;              
            seg_display = seg_numbers[(value / 10) % 10] | (1 << 7); 
        }
    }
    status = !status;       //kehre status um
}

int main(){
    hw_init();      // Initialisierung der Hardwarekomponenten
    Thread event_thread;    // Erstellt ein neues Thread-Objekt.
    event_thread.start(callback(&queue, &EventQueue::dispatch_forever));    // Startet den Thread und übergibt den Callback an die Event-Queue.
    seg_switch.attach([] {queue.call(isr_Display);}, WAITING_TIME);     // Registrieren der `display` Funktion in der Event-Queue, die periodisch aufgerufen wird
    while (true) {} // Endlosschleife, die den Hauptthread am Leben hält
}