/*
    Copyright 2016-2016 Peter Rosenberg

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
/*
  This file is part of the EEPROMWearLevel library for Arduino.
*/

#ifndef EEPROM_WEAR_LEVEL_H
#define EEPROM_WEAR_LEVEL_H

#include <EEPROM.h>

/**
   uncomment to deactivate the range check of idx.
*/
//#define NO_RANGE_CHECK
/**
   uncomment to use an array instead of the real EEPROM for testing purpose
*/
//#define NO_EEPROM_WRITES
/**
   uncomment to write debug logs to Serial
*/
//#define DEBUG_LOG
/**
   the size of the fake eeprom if used
*/
#ifdef NO_EEPROM_WRITES
#define FAKE_EEPROM_SIZE 34
#endif

/*
   the index of the layoutVersion byte
*/
#define INDEX_VERSION 0
/**
   definition of no data, happens when no data has been written yet.
*/
#define NO_DATA -1

/**
   returned when an error occured, e.g. out of index if RANGE_CHECK is defined
*/
#define ERROR_CODE -2

class EEPROMWearLevel: EEPROMClass {
  public:
    /**
        Initialises EEPROMWearLevel. One of the begin() methods must be called
        before any other method.
        This method uses the whole EEPROM for wear leveling.
        @param layoutVersion your version of the EEPROM layout. When ever you change any value
        on the begin() method, the layoutVersion must be incremented. This will reset EEPROMWearLevel
        and initilize it and the EEPROM to the given values.
        @param amountOfIndexes the amount of indexes you want to use.
    */
    void begin(const byte layoutVersion, const int amountOfIndexes);

    /**
        Initialises EEPROMWearLevel. One of the begin() methods must be called
        before any other method.
        @param layoutVersion your version of the EEPROM layout. When ever you change any value
        on the begin() method, the layoutVersion must be incremented. This will reset EEPROMWearLevel
        and initilize it and the EEPROM to the given values.
        @param amountOfIndexes the amount of indexes you want to use.
        @param eepromLengthToUse the length of the EEPROM to use for wear leveling in case you want
        to use parts of the EEPROM for other purpose.
    */
    void begin(const byte layoutVersion, const int amountOfIndexes, const int eepromLengthToUse);

    /**
        Initialises EEPROMWearLevel. One of the begin() methods must be called
        before any other method.
        @param layoutVersion your version of the EEPROM layout. When ever you change any value
        on the begin() method, the layoutVersion must be incremented. This will reset EEPROMWearLevel
        and initialise it and the EEPROM to the given values.
        @param lengths array of the partition lengths to use on the EEPROM. The length includes the
        control bytes so the usable length for data is smaller. You can get the max length for data
        by calling getMaxDataLength().
        The array must contain amountOfIndexes entries.
        @param amountOfIndexes the amount of indexes you want to use.
    */
    void begin(const byte layoutVersion, const int lengths[], const int amountOfIndexes);

    /**
      Returns the amount of indexes that can be used. This value is defined by the begin() method.
    */
    unsigned int length();

    /**
       returns the maximum size a single element can be. If the element is larger than half of
       the maximum size, no wear levelling will be possible.
    */
    int getMaxDataLength(const int idx) const;

    /**
       reads one byte from idx
    */
    uint8_t read(const int idx);

    /**
       writes one byte if it is not the same as the last one.
    */
    void update(const int idx, const uint8_t val);

    /**
       writes one byte no matter what value was written before.
    */
    void write(const int idx, const uint8_t val);

    /**
       reads the last written value of idx or leaves t unchanged if no
       value written yet.
    */
    template< typename T > T &get(const int idx, T &t) {
      return getImpl(idx, t);
    }

    /**
       writes a new value if it is not the same as the last one
    */
    template< typename T > const T &put(const int idx, const T &t) {
      return put(idx, t, true);
    }

    /**
       writes a new value no matter what value was written before.
    */
    template< typename T > const T &putToNext(const int idx, const T &t) {
      return put(idx, t, false);
    }

    /**
        returns the first index used to store data for this idx.
        This method can be called to use EEPROMWEarLevel as a ring buffer.
    */
    int getStartIndexEEPROM(const int idx);

    /**
       returns the current read index of this idx.
        This method can be called to use EEPROMWEarLevel as a ring buffer.
    */
    int getCurrentIndexEEPROM(const int idx, int dataLength) ;

    /**
       prints the EEPROMWearLevel status to print. Use Serial to
       print to the default serial port.
    */
    void printStatus(Print &print);

    /**
       prints content of the EEPROM to print
    */
    void printBinary(Print &print, int startIndex, int endIndex);

    /**
       Constructor of the EEPROMWearLevel. Do not call this method as there
       is only one instance of EEPROMWearLevel supported.
    */
    EEPROMWearLevel();

