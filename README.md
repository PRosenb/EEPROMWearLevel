# Arduino EEPROMWearLevel Library #
https://github.com/PRosenb/EEPROMWearLevel

EEPROMWearLevel bases on the [EEPROM library](https://www.arduino.cc/en/Reference/EEPROM) included in the Android framework.  
It reduces EEPROM wear by writting every new value to an other EEPROM location. The current location is stored by writing a single bit for every EEPROM location. When all are used, it starts again on the first location.  
The begin() method must be called first and lets you define how many values need to be stored and how much of the EEPROM shall be used.  
  
## Features ##
- Same interface as EEPROM.h
- Easy to use
- Small footprint
- Reduces EEPROM wear
- Balances data writes inside of given area
- Uses single bit writes to store current index
- Can be used as ring buffer

## Installation ##
- The library can be installed directly in the [Arduino Software (IDE)](https://www.arduino.cc/en/Main/Software) as follows:
  - Menu Sketch->Include Library->Manage Libraries...
  - On top right in "Filter your search..." type: EEPROMWearLevel
  - The EEPROMWearLevel library will show
  - Click on it and then click "Install"
  - For more details see manual [Installing Additional Arduino Libraries](https://www.arduino.cc/en/Guide/Libraries#toc3)
- If you do not use the [Arduino Software (IDE)](https://www.arduino.cc/en/Main/Software):
  - [Download the latest version](https://github.com/PRosenb/EEPROMWearLevel/releases/latest)
  - Uncompress the downloaded file
  - This will result in a folder containing all the files for the library. The folder name includes the version: **EepromWearLevel-x.y.z**
  - Rename the folder to **EEPROMWearLevel**
  - Copy the renamed folder to your **libraries** folder
  - From time to time, check on https://github.com/PRosenb/EEPROMWearLevel if updates become available

## Getting Started ##
Simple configuration:
```c++
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
```

## Examples ##
The following example sketches are included in the **EepromWearLevel** library.  
You can also see them in the [Arduino Software (IDE)](https://www.arduino.cc/en/Main/Software) in menu File->Examples->EEPROMWearLevel.
- [**SimpleConfiguration**](https://github.com/PRosenb/EEPROMWearLevel/blob/master/examples/SimpleConfiguration/SimpleConfiguration.ino): Simple example.
- [**RingBuffer**](https://github.com/PRosenb/EEPROMWearLevel/blob/master/examples/RingBuffer/RingBuffer.ino): Ring buffer example.

## Reference ##
### Methods ###
```c++
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
        and initilize it and the EEPROM to the given values.
        @param lengths array of lengts to use on the EEPROM. The amount of indexes will be equals
        to the entries in lenths.
    */
    void begin(const byte layoutVersion, const int lengths[]);
    
     /**
       Returns the amount of indexes that can be used. This value is defined by the begin() method.
    */
    unsigned int length();

    /**
       returns the maximum size a single element can be. If the element is larger than half of
       the maximum size, no wear levelling will be possible.
    */
    int getMaxDataLength(const int idx);

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
    template< typename T > T &get(const int idx, T &t);
    
    /**
       writes a new value if it is not the same as the last one
    */
    template< typename T > const T &put(const int idx, const T &t);
    
    /**
       writes a new value no matter what value was written before.
    */
    template< typename T > const T &putToNext(const int idx, const T &t);
    
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
       prints the EEPROMWearLevel status to Serial.
    */
    void printStatus();
    
    /**
       prints content of the EEPROM to Serial
    */
    void printBinary(int startIndex, int endIndex);
```

## Implementation Notes ##
### Control Bytes ###
EEPROMWearLevel uses single bits to store the current index. Single bits of an EEPROM byte can only be programmed from 1 to 0. If a bit needs to change from 0 to 1, the whole byte must be cleared.  
EEPROMWearLevel uses control bytes to store the location of the current index of the data. Every bit stands for one byte of data. A bit as 0 stands for data is in use, 1 for not yet used.

An empty control byte looks like this:

    Index:              01234567
    Bit representation: 11111111
    
Writing data of 2 bytes length changes it to (2 bits are programmed to 0 without clearing the whole byte):

    Index:              01234567
    Bit representation: 00111111
    
The control byte above states, that the first two indexes are used for data, the rest if free.  
If you use larger partitions, the same is done with multiple control bytes. When all of them are marked as used (all bits 0), all bits in all control bytes of the partition are cleared what sets them back to 1.

### EEPROM layout ###
EEPROMWearLevel uses one partition for every idx you use. A partition with a length of 18 bytes is represented as follows:

    EEPROM index: 0             1            2            3          4          .. 18
    Description:  layoutVersion controlByte0 controlByte1 dataIndex0 dataIndex1 .. dataIndex15
    
In this example, two of the 18 bytes in the partition are used as control bytes while the other 16 is used to store the data.
    
The 'layoutVersion' is used to clear control bytes when their position on the EEPROM is changed by using other arguments on the method 'begi()'. It is therefore important to change the 'layoutVersion' when ever a change is made of the arguments of the 'begin()' method. A change of 'layoutVersion' causes EEPROMWearLevel to reset the required control bytes so that it can use them to store the indexes.

## Contributions ##
Enhancements and improvements are welcome.

## License ##
```
Arduino EEPROMWearLevel Library
Copyright (c) 2016 Peter Rosenberg (https://github.com/PRosenb).

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```
