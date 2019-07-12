#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <EEPROM.h>



int n = 126;                         //количество ячеек памяти в EEPROM attiny25 - 128; attiny45 - 256; attiny85 - 512 

class eeprom{
  public:

  void clear(){
    for (int i =0; i <= n; i++){
     EEPROM[i] = 0;
    }
  }
  int searchZero(){
    for(int i = 0; i <= n; i++){
      if (EEPROM[i] == 0){
        return i;
      }
    }
    return 0;
  }
  int returnCounter(){
    int zero = searchZero();
    if (zero == 0){
      return EEPROM[n];
    }
    return EEPROM[zero - 1];
  }
  bool checkCounter(int maxi = 10){
    if (returnCounter() == maxi){
      if (searchZero() == n){
        int zero = searchZero();
        EEPROM[0] = 0;
        EEPROM[zero] = 1;
      } else {
        int zero = searchZero();
        EEPROM[zero + 1] = 0;
        EEPROM[zero] = 1;
      }
    }
  }
  void counterPlus(){
    int zero = searchZero();
    if (zero == 0){
      EEPROM[n]++;
    } else{
      EEPROM[zero - 1]++;
    }
  }
};
eeprom a;

ISR (WDT_vect) {                      //обработка прерывания watchdog
  if (EEPROM[0] >= 4){
      EEPROM[0] = 0;
    ADCSRA = (1 << ADEN) ;              //включение АЦП (в POWER_DOWN_MODE выключен)
    digitalWrite(3, HIGH);
    digitalWrite(1, HIGH);
    if ((analogRead(2) < 300)/* && (analogRead(0) > 600)*/){
      digitalWrite(4, HIGH);
      _delay_ms(100);
      digitalWrite(4, LOW);  
    }
    ADCSRA = (0 << ADEN) ;            //выключение АЦП(потребляет о.3 мА)
    digitalWrite(3, HIGH);
    digitalWrite(1, HIGH);
    WDTCR |= (1 << WDIE);             // разрешаем прерывания по watchdog. Иначе будет ресет.
  }
  EEPROM[0] = EEPROM[0] + 1;
}

int main(void)
{
    _delay_ms(100);
    //a.clear();
    pinMode(0, INPUT);
    pinMode(1, OUTPUT);
    pinMode(2, INPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);


    wdt_reset();          
    wdt_enable(WDTO_2S);            //watch_dog инициализация 8s timer
    
    WDTCR |= (1 << WDIE);           // разрешаем прерывания по ватчдогу. Иначе будет резет.
    sei();                          //установить флаг глобального прерывания http://www.gaw.ru/html.cgi/txt/doc/micros/avr/asm/sei.htm
    MCUCR=0x30;                     // настраиваем режим энергосбережения. POWER_DOWN_MODE
   
    while(1) {
      sleep_mode();
    }
}
