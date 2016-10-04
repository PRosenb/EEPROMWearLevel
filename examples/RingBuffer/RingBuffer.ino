#include <EEPROMWearLevel.h>

#define EEPROM_LAYOUT_VERSION 4
#define AMOUNT_OF_INDEXES 1
#define INDEX_RING_BUFFER 0

void setup() {
  Serial.begin(9600);
  while (!Serial);

  EEPROMwl.begin(EEPROM_LAYOUT_VERSION, AMOUNT_OF_INDEXES, 32);

  writeData();
  EEPROMwl.printBinary(Serial, 0, 10);
  readData();
}

void loop() {
}

void writeData() {
  long value = 111;
  EEPROMwl.putToNext(INDEX_RING_BUFFER, value);

  value = 222;
  EEPROMwl.putToNext(INDEX_RING_BUFFER, value);

  value = 333;
  EEPROMwl.putToNext(INDEX_RING_BUFFER, value);
}

void readData() {
  long value = 111;
  int dataLength = sizeof(value);
  int startIndex = EEPROMwl.getStartIndexEEPROM(INDEX_RING_BUFFER);
  for (int i = 0; i * dataLength <  EEPROMwl.getMaxDataLength(INDEX_RING_BUFFER); i++) {
    Serial.print(i);
    Serial.print(F(": "));
    Serial.println(EEPROM.get(startIndex + i * dataLength, value));
  }
}
