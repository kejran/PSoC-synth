#ifndef NOTES
#define NOTES
    
#include <stdint.h>
#include <string.h>
#include "global.h"
    
typedef struct {
    uint16_t a;
    uint16_t d;
    uint16_t s;
    uint16_t r;
    uint8_t state;
} envelope;

typedef struct {
    uint32_t phase_accumulator; //q0.32
    volatile uint32_t phase_step; //q0.32
    uint8_t note;
    uint8_t velocity; //q0.8
    envelope env;
} voice;  

uint16_t pow2(uint8_t value);
extern int16_t pitchbend;
extern voice voice_bank[VOICES];

int8_t find_note(uint8_t value) {
    for (int i=0; i<VOICES; i++) 
        if (voice_bank[i].velocity && voice_bank[i].note==value) return i;
    return -1;
}

uint32_t note_step_offset(uint8_t midi_note, int16_t shift) { // shift is signed 7q8 only!
    midi_note+= (shift>>8); //make sure to clamp the note&shift later
    shift&=0xff;
    if (!shift) return notes[midi_note]; //skip calculations if no shift

    uint64_t lerp = ((uint64_t)notes[midi_note])*(256-shift) + ((uint64_t)notes[midi_note+1])*shift;  //this 64-bit is going to bite me in the ass later
    return lerp>>8; //linear interpolation for now!
}

void recalculate_step(voice* v) {
    v->phase_step = note_step_offset(v->note, pitchbend);
}

void set_note(uint8_t v, uint8_t note, uint8_t velocity) {
    voice_bank[v].velocity = 0;
//    voice_bank[v].phase_step = notes[note];
    voice_bank[v].note = note;
    recalculate_step(voice_bank+v);
    voice_bank[v].velocity = velocity;
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