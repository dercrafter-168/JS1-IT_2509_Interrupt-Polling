#if 1

#include "mbed.h"

Ticker seg_switch;
Timeout debounceTimeout;

PortOut seg_display(PortC, 0xFF);

DigitalOut seg_left(PC_11);
DigitalOut seg_right(PC_12);
DigitalOut seg_led(PA_5);

InterruptIn plusButton(PA_1);
InterruptIn subtractButton(PA_6);
InterruptIn resetButton(PA_10);

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
constexpr int MAX_COUNT = 299;
constexpr int SEG_LIMIT_1 = 99;
constexpr int SEG_LIMIT_2 = 199;

constexpr std::chrono::milliseconds WAITING_TIME(2);
constexpr std::chrono::milliseconds DEBOUNCE_TIME(150);

bool status;
volatile int counter = MIN_COUNT;

void printer(int pPrinterCode, int pExtensionCode){
    printf("[printer] starting printer() method \n");

    if(pPrinterCode==0){
        printf("[mainV2] starting main() method \n");
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