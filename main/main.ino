inline void SetupPins(){
  //         x              - Биты отвечают за назначение пинов
  //         |x             - 1 - OUTPUT; 0 - INPUT
  //         ||DDB5         - PB5
  //         |||DDB4        - PB4
  //         ||||DDB3       - PB3
  //         |||||DDB2      - PB2
  //         ||||||DDB1     - PB1
  //         |||||||DDB0    - PB0
  //         ||||||||
  DDRB = 0b00111010;
}

/*inline void SetupWatchdog(){

  // WDP3  WDP2  WDP1  WDP0 - отвечают за время срабатывания watchdog
  //  0      0     0     0  - 16ms
  //  0      1     0     0  - 25ms
  //  0      1     1     0  - 1s
  //  0      1     1     1  - 2s
  //  1      0     0     0  - 4s
  //  1      0     0     1  - 8s - maximum
  
  // WDE  WDIE  - отвечают за режимы watchdog
  //  0    0    - none
  //  0    1    - interrupt mode
  //  1    0    - reset mode
  //  1    1    - interrupt reset mode

  //          WDIF            - устнавливается 1 когда вотчдог таймер срабатывает (в режиме прерываний)
  //          |WDIE           - меняет режимы 
  //          ||WDP3          - 
  //          |||WDCE         - необходимо установить еденицу когда WDE - 0  
  //          ||||WDE         - включение watchdog
  //          |||||WDP2       - 
  //          ||||||WDP1      - 
  //          |||||||WDP0     - 
  //          ||||||||
  //WDTCR = 0b00000000;
  
  WDTCR |= (0 << WDIE);
  WDTCR |= (1 << WDE);
  WDTCR |= (1 << WDP3);
  WDTCR |= (0 << WDP2);
  WDTCR |= (0 << WDP1);
  WDTCR |= (0 << WDP0);
  
}*/
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
