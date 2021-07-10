#include <Arduino.h>
#include "EEPROMWearLevel.h"

EEPROMWearLevel EEPROMwl;

EEPROMWearLevel::EEPROMWearLevel() {
	amountOfIndexes = 0;
#ifdef NO_EEPROM_WRITES
	for (int i = 0; i < FAKE_EEPROM_SIZE; i++) {
		fakeEeprom[i] = 0xFF;
	}
#endif
}

void EEPROMWearLevel::begin(const byte layoutVersion, const int amountOfIndexes) {
	begin(layoutVersion, amountOfIndexes, EEPROMClass::length());
}

void EEPROMWearLevel::begin(const byte layoutVersion, const int amountOfIndexes, const int eepromLengthToUse) {
	int startIndex = 1; // index 0 reserved for the version
	EEPROMWearLevel::amountOfIndexes = amountOfIndexes;
	// +1 to store a place holder element in the last
	// place to get the lenth of the last element
	eepromConfig = new EEPROMConfig[amountOfIndexes + 1];
	const int singleLength = eepromLengthToUse / amountOfIndexes;
	int index;
	for (index = 0; index < amountOfIndexes; index++) {
		eepromConfig[index].startIndexControlBytes = startIndex;
		startIndex += singleLength;
	}
	// the last one as a placeholder to calculate the length of the last real element
	eepromConfig[index].startIndexControlBytes = startIndex;

	init(layoutVersion);
}

void EEPROMWearLevel::begin(const byte layoutVersion, const int lengths[], const int amountOfIndexes) {
	int startIndex = 1; // index 0 reserved for the version
	EEPROMWearLevel::amountOfIndexes = amountOfIndexes;
	// +1 to store a place holder element in the last
	// place to get the lenth of the last element
	eepromConfig = new EEPROMConfig[amountOfIndexes + 1];
	int index;
	for (index = 0; index < amountOfIndexes; index++) {
		eepromConfig[index].startIndexControlBytes = startIndex;
		startIndex += lengths[index];
	}
	// the last one as a placeholder to calculate the length of the last real element
	eepromConfig[index].startIndexControlBytes = startIndex;

	init(layoutVersion);
}

void EEPROMWearLevel::init(const byte layoutVersion) {
	const byte previousVersion = readByte(INDEX_VERSION);
#ifndef NO_EEPROM_WRITES
	EEPROMClass::update(INDEX_VERSION, layoutVersion);
#else
	fakeEeprom[INDEX_VERSION] = layoutVersion;
#endif

	// -1 because the last one is a placeholder
	int index;
	for (index = 0; index < amountOfIndexes; index++) {
		const int controlBytesCount = getControlBytesCount(index);
		if (layoutVersion != previousVersion) {
			clearBytesToOnes(eepromConfig[index].startIndexControlBytes, controlBytesCount);
		}

		eepromConfig[index].lastIndexRead = findIndex(eepromConfig[index], controlBytesCount);
	}
	// the last one as a placeholder to calculate the length of the last real element
	eepromConfig[index].lastIndexRead = NO_DATA;

	// prevent warning about not using EEPROM
	(void)EEPROM;
}

unsigned int EEPROMWearLevel::length() {
	return amountOfIndexes;
}

int EEPROMWearLevel::getMaxDataLength(const int idx) const {
#ifndef NO_RANGE_CHECK
	if (idx >= amountOfIndexes) {
		logOutOfRange(idx);
		return 0;
	}
#endif
	return eepromConfig[idx + 1].startIndexControlBytes
	       - eepromConfig[idx].startIndexControlBytes
	       - getControlBytesCount(idx);
}

uint8_t EEPROMWearLevel::read(const int idx) {
#ifndef NO_RANGE_CHECK
	if (idx >= amountOfIndexes) {
		logOutOfRange(idx);
		return 0;
	}
#endif
	uint8_t value = 0;
	return get(idx, value);
}

void EEPROMWearLevel::update(const int idx, const uint8_t val) {
	put(idx, val, true);
}

void EEPROMWearLevel::write(const int idx, const uint8_t val) {
	put(idx, val, false);
}

