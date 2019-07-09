#include <EEPROM.h>

  int n = 10; //                        //количество ячеек памяти в EEPROM attiny25 - 128; attiny45 - 256; attiny85 - 512 

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
      return true;
    }
    return false;
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
void printEEPROM(int count){
  for (int i = 0; i <= count; i++){
    Serial.print(EEPROM[i]);
    Serial.print(" ");
  }
  Serial.println(" ");
}
eeprom a;
void setup() {
 
  a.clear();
  Serial.begin(9600);
  //printEEPROM(n);
  //Serial.println(returnCounter());
  //EEPROM[9] = 10;
 // printEEPROM(n);
  //Serial.println(returnCounter());
for (int i = 0; i <= 10; i++){
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
