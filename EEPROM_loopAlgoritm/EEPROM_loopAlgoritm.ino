#include <EEPROM.h>

int n = 9;

void printEEPROM(int count){
  for (int i = 0; i <= count; i++){
    Serial.print(EEPROM[i]);
    Serial.print(" ");
  }
  Serial.println(" ");
}

void clearEEPROM(){
  for (int i =0; i <= n; i++){
    EEPROM[i] = 0;
  }
}

int searchZeroEEPROM(){
  for(int i = 0; i <= n; i++){
    if (EEPROM[i] == 0){
      return i;
    }
  }
  return 0;
}

int returnCounter(){
  int zero = searchZeroEEPROM();
  if (zero == 0){
    return EEPROM[n];
  }
  return EEPROM[zero - 1];
}

void checkCounterEEPROM(int maxi = 10){
  if (returnCounter() == maxi){
    if (searchZeroEEPROM() == n){
      int zero = searchZeroEEPROM();
      EEPROM[0] = 0;
      EEPROM[zero] = 1;
    } else {
      int zero = searchZeroEEPROM();
      EEPROM[zero + 1] = 0;
      EEPROM[zero] = 1;
    }
  }
}

void counterPlusEEPROM(){
  checkCounterEEPROM();
  int zero = searchZeroEEPROM();
  if (zero == 0){
    EEPROM[n]++;
  } else{
    EEPROM[zero - 1]++;
  }
}

void setup() {
  clearEEPROM();
  Serial.begin(9600);
  //printEEPROM(n);
  //Serial.println(returnCounter());
  //EEPROM[9] = 10;
 // printEEPROM(n);
  //Serial.println(returnCounter());
}

void loop() {
  printEEPROM(n);
  counterPlusEEPROM();
}
