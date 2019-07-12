/*
 * atmel.c
 *
 * Created: 12.07.2019 14:19:46
 * Author : 75300
 */ 
# define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
	DDRB = 0b00111110;

	PORTB = 0b11110000;
	
	_delay_ms(1000);
    /* Replace with your application code */
    while (1) 
    {
			PORTB = 0b11111111;
			_delay_ms(1000);
			PORTB = 0b00000000;
			_delay_ms(1000);
    }
}