int EEPROMWearLevel::getStartIndexEEPROM(const int idx) {
#ifndef NO_RANGE_CHECK
	if (idx >= amountOfIndexes) {
		logOutOfRange(idx);
		return ERROR_CODE;
	}
#endif
	return eepromConfig[idx].startIndexControlBytes + getControlBytesCount(idx);
}

int EEPROMWearLevel::getCurrentIndexEEPROM(const int idx, int dataLength) {
#ifndef NO_RANGE_CHECK
	if (idx >= amountOfIndexes) {
		logOutOfRange(idx);
		return ERROR_CODE;
	}
#endif
	// +1 because lastIndexRead is the last index of the current element
	return eepromConfig[idx].lastIndexRead + 1 - dataLength;
}

int EEPROMWearLevel::getWriteStartIndex(const int idx, const int dataLength, const byte *values, const bool update, const int controlBytesCount) {
	if (dataLength > getMaxDataLength(idx)) {
#ifdef DEBUG_LOG
		Serial.print(F("dataLength too long. Max: "));
		Serial.print(getMaxDataLength(idx));
		Serial.print(F(", is: "));
		Serial.println(dataLength);
#endif
		return -2;
	}

	EEPROMConfig &config = eepromConfig[idx];
	int previousLastIndex = config.lastIndexRead;
	if (previousLastIndex == NO_DATA) {
		// no data yet so we set the index to one before the first index
		previousLastIndex = config.startIndexControlBytes + controlBytesCount - 1;
	}
	if (update) {
		boolean equal = true;
		const int previousStartIndex = previousLastIndex - (dataLength - 1);
		for (int i = 0; previousStartIndex + i <= previousLastIndex; i++) {
			if (readByte(previousStartIndex + i) != values[i]) {
				equal = false;
				break;
			}
		}
		if (equal) {
#ifdef DEBUG_LOG
			Serial.println(F("value is equal, do not write it"));
#endif
			return -3;
		}
	}

	int newStartIndex = previousLastIndex + 1;
	// eepromConfig[idx + 1].startIndexControlBytes is the first
	// index of the next one. The last one has a placehoder for
	// this purpose.
	// not >= because newAbsoluteIndex is the first write position
	if (newStartIndex + dataLength > eepromConfig[idx + 1].startIndexControlBytes) {
		// last bit is not set
		// all used, start again
#ifdef DEBUG_LOG
		Serial.println(F("all used, start again"));
#endif
		clearBytesToOnes(config.startIndexControlBytes, controlBytesCount);
		newStartIndex = config.startIndexControlBytes + controlBytesCount;
	}
	return newStartIndex;
}

void EEPROMWearLevel::updateControlBytes(int idx, int newStartIndex, int dataLength, const int controlBytesCount) {
	EEPROMConfig &config = eepromConfig[idx];
	// -1 because it is the last index
	config.lastIndexRead = newStartIndex + dataLength - 1;
	const int startIndexData = config.startIndexControlBytes + controlBytesCount;
	const int startIndexRelative = newStartIndex - startIndexData;
	int controlByteIndex = startIndexRelative / 8;
	byte newBitPosInControlByte = startIndexRelative - controlByteIndex * 8;

	// unset
	byte writeMask = 0xFF;
	for (int i = 0; i < dataLength; i++) {
		writeMask  &= ~(1 << (7 - newBitPosInControlByte));
		newBitPosInControlByte++;
		if (newBitPosInControlByte > 7) {
			// write and go to next byte
			programZeroBitsToZero(config.startIndexControlBytes + controlByteIndex, writeMask, 2);
			writeMask = 0xFF;
			newBitPosInControlByte = 0;
			controlByteIndex++;
		}
	}
	if (writeMask != 0xFF) {
		programZeroBitsToZero(config.startIndexControlBytes + controlByteIndex, writeMask, 2);
	}
}

