#if defined(ARDUINO_ARCH_AVR)

#include <Arduino.h>
#include "EEPROMWearLevel.h"

#ifndef NO_EEPROM_WRITES
void EEPROMWearLevel::programZeroBitsToZero(int index, byte byteWithZeros) {
  // EEPROM Mode Bits.
  // EEPM1.0 = 0 0 - Mode 0 Erase & Write in one operation.
  // EEPM1.0 = 0 1 - Mode 1 Erase only.
  // EEPM1.0 = 1 0 - Mode 2 Write only.
  EECR |= 1 << EEPM1;
  EECR &= ~(1 << EEPM0);
  // EEPROM Ready Interrupt Enable.
  // EERIE = 0 - Interrupt Disable.
  // EERIE = 1 - Interrupt Enable.
  EECR &= ~(1 << EERIE);

  // Set EEPROM address - 0x000 - 0x3FF.
  EEAR = index;
  // Data write into EEPROM.
  EEDR = byteWithZeros;

  uint8_t u8SREG = SREG;
  cli();
  // Wait for completion previous write.
  while (EECR & (1 << EEPE));
  // EEMPE = 1 - Master Write Enable.
  EECR |= (1 << EEMPE);
  // EEPE = 1 - Write Enable.
  EECR  |=  (1 << EEPE);
  SREG = u8SREG;

  // Wait for completion of write.
  while (EECR & (1 << EEPE));
}
#endif

void EEPROMWearLevel::clearByteToOnes(int index) {
  // EEPROM Mode Bits.
  // EEPM1.0 = 0 0 - Mode 0 Erase & Write in one operation.
  // EEPM1.0 = 0 1 - Mode 1 Erase only.
  // EEPM1.0 = 1 0 - Mode 2 Write only.
  // set EEPM0
  EECR |= 1 << EEPM0;
  // clear EEPM1
  EECR &= ~(1 << EEPM1);
  // EEPROM Ready Interrupt Enable.
  // EERIE = 0 - Interrupt Disable.
  // EERIE = 1 - Interrupt Enable.
  EECR &= ~(1 << EERIE);

  // Set EEPROM address - 0x000 - 0x3FF.
  EEAR = index;//0x000;

  uint8_t u8SREG = SREG;
  cli();
  // Wait for completion previous write.
  while (EECR & (1 << EEPE));
  // EEMPE = 1 - Master Write Enable.
  EECR |= (1 << EEMPE);
  // EEPE = 1 - Write Enable.
  EECR  |=  (1 << EEPE);
  SREG = u8SREG;
}

#endif // defined(ARDUINO_ARCH_AVR)
