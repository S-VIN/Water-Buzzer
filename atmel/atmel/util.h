#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdlib.h>

inline uint16_t myRandom(uint16_t min, uint16_t max) {
    return (rand() % (max - min) + min);
}

void delay_ms (uint16_t count) {
    while (count--) {
        _delay_ms (1);

    }
}

void delay_us (uint16_t count) {
    while (count--) {
        _delay_us (1);
    }
}