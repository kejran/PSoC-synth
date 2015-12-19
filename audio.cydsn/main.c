#include <project.h>
#include "notes.h"

#define INTERPOLATE
#define SOFTCLIPPING

#define Q16MUL(a, b) ((((a&0xffff)*b)>>16)+((a>>16)*b)) //multiplies xQ16 times xQ16
#define Q16LERP(a, b, amount) (((b*amount)+(a*(256-amount)))>>8) //lerp between two q16 using q8

voice voice_bank[VOICES]={{0,0,0,0,0,0,0,0,0}};
uint16_t global_volume = 65535/4; //q16;
uint8_t voice_queue[VOICES];

uint8_t led_countdown=0;

int32_t clip (int32_t value) {   
	if (value>32767) return 32767;
    if (value<-32768) return -32768;
    int32_t x = 98304-2*((value*value)>>16);
    return (x*value)>>16;
}

int16_t interpolate(uint32_t acc) {
    #ifdef INTERPOLATE
        int id1 = acc>>23;
        int id2 = (id1+1)&511;
        int offset = (acc>>15)&0xff;
        return Q16LERP(sine[id1], sine[id2], offset);
    #else
        return sine[acc>>23];
    #endif
}

CY_ISR(sample_isr_fn) {
    
    int32_t acc = 0;
    for (int i=0; i<VOICES; i++) {
        voice_bank[i].phase_accumulator += voice_bank[i].phase_step;
        acc += voice_bank[i].velocity*(interpolate(voice_bank[i].phase_accumulator));
    }
    acc>>=8;
    acc = Q16MUL(acc, global_volume);
   
    #ifdef SOFTCLIPPING
        if (!Button_Read())
        acc = clip(acc);
    #endif
    
    acc = (acc>>8)+128;    
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

void cc_update(uint8_t control, uint8_t value) {
    // CC handling happens here; volume/filter modification, etc
}

int main() {
    
    for (int i=0; i<VOICES; i++) 
        voice_queue[i]=i;
        
    CyGlobalIntEnable; 

    IDAC_Start();
    VOutAmp_Start();
    MidiUART_Start();
    
    sample_isr_StartEx(sample_isr_fn);
    SampleTimer_Start();
    
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
        
        if ((value&0xf0)==0xB0) { // CC
            cc_update(uart_get_blocking(), uart_get_blocking());
            
        }
        
        if (value==0xfc) //midi stop
            for (int i=0; i<VOICES; i++) 
                set_note(i, 0, 0);
        
    }
}

