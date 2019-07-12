#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
     // ADCSRA = (1 << ADEN) ;              //включение АЦП (в POWER_DOWN_MODE выключен)
        digitalWrite(4, HIGH);
      _delay_ms(1000);
      digitalWrite(4, LOW);  
    pinMode(2, INPUT);
    pinMode(4, OUTPUT);
    if (analogRead(2) > 0){
      digitalWrite(4, HIGH);
      _delay_ms(100); 
      digitalWrite(4, LOW); 
      
    }
}
