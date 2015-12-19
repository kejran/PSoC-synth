#include <project.h>
#include "notes.h"
#include "global.h"

voice voice_bank[VOICES]={{0,0,0,0,0,0,0,0,0}};
uint16_t global_volume = 65535/4; //q16;
uint8_t voice_queue[VOICES];
uint16_t pow2_table[9] = {0, 5931, 12400, 19454, 27146, 35534, 44681, 54658, 65535};
int16_t pitchbend=0;
uint8_t led_countdown=0;

uint16_t pow2(uint8_t value) { //calculate 2^x-1
    return Q16LERP(pow2_table[value>>8], pow2_table[(value>>8)+1], value&0xff);
}

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
            //uart_get_blocking(); //useless, and just in case we have to deal with running status
        }
        
        if ((value&0xf0)==0xb0)  // CC
            cc_update(uart_get_blocking(), uart_get_blocking());
            
        if ((value&0xf0)==0xe0) {//pitch bend
            uint8_t lsb = uart_get_blocking(); //7-bit
            uint8_t msb = uart_get_blocking(); //7-bit
            int16_t pitch = lsb + (msb<<7) - 0x2000; //14-bit offset unsigned to signed q16
            pitchbend = pitch>>6;
            for (int i=0; i<VOICES; i++) 
                recalculate_step(voice_bank+i);
        }
        
        if (value==0xfc) //midi stop
            for (int i=0; i<VOICES; i++) 
                set_note(i, 0, 0);
        
    }
}

