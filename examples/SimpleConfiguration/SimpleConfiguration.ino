#include <EEPROMWearLevel.h>

#define EEPROM_LAYOUT_VERSION 0
#define AMOUNT_OF_INDEXES 2

#define INDEX_CONFIGURATION_VAR1 0
#define INDEX_CONFIGURATION_VAR2 1

void setup() {
  Serial.begin(9600);
  while (!Serial);

  EEPROMwl.begin(EEPROM_LAYOUT_VERSION, AMOUNT_OF_INDEXES);

  writeConfiguration();
  readConfiguration();
}

void loop() {
}

void writeConfiguration() {
  // write a byte
  EEPROMwl.update(INDEX_CONFIGURATION_VAR1, 12);

  long var2 = 33333;
  EEPROMwl.put(INDEX_CONFIGURATION_VAR2, var2);
}

void readConfiguration() {
  byte var1 = EEPROMwl.read(INDEX_CONFIGURATION_VAR1);
  Serial.print(F("var1: "));
  Serial.println(var1);

  long var2 = -1;
  EEPROMwl.get(INDEX_CONFIGURATION_VAR2, var2);
  Serial.print(F("var2: "));
  Serial.println(var2);
}

