#include <avr/eeprom.h>

const  int n = 10; //                        //количество ячеек памяти в EEPROM attiny25 - 128; attiny45 - 256; attiny85 - 512 

class eeprom{
  public:
  
  int Eread(int i){
    return eeprom_read_byte((uint8_t*)i);
  }
  void Ewrite(int addr,  int inf){
    eeprom_write_byte((uint8_t*) addr, (uint8_t*) inf) ;
  }
  void clear(){
    for (int i =0; i <= n; i++){
      Ewrite(i, 0);
    }
  }
  int searchZero(){
    int inf = 0;
    for(int i = 0; i <= n; i++){
      if (Eread(i) == 0){
        return i;
      }
    }
    return 0;
  }
  int returnCounter(){
    int zero = searchZero();
    if (zero == 0){
      return Eread(n);
    }
      return Eread(zero - 1);
  }
  bool checkCounter(int maxi = 10){
    if (returnCounter() == maxi){
      if (searchZero() == n){
        int zero = searchZero();
        Ewrite(0, 0);
        Ewrite(zero, 1);
      } else {
        int zero = searchZero();
        Ewrite(zero + 1, 0);
        Ewrite(zero, 1);
      }
    }
  }
  void counterPlus(){
    int zero = searchZero();
    if (zero == 0){
      Ewrite(n, Eread(n) + 1);
    } else{
      Ewrite(zero - 1, Eread(zero - 1) + 1);
    }
  }
};

void printEEPROM(int count){
  for (int i = 0; i <= count; i++){
    Serial.print(Eread(i));
    Serial.print(" ");
  }
  Serial.println(" ");
}
eeprom a;
void setup() {
 
  a.clear();
  Serial.begin(9600);
  printEEPROM(n);
  
  
for (int i = 0; i <= 1000; i++){
  printEEPROM(n);
  bool o = a.checkCounter(4);
  if (o){
   Serial.println(i);
  }
  a.counterPlus();
}
 printEEPROM(n);
}

void loop() {
  
 // printEEPROM(n);
  
  //Serial.println(a.checkCounter(4));
  //a.counterPlus();
}
