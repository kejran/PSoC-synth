#ifndef GLOBAL    
#define GLOBAL
    
#include <stdint.h>
    
#define Q16MUL(a, b) ((((a&0xffff)*b)>>16)+((a>>16)*b)) //multiplies xQ16 times xQ16
#define Q16LERP(a, b, amount) (((b*(amount))+(a*(256-(amount))))>>8) //lerp between two q16 using q8

#define VOICES 12
    
#define INTERPOLATE
#define SOFTCLIPPING

extern const int16_t sine[512];
extern const uint32_t notes[128];
extern uint8_t voice_queue[VOICES];
    
#endif