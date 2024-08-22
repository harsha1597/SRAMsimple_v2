/*  SRAMsimple.h - Library for reading and writing data from an Arduino Uno to a 23LC1024 chip.
 *  Original sketch created by J. B. Gallaher on 07/09/2016. 
 *  Library created by David Dubins, November 12th, 2018.
 *  Released into the public domain.
 */

#ifndef SRAMsimple_h
#define SRAMsimple_h

#include <Arduino.h>
#include <SPI.h>
#include "mbed.h"
/************SRAM opcodes: commands for the 23AA04M SRAM memory chip ******************/
#define RDSR        0x05000000 // Read status register
#define WRSR        0x01000000
#define READ        0x03000000 
#define HSREAD      0x0B000000 // High Speed Read
#define WRITE       0x02000000
#define Sequential  0x00401400  // Sequential mode (read/write blocks of memory)
#define ByteMode    0x00001400    // Byte mode (read/write one byte at a time)
#define PageMode    0x00801400 // Page Mode

extern byte CS;		    // Global variable for CS pin (default 10)

using namespace mbed;

SPI spi(PC_3, PC_2, PI_1);            // MOSI,MISO,SCK, (CS not added here as it results in unexpected behaviour)
spi.frequency(100000000);             // Set up your frequency.
spi.format(32, 0);                  // Messege length (bits), SPI_MODE - check these in your SPI decice's data sheet.
  

class SRAMsimple {
  public:
    SRAMsimple();
    ~SRAMsimple();
    void SetMode(byte CSpin, char Mode);
    void ReadMode();
    void WriteByte(uint32_t address, byte data_byte);
    byte ReadByte(uint32_t address);
    void WriteByteArray(uint32_t address, byte *data, uint16_t big);
    void ReadByteArray(uint32_t address, byte *data, uint16_t big);
    void WriteInt(uint32_t address, int data);  
    int ReadInt(uint32_t address);
    void WriteIntArray(uint32_t address, int *data, uint16_t big);
    void ReadIntArray(uint32_t address, int *data, uint16_t big);
    void WriteUnsignedInt(uint32_t address, unsigned int data);
    unsigned int ReadUnsignedInt(uint32_t address);
    void WriteUnsignedIntArray(uint32_t address, unsigned int *data, uint16_t big);
    void ReadUnsignedIntArray(uint32_t address, unsigned int *data, uint16_t big);
    void WriteLong(uint32_t address, long data);
    long ReadLong(uint32_t address);
    void WriteLongArray(uint32_t address, long *data, uint16_t big);
    void ReadLongArray(uint32_t address, long *data, uint16_t big);
    void WriteUnsignedLong(uint32_t address, unsigned long data);
    unsigned long ReadUnsignedLong(uint32_t address);
    void WriteUnsignedLongArray(uint32_t address, unsigned long *data, uint16_t big);
    void ReadUnsignedLongArray(uint32_t address, unsigned long *data, uint16_t big);
    void WriteFloat(uint32_t address, float data);
    float ReadFloat(uint32_t address);
    void WriteFloatArray(uint32_t address, float *data, uint16_t big);
    void ReadFloatArray(uint32_t address, float *data, uint16_t big);
};

#endif
