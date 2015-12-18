#ifndef QUEUE
#define QUEUE
    
#include <stdint.h>
    
void push_q(uint8_t* queue, uint8_t value) {
    queue[0]++;
    queue[queue[0]]=value;
}

uint8_t pop_q(uint8_t* queue) {
    if (!queue[0]) return -1;
    return queue[queue[0]--];
}
    
#endif