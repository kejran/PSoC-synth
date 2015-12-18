#ifndef NOTES
#define NOTES
    
#include <stdint.h>
#include <string.h>

#define VOICES 8
    
typedef struct {
    uint32_t phase_accumulator; //q0.32
    volatile uint32_t phase_step; //q0.32
    uint8_t note;
    uint8_t velocity; //q0.8
} voice;  
    
extern const int16_t sine[512];
extern const uint32_t notes[128];
extern voice voice_bank[VOICES];
extern uint8_t voice_queue[VOICES];
    
  
    
void set_note(uint8_t v, uint8_t note, uint8_t velocity) {
    voice_bank[v].phase_step = notes[note];
    voice_bank[v].note = note;
    voice_bank[v].velocity = velocity;
}

int8_t find_note(uint8_t value) {
    for (int i=0; i<VOICES; i++) 
        if (voice_bank[i].velocity && voice_bank[i].note==value) return i;
    return -1;
}

void shiftr(int8_t voice) {
    uint8_t id = voice_queue[voice];
    memmove(voice_queue+voice, voice_queue+voice+1, VOICES-1-voice);
    voice_queue[VOICES-1]=id;
}

void shiftl(int8_t voice) {
    uint8_t id = voice_queue[voice];
    memmove(voice_queue+1, voice_queue, voice);
    voice_queue[0]=id;
}

void note_on(uint8_t note, uint8_t velocity) {
    
    set_note(voice_queue[0], note, velocity);
    shiftr(0);
    
}

void note_off(uint8_t note) {

    int8_t id = find_note(note);
    if (id==-1) return;
    
    for (int i=0; i<VOICES; i++) 
        if (voice_queue[i]==id) {shiftl(i); break;}
    
    set_note(id, 0, 0);
    
}

#endif