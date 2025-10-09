#if 1
#include "mbed.h"

Ticker seg_switch;
Timeout debounceTimeout;
PortOut seg_display(PortC, 0xFF);
DigitalOut seg_left(PC_11);
DigitalOut seg_right(PC_12);
InterruptIn plusButton(PA_10);
InterruptIn subtractButton(PA_6);
InterruptIn resetButton(PA_1);
constexpr int seg_numbers[10] = {
    0b00111111,
    0b00000110,
    0b01011011,
    0b01001111,
    0b01100110,
    0b01101101,
    0b01111101,
    0b00000111,
    0b01111111,
    0b01101111
};
constexpr int MIN_COUNT = 0;
constexpr int MAX_COUNT = 99;
constexpr std::chrono::milliseconds WAITING_TIME(10);
constexpr std::chrono::milliseconds DEBOUNCE_TIME(150);
bool status;
volatile int counter = MIN_COUNT;

void enable_isr(int pISR_Code){
    if(pISR_Code==0){
        plusButton.enable_irq();
    }else if(pISR_Code==1){
        subtractButton.enable_irq();
    }else if(pISR_Code==2){
        resetButton.enable_irq();
    }
}
void isr_plus(){
    plusButton.disable_irq();
    if(counter >= MIN_COUNT && counter < MAX_COUNT){
        counter = counter + 1;
    }
    debounceTimeout.attach(callback([] { enable_isr(0); }), DEBOUNCE_TIME);
}
void isr_subtract(){
    subtractButton.disable_irq();
    if(counter > MIN_COUNT && counter <= MAX_COUNT){
        counter = counter - 1;
    }
    debounceTimeout.attach(callback([] { enable_isr(1); }), DEBOUNCE_TIME);
}
void isr_reset(){
    resetButton.disable_irq();
    counter = 0;
    debounceTimeout.attach(callback([] { enable_isr(2); }), DEBOUNCE_TIME);
}
void isr_Display(){
    int value = counter;
    if (status == 0) {
        seg_left = 1;
        seg_right = 0;
        seg_display = seg_numbers[value % 10];
    } else {
        seg_left = 0;
        seg_right = 1;
        seg_display = seg_numbers[(value / 10) % 10];
    }
    status = !status;
}
void hw_init(){
    seg_display = 0b00000000;
    status = true;
    seg_left = 0;
    seg_right = 0;
}
int main(){
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