void EEPROMWearLevel::printStatus(Print &print) {
	print.println(F("EEPROMWearLevel status: "));
	for (int index = 0; index < amountOfIndexes; index++) {
		const int controlBytesCount = getControlBytesCount(index);
		print.print(index);
		print.print(F(": "));
		print.print(eepromConfig[index].startIndexControlBytes);
		print.print(F("-"));
		print.print(eepromConfig[index + 1].startIndexControlBytes);
		print.print(F(" with "));
		print.print(controlBytesCount);
		print.print(F(" ctrl bytes at "));
		print.println(eepromConfig[index].lastIndexRead);
	}
}

void EEPROMWearLevel::printBinary(Print &print, int startIndex, int endIndex) {
	for (int i = startIndex; i <= endIndex; i++) {
		const byte value = readByte(i);
		print.print(i);
		print.print(F(":"));
		printBinWithLeadingZeros(print, value);
		print.print(F("/"));
		print.print(value);
		print.print(F(" "));
		// +1 to put line break before 10, 20..
		if ((i + 1) % 10 == 0) {
			print.println();
		}
	}
	print.println();
}

int EEPROMWearLevel::getControlBytesCount(const int idx) const {
	const int length = eepromConfig[idx + 1].startIndexControlBytes - eepromConfig[idx].startIndexControlBytes;
	// Every byte of stored user data is controlled by one bit in the control bytes.
	// Therefore, one byte of user data uses 1 byte for the data + 1 bit in the control bytes.
	// That is 8 bits for the data byte and 1 bit in the control bytes, summed up to 9 bits in total within the partition.
	// Examples:
	// - Partition with 9 bytes:
	//   1 byte as control byte (8 bits as control bits), 8 bytes for data.
	// - Partition with 17 bytes:
	//   2 bytes as control bytes (2x 8 bits = 16 bits = 15 bits as control bits + 1 bit unused), 15 bytes for data.
	// - Partition with 79 bytes:
	//   9 bytes as control bytes (9x 8 bits = 72 bits = 70 bits as control bits + 2 bits unused), 70 bytes for data.
	if ((length % 9) == 0) {
		// div exact
		return length / 9;
	} else {
		// div not exact so we need to round up
		return length / 9 + 1;
	}
}

int EEPROMWearLevel::findIndex(const EEPROMConfig &config, const int controlBytesCount) {
	const int startIndexData = config.startIndexControlBytes + controlBytesCount;
	const int controlByteIndex = findControlByteIndex(config.startIndexControlBytes, controlBytesCount);
	const byte currentByte = readByte(controlByteIndex);

	byte mask = 1;
	int bitPosInByte = 7;
	// do while bit set and bit still in mask
	while ((currentByte & mask) != 0 && mask != 0) {
		mask <<= 1;
		bitPosInByte--;
	}

	const int controlByteIndexRelative = controlByteIndex - config.startIndexControlBytes;
	const int amountOfWholeBytes = controlByteIndexRelative;
	const int indexRelative = amountOfWholeBytes * 8 + bitPosInByte;
#ifdef DEBUG_LOG
	Serial.print(F("findIndex: start ctrl bytes: "));
	Serial.print(config.startIndexControlBytes);
	Serial.print(F(": relative ctrl byte: "));
	Serial.print(controlByteIndexRelative);
	Serial.print(F(", bit in ctrl byte: "));
	Serial.print(bitPosInByte);
	Serial.print(F(", calculated index: "));
	Serial.println(startIndexData + indexRelative);
#endif
	if (indexRelative == -1) {
		// not data yet
		return NO_DATA;
	} else {
		return startIndexData + indexRelative;
	}
}

/**
   returns the index of the control byte that contains the bit which
   points to the next write position.
   The index is inside of control bytes even if all used.
   Does a binary search to find the index.
 */
