#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>

int counter = 0;


ISR (WDT_vect) {                      //обработка прерывания watchdog
  if (counter > 4){
    counter = 0;
  ADCSRA = (1 << ADEN) ;              //включение АЦП (в POWER_DOWN_MODE выключен)
  digitalWrite(0, HIGH);
  if ((analogRead(3) < 300) && (analogRead(1) > 600)){
    digitalWrite(4, HIGH);
    _delay_ms(1000);
    digitalWrite(4, LOW);  
  };
  ADCSRA = (0 << ADEN) ;            //выключение АЦП(потребляет о.3 мА)
  digitalWrite(0,LOW);
  WDTCR |= (1 << WDIE);             // разрешаем прерывания по watchdog. Иначе будет ресет.
  } else {counter++;};
}

int main(void)
{
    pinMode(0, OUTPUT);
    pinMode(3, INPUT);
    pinMode(4, OUTPUT);
    pinMode(1, INPUT);
    digitalWrite(0, LOW);
    wdt_reset();          
    wdt_enable(WDTO_2S);            //watch_dog инициализация 8s timer
    
    WDTCR |= (1 << WDIE);           // разрешаем прерывания по ватчдогу. Иначе будет резет.
    sei();                          //установить флаг глобального прерывания http://www.gaw.ru/html.cgi/txt/doc/micros/avr/asm/sei.htm
    MCUCR=0x30;                     // настраиваем режим энергосбережения. POWER_DOWN_MODE
   
    while(1) {
      sleep_mode();
    }
}
