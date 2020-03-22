#if defined(ARDUINO_ARCH_MEGAAVR)

#include <Arduino.h>
#include "EEPROMWearLevel.h"

#ifndef NO_EEPROM_WRITES
void EEPROMWearLevel::programZeroBitsToZero(int index, byte byteWithZeros) {
  // To write to page buffer get a pointer to that location in memory...
  // on currently available megaavr parts, all have 256 or fewer b of EEPROM
  // so we make sure we don't try to write somewhere not in the EEPROM
  // only bytes changed in page buffer will be written when writing EEPROM
  byte * dataptr = (byte *) (0x1400 | (0xFF & index));

  while (NVMCTRL.STATUS & NVMCTRL_EEBUSY_bm);  //make sure EEPROM is ready
  //Now just write the byte to that location...
  *dataptr = byteWithZeros;
  //disable interrupts
  uint8_t u8SREG = SREG;
  cli();
  _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_PAGEWRITE_gc);
  SREG = u8SREG; //can reenable interrupts as soon as we do this...
  while (NVMCTRL.STATUS & NVMCTRL_EEBUSY_bm);  //wait ti be done
  //that byte has now been written successfully
}
#endif

void EEPROMWearLevel::clearByteToOnes(int index) {
  // To erase, same procedure as writing, only we write a dummy byte
  // to that location in the page buffer, which is cleared after every
  // operation, whether it is a write or an erase.
  // only bytes changed in page buffer will be erased when erasing EEPROM
  byte * dataptr = (byte *) (0x1400 | (0xFF & index));

  while (NVMCTRL.STATUS & NVMCTRL_EEBUSY_bm);
  *dataptr = 0xFF; //this is the dummy value
  uint8_t u8SREG = SREG;
  cli();
  _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_PAGEERASE_gc); //just issue different command
  SREG = u8SREG;
  while (NVMCTRL.STATUS & NVMCTRL_EEBUSY_bm);
  //that byte has now been erased successfully
}

#endif // defined(ARDUINO_ARCH_MEGAAVR)
