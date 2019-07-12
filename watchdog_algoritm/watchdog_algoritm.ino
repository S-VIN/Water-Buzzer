
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <EEPROM.h>

bool wdt_flag = false;

void action() {
  
}

int main(void)
{
    pinMode(4, OUTPUT);
    wdt_reset();   
           
    wdt_enable(WDTO_8S);            //watch_dog инициализация 8s timer
    
    digitalWrite(4, HIGH);
    _delay_ms(100);
    digitalWrite(4, LOW); 
    //WDTCR |= (1 << WDIE);           // разрешаем прерывания по ватчдогу. Иначе будет резет.
    //sei();                          //установить флаг глобального прерывания http://www.gaw.ru/html.cgi/txt/doc/micros/avr/asm/sei.htm
    MCUCR=0x30;                     // настраиваем режим энергосбережения. POWER_DOWN_MODE
    
    while(1) {
      if (wdt_flag) {
        wdt_flag = false;
        action();
      }
      sleep_mode();
    }
}