int EEPROMWearLevel::findControlByteIndex(const int startIndex, const int length) {
	const int endIndex = startIndex + length - 1;
	int lowerBound = startIndex;
	int upperBound = endIndex;
	int midPoint = -1;

	byte currentByte = 0xFF;
	while (lowerBound <= upperBound) {
		// compute the mid point
		midPoint = lowerBound + (upperBound - lowerBound) / 2;

		currentByte = readByte(midPoint);
		if (currentByte != 0 && currentByte != 0xFF) {
			// searched control byte found because it is neigher 0 nor 0xFF what means
			// that some of its bits are in use, some not
			return midPoint;
		} else {
			if (currentByte == 0) {
				// searched control byte is in upper half
				lowerBound = midPoint + 1;
			} else {
				// searched control byte is in lower half
				upperBound = midPoint - 1;
			}
		}
	}

	if (currentByte == 0) {
		// searched control byte is above
		int controlByteIndex = midPoint;
		while (currentByte == 0 && controlByteIndex < endIndex) {
			controlByteIndex++;
			currentByte = readByte(controlByteIndex);
		}
		return controlByteIndex;
	} else {
		// currentByte == 0xff so searched control byte is below
		int controlByteIndex = midPoint;
		while (currentByte == 0xFF && controlByteIndex >= startIndex) {
			controlByteIndex--;
			currentByte = readByte(controlByteIndex);
		}
		// we want the byte where all bits are still set
		// except when it is the last one.
		if (controlByteIndex >= endIndex) {
			// do not go to next because it is the last one
			return controlByteIndex;
		} else {
			// go to next as current control byte is 0
			return controlByteIndex + 1;
		}
	}
}

inline byte EEPROMWearLevel::readByte(const int index) {
#ifndef NO_EEPROM_WRITES
	return EEPROMClass::read(index);
#else
	return fakeEeprom[index];
#endif
}


// http://www.tronix.io/data/avratmega/eeprom/
void EEPROMWearLevel::programBitToZero(int index, byte bitIndex) {
	byte value = 0xFF;
	value &= ~(1 << (7 - bitIndex));
	programZeroBitsToZero(index, value);
}

void EEPROMWearLevel::programZeroBitsToZero(int index, byte byteWithZeros, int retryCount) {
	do {
#ifdef DEBUG_LOG
		Serial.print(F("programZeroBitsToZero: , index: "));
		Serial.print(index);
		Serial.print(F(", byteWithZeros: "));
		printBinWithLeadingZeros(byteWithZeros);
		Serial.println();
#endif
		programZeroBitsToZero(index, byteWithZeros);
		retryCount--;
#ifdef DEBUG_LOG
		Serial.print(F("EEPROM is: "));
		printBinWithLeadingZeros(readByte(index));
		Serial.println();
#endif
		// byteWithZeros ^ 0xFF inverts all bits of byteWithZeros
	} while (retryCount > 0 && (readByte(index) & (byteWithZeros ^ 0xFF)) != 0);
}

#ifdef NO_EEPROM_WRITES
// emulate EEPROM behaviour to program only bits that are 0
void EEPROMWearLevel::programZeroBitsToZero(int index, byte byteWithZeros) {
	byte mask = 1;
	int bitPosInByte = 7;
	// do while bit still in mask
	while (mask != 0) {
		if ((byteWithZeros & mask) == 0) {
			fakeEeprom[index] &= ~(1 << (7 - bitPosInByte));
		}
		mask <<= 1;
		bitPosInByte--;
	}
}
#endif

void EEPROMWearLevel::clearBytesToOnes(int fromIndex, int length) {
	for (int i = fromIndex; i < fromIndex + length; i++) {
		if (readByte(i) != 0xFF) {
#ifndef NO_EEPROM_WRITES
			clearByteToOnes(i);
#else
			fakeEeprom[i] = 0xFF;
#endif
#ifdef DEBUG_LOG
			Serial.print(F("clear byte: "));
			Serial.println(i);
#endif
		}
	}
}

void EEPROMWearLevel::printBinWithLeadingZeros(Print &print, const byte value) const {
	byte mask = 1 << 7;
	for (int bitPosInByte = 0; bitPosInByte < 8; bitPosInByte++) {
		print.print((value & mask) != 0 ? 1 : 0);
		mask >>= 1;
	}
}

void EEPROMWearLevel::logOutOfRange(__attribute__((unused)) int idx) const {
#ifdef DEBUG_LOG
	Serial.print(F("idx out of range: "));
	Serial.print(idx);
	Serial.print(F(" of "));
	Serial.println(amountOfIndexes);
#endif
}
