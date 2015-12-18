#include <project.h>
#include "notes.h"

voice voice_bank[VOICES]={{0,0,0,0}};
uint16_t global_volume = 65535/4; //q8.16;
uint8_t voice_queue[VOICES];

uint8_t led_countdown=0;

CY_ISR(sample_isr_fn) {
    
    int32_t acc = 0;
    for (int i=0; i<VOICES; i++) {
        voice_bank[i].phase_accumulator += voice_bank[i].phase_step;
        acc += voice_bank[i].velocity*(sine[voice_bank[i].phase_accumulator>>23]);
    }
    acc>>=8;
    acc*= global_volume;
    acc = (acc>>24)+128;    
    if (acc<0) acc=0;
    if (acc>255) acc=255;
    IDAC_SetValue(acc);
    
    if (led_countdown) led_countdown--;
    LED_Write(led_countdown?1:0);
    
    SampleTimer_ReadStatusRegister();
}

uint8_t uart_get_blocking() {
    while(!MidiUART_SpiUartGetRxBufferSize());
    return MidiUART_UartGetByte();
}

int main() {
    
    for (int i=0; i<VOICES; i++) 
        voice_queue[i]=i;
    
    CyGlobalIntEnable; 

    IDAC_Start();
    sample_isr_StartEx(sample_isr_fn);
    SampleTimer_Start();
    VOutAmp_Start();
    MidiUART_Start();
    
    for(;;) {
        uint32_t value = uart_get_blocking();
               
        if (!(value&0x80)) continue; //not a command
        
        if ((value&0xf0)==0x90) { //note on
            note_on(uart_get_blocking(), uart_get_blocking()*2);
            led_countdown = 255;
        }
        
        if ((value&0xf0)==0x80) { //note off
            note_off(uart_get_blocking());
            uart_get_blocking();
        }
        
        if (value==0xfc) //midi stop
            for (int i=0; i<VOICES; i++) 
                set_note(i, 0, 0);
        
    }
}