  private:
    class EEPROMConfig {
      public:
        /**
           the first index of this EEOROMConfig what is equal to the first
           index of the control bytes
        */
        int startIndexControlBytes;
        /**
           the last index of the current data. It is equal to the start index
           if the data length is 1.
           NO_DATA (-1) for no data
        */
        int lastIndexRead;
    };

    EEPROMConfig *eepromConfig;
#ifdef NO_EEPROM_WRITES
    byte fakeEeprom[FAKE_EEPROM_SIZE];
#endif
    int amountOfIndexes;

    void init(const byte layoutVersion);

    /**
       values, update and controlBytesCount are passed in to allow comparison
       and for optimization purpose to not calculate controlBytesCount multiple times.
    */
    int getWriteStartIndex(const int idx, const int dataLength, const byte *values,
                           const bool update, const int controlBytesCount);
    /**
       controlBytesCount are passed in for optimization purpose to not calculate controlBytesCount
       multiple times.
    */
    void updateControlBytes(int idx, int newStartIndex, int dataLength, const int controlBytesCount);

    int getControlBytesCount(const int index) const;
    /**
       Finds the index by looking at the control bytes. All used bits are 0, all unused ones 1.
       controlBytesCount are passed in for optimization purpose to not calculate controlBytesCount
       multiple times.
    */
    int findIndex(const EEPROMConfig &config, const int controlBytesCount);
    /**
       find the control byte where the current index is stored. That is the first byte
       where not all bits are 0.
    */
    int findControlByteIndex(const int startIndex, const int length);
    /**
       read one byte from EEPROM.
    */
    inline byte readByte(const int index);

    /**
       set one bit to 0 without erasing the whole byte before.
    */
    void programBitToZero(int index, byte bitIndex);
    /**
       Try retryCount times if EEPROM has not changed to expected value.
    */
    void programZeroBitsToZero(int index, byte byteWithZeros, int retryCount);
    /**
       set all bits that are 0 in byteWithZeros to zero without erasing the whole byte
       and without changing the bits that are 1 in byteWithZeros.
    */
    void programZeroBitsToZero(int index, byte byteWithZeros);
    /*
       set all bits in all given bytes to one with an erase operation.
    */
    void clearBytesToOnes(int startIndex, int length);
    /**
       set all bits in the given byte to one with an erase operation.
    */
    void clearByteToOnes(int index);
    /*
       print the given byte to print in binary with adding missing zeros on the left
       and in dec after a /.
    */
    void printBinWithLeadingZeros(Print &print,  const byte value) const;

    /**
       print out of range error message to serial if DEBUG_LOG defined
    */
    void logOutOfRange(int idx) const;

    // --------------------------------------------------------
    // implementation of template methods
    // --------------------------------------------------------
    template< typename T > T &getImpl(const int idx, T &t) {
#ifndef NO_RANGE_CHECK
      if (idx >= amountOfIndexes) {
        logOutOfRange(idx);
        return t;
      }
#endif
      const int lastIndex = eepromConfig[idx].lastIndexRead;
      if (lastIndex != NO_DATA) {
        const int dataLength = sizeof(t);
        // +1 because it is the last index
        const int firstIndex = lastIndex + 1 - dataLength;
#ifndef NO_EEPROM_WRITES
        t = EEPROMClass::get(firstIndex, t);
#else
        byte *values = (byte*) &t;
        for (int i = 0; i < dataLength; i++) {
          values[i] = fakeEeprom[firstIndex + i];
        }
#endif
      } else {
#ifdef DEBUG_LOG
        Serial.println(F("no data"));
#endif
      }
      return t;
    }

    /**
       write t to idx. If update is true, writting is only done
       if the previous value was different.
    */
    template< typename T > const T &put(const int idx, const T &t, const bool update) {
#ifndef NO_RANGE_CHECK
      if (idx >= amountOfIndexes) {
        logOutOfRange(idx);
        return t;
      }
#endif
      const int dataLength = sizeof(t);
      const byte *values = (const byte*) &t;
      const int controlBytesCount = getControlBytesCount(idx);

      const int writeStartIndex = getWriteStartIndex(idx, dataLength, values, update, controlBytesCount);
      if (writeStartIndex < 0) {
        return t;
      }
#ifndef NO_EEPROM_WRITES
      EEPROMClass::put(writeStartIndex, t);
#else
      for (int i = 0; i < dataLength; i++) {
        fakeEeprom[writeStartIndex + i] = values[i];
      }
#endif
      updateControlBytes(idx, writeStartIndex, dataLength, controlBytesCount);
      return t;
    }
};


/**
   the instance of EEPROMWearLevel
*/
extern EEPROMWearLevel EEPROMwl;

#endif // #ifndef EEPROM_WEAR_LEVEL_H
