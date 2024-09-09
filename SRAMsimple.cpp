/*  SRAMsimple.cpp - Library for reading and writing data from an Arduino Uno to a 23LC1024 chip.
 *  Original sketch created by J. B. Gallaher on 07/09/2016. 
 *  Library created by David Dubins, November 12th, 2018.
 *  Released into the public domain.
 */
// #include "mbed.h"
// #include "Arduino.h"

#include "SRAMsimple.h"

using namespace mbed;
#define CS 0x80

SRAMsimple::SRAMsimple(SPI& spi_param) : _spi(spi_param)
{
  // SPI _spi(PC_3, PC_2, PI_1);           // MOSI,MISO,SCK, (CS not added here as it results in unexpected behaviour)
  _spi.frequency(60000000);             // Set up your frequency.
  _spi.format(32, 0);  // Message length (bits), SPI_MODE - check these in your SPI decice's data sheet.
}
SRAMsimple::~SRAMsimple(){/*nothing to destruct*/}

// byte CS=PI_0; // default CS global variable

/*  Set up the memory chip to either single byte or sequence of bytes mode **********/
void SRAMsimple::SetMode(uint32_t Mode){            // Select for single or multiple byte transfer
  
  digitalWrite(CS,LOW);
  _spi.write((uint32_t)WRSR | Mode); // command to write to Status register
  digitalWrite(CS,HIGH);
}

void SRAMsimple::ReadMode(){            // Select for single or multiple byte transfer
  uint32_t read_word;
  digitalWrite(CS,LOW);
  read_word = _spi.write((uint32_t) RDSR); // 1 Byte instruction + 3 Byte Wait cycles
  digitalWrite(CS,HIGH);
  read_word = (uint16_t) (read_word >> 8 ); 
  
}

/************ Byte transfer functions ***************************/
void SRAMsimple::WriteWord(uint32_t address, uint32_t data_byte) {
  
  digitalWrite(CS, LOW);                         // set SPI slave select LOW;
  _spi.write((uint32_t)WRITE | address);
  _spi.write((uint32_t)data_byte);                      // write the data to the memory location
  digitalWrite(CS, HIGH);  

  Serial.print("Writing the value at");
  Serial.print(address);
  Serial.print(" : ");
  Serial.println((uint32_t)data_byte);   
}

uint32_t SRAMsimple::ReadWord(uint32_t address) {
  
  uint32_t read_word;
  digitalWrite(CS,LOW);
  _spi.write((uint32_t)READ | address);// 1 Byte instruction
  read_word = _spi.write((uint32_t)0);
  digitalWrite(CS,HIGH);

  Serial.print("Reading the value at");
  Serial.print(address);
  Serial.print(" : ");
  Serial.println((uint32_t)read_word);
  // read_word = (uint8_t) (read_word >> 24 ); //Data recieved MSB first
  
  return read_word;                              // send data back to the calling function
}

// SPI SRAM Malloc
uint32_t SRAMsimple::SRAMMalloc(size_t size) {
    if (SRAM_ADDR + size >= SRAM_SIZE) {
        // No more memory available in external SRAM
        return 0xFFFFFFFF; 
    }

    uint32_t allocated_address = SRAM_ADDR;
    SRAM_ADDR += size;
    return allocated_address;
}
// Writes a byte array into SRAM at a specific location
void SRAMsimple::SpiWriteByteArray(uint32_t address, uint8_t *data, size_t size)
{  
    digitalWrite(CS,LOW);
    spi.write((uint8_t)WRITE);
    spi.write((uint8_t)((address >> 16)& 0xFF));
    spi.write((uint8_t)((address >> 8)& 0xFF));
    spi.write((uint8_t)address);
    for(size_t i=0;i<size;i++)
    {
      spi.write(data[i]);
    }
    
    digitalWrite(CS,HIGH);
}
//uint16_t value;

// Reads bytes into an array
void SRAMsimple::SpiReadByteArray(uint32_t address, uint16_t size, uint8_t* readarray)
{

    
    digitalWrite(CS,LOW);
    spi.write((uint8_t)READ);
    spi.write((uint8_t)((address >> 16)& 0xFF));
    spi.write((uint8_t)((address >> 8)& 0xFF));
    spi.write((uint8_t)address);
    for(int i=0;i<size;i++)
    {
      readarray[i] = spi.write((uint8_t)0);
    }
    digitalWrite(CS,HIGH);
    
}

// Reads file in SD card in chunks into a buffer and writes it into the SD card
void SRAMsimple::WriteFileInChunks(const char* filepath, size_t chunk_size = 32*100) {
    
    FILE *file = fopen(filepath, "r");

    // Buffer to hold each chunk
    uint8_t buffer[chunk_size];

    // fread returns chunk size
    size_t bytesRead;
    uint32_t file_size=0;
    uint32_t address=0;
    uint32_t file_address;

    if (file == NULL) {
        Serial.println("Failed to open file.");
        return ;
    }
    
    while ((bytesRead = fread(buffer, 1, chunk_size, file)) > 0) {

        address = SRAMMalloc(bytesRead);

        if (address!= 0xFFFFFFFF){
            if (file_size==0)
            {
                // Save initial file address
                file_address = address;
                
            }
        
        SpiWriteByteArray(address,buffer,bytesRead);
        file_size+=bytesRead;
        }
    }
    
    // Close the file after reading
    fclose(file);
    model_data.address = address;
    model_data.size = file_size;
    
    Serial.println("File reading completed.");
    
}


// void SRAMsimple::ReadByteArray(uint32_t address, byte *data, uint16_t big){
//   SetMode(CS,Sequential);                         // set to send/receive multiple bytes of data
//   digitalWrite(CS, LOW);                          // start new command sequence
//   _spi.write(READ);                             // send READ command
//   _spi.write((byte)(address >> 16));            // send high byte of address
//   _spi.write((byte)(address >> 8));             // send middle byte of address
//   _spi.write((byte)address);                    // send low byte of address
//   for(uint16_t i=0; i<big; i++){
//     data[i] = _spi.write(0x00);                 // read the data byte
//   }
//   digitalWrite(CS, HIGH);                         // set SPI slave select HIGH
// }

// /*********** Write and Read an Integer (2 bytes) ************************/
// void SRAMsimple::WriteInt(uint32_t address, int data){
//   SetMode(CS,Sequential);                         // set to send/receive multiple bytes of data
//   byte temp[2]; 				  // temporary array of bytes with 2 elements
//   temp[0]=(byte)(data>>8);                        // high byte of integer
//   temp[1]=(byte)(data);                           // low byte of integer
//   digitalWrite(CS, LOW);                          // start new command sequence
//   _spi.write(WRITE);                            // send WRITE command
//   _spi.write((byte)(address >> 16));            // send high byte of address
//   _spi.write((byte)(address >> 8));             // send middle byte of address
//   _spi.write((byte)address);                    // send low byte of address
//   _spi.write(temp, 2);                          // transfer an array of data => needs array name & size (2 elements)
//   digitalWrite(CS, HIGH);                         // set SPI slave select HIGH
// }

// int SRAMsimple::ReadInt(uint32_t address){
//   SetMode(CS,Sequential);                         // set to send/receive multiple bytes of data
//   byte temp[2]; 				  // temporary array of bytes with 2 elements
//   int data=0;
//   digitalWrite(CS, LOW);                          // start new command sequence
//   _spi.write(READ);                             // send READ command
//   _spi.write((byte)(address >> 16));            // send high byte of address
//   _spi.write((byte)(address >> 8));             // send middle byte of address
//   _spi.write((byte)address);                    // send low byte of address
//   for(uint16_t i=0; i<2; i++){
//     temp[i] = _spi.write(0x00);                 // read the data byte
//   }
//   digitalWrite(CS, HIGH);                         // set SPI slave select HIGH
//   data=((int)temp[0]<<8)+temp[1];                      // data=high byte & low byte
//   return data;
// }

// /*********** Write and Read an Integer Array ************************/
// void SRAMsimple::WriteIntArray(uint32_t address, int *data, uint16_t big){
//   SetMode(CS,Sequential);                         // set to send/receive multiple bytes of data
//   byte temp[big*2]; 				  // temporary array of bytes with 2 elements
//   uint16_t j=0;                                   // byte counter
//   for(uint16_t i=0; i<big; i++){                   // take apart integers into high and low bytes
//     temp[j]=(byte)(data[i]>>8);                   // high byte of integer
//     temp[j+1]=(byte)(data[i]);                    // low byte of integer
//     j+=2;                                         // increment counter
//   }
//   digitalWrite(CS, LOW);                          // start new command sequence
//   _spi.write(WRITE);                            // send WRITE command
//   _spi.write((byte)(address >> 16));            // send high byte of address
//   _spi.write((byte)(address >> 8));             // send middle byte of address
//   _spi.write((byte)address);                    // send low byte of address
//   _spi.write(temp, big*2);                      // transfer an array of data => needs array name & size (2 elements)
//   digitalWrite(CS, HIGH);                         // set SPI slave select HIGH
// }

// void SRAMsimple::ReadIntArray(uint32_t address, int *data, uint16_t big){
//   SetMode(CS,Sequential);                         // set to send/receive multiple bytes of data
//   byte temp[big*2]; 				  // temporary array of bytes with 2 elements
//   uint16_t j=0;                                   // byte counter
//   digitalWrite(CS, LOW);                          // start new command sequence
//   _spi.write(READ);                             // send READ command
//   _spi.write((byte)(address >> 16));            // send high byte of address
//   _spi.write((byte)(address >> 8));             // send middle byte of address
//   _spi.write((byte)address);                    // send low byte of address
//   for(uint16_t i=0; i<(big*2); i++){
//     temp[i] = _spi.write(0x00);                 // read the data byte
//   }
//   digitalWrite(CS, HIGH);                         // set SPI slave select HIGH
//   for(uint16_t i=0; i<big; i++){
//     data[i]=((int)temp[j]<<8)+temp[j+1];          // data=high byte & low byte
//     j+=2;                                         // increment counter
//   }
// }

// /*********** Write and Read an Unsigned Integer (2 bytes) ************************/
// void SRAMsimple::WriteUnsignedInt(uint32_t address, unsigned int data){ 
//   SetMode(CS,Sequential);                         // set to send/receive multiple bytes of data
//   byte temp[2]; 				  // temporary array of bytes with 2 elements
//   temp[0]=(byte)(data>>8);                        // high byte of integer
//   temp[1]=(byte)(data);                           // low byte of integer
//   digitalWrite(CS, LOW);                          // start new command sequence
//   _spi.write(WRITE);                            // send WRITE command
//   _spi.write((byte)(address >> 16));            // send high byte of address
//   _spi.write((byte)(address >> 8));             // send middle byte of address
//   _spi.write((byte)address);                    // send low byte of address
//   _spi.write(temp, 2);                          // transfer an array of data => needs array name & size (2 elements)
//   digitalWrite(CS, HIGH);                         // set SPI slave select HIGH
// }

// unsigned int SRAMsimple::ReadUnsignedInt(uint32_t address){
//   SetMode(CS,Sequential);                         // set to send/receive multiple bytes of data
//   byte temp[2]; 				  // temporary array of bytes with 2 elements
//   unsigned int data=0;
//   digitalWrite(CS, LOW);                          // start new command sequence
//   _spi.write(READ);                             // send READ command
//   _spi.write((byte)(address >> 16));            // send high byte of address
//   _spi.write((byte)(address >> 8));             // send middle byte of address
//   _spi.write((byte)address);                    // send low byte of address
//   for(uint16_t i=0; i<2; i++){
//     temp[i] = _spi.write(0x00);                 // read the data byte
//   }
//   digitalWrite(CS, HIGH);                         // set SPI slave select HIGH
//   data=(temp[0]<<8)+temp[1];                      // data=high byte & low byte
//   return data;
// }

// /*********** Write and Read an Unsigned Integer Array ************************/
// void SRAMsimple::WriteUnsignedIntArray(uint32_t address, unsigned int *data, uint16_t big){
//   SetMode(CS,Sequential);                         // set to send/receive multiple bytes of data
//   byte temp[big*2]; 				  // temporary array of bytes with 2 elements
//   uint16_t j=0;                                   // byte counter
//   for(uint16_t i=0; i<big; i++){                   // take apart integers into high and low bytes
//     temp[j]=(byte)(data[i]>>8);                   // high byte of integer
//     temp[j+1]=(byte)(data[i]);                    // low byte of integer
//     j+=2;                                         // increment counter
//   }
//   digitalWrite(CS, LOW);                          // start new command sequence
//   _spi.write(WRITE);                            // send WRITE command
//   _spi.write((byte)(address >> 16));            // send high byte of address
//   _spi.write((byte)(address >> 8));             // send middle byte of address
//   _spi.write((byte)address);                    // send low byte of address
//   _spi.write(temp, big*2);                      // transfer an array of data => needs array name & size (2 elements)
//   digitalWrite(CS, HIGH);                         // set SPI slave select HIGH
// }

// void SRAMsimple::ReadUnsignedIntArray(uint32_t address, unsigned int *data, uint16_t big){
//   SetMode(CS,Sequential);                         // set to send/receive multiple bytes of data
//   byte temp[big*2]; 				  // temporary array of bytes with 2 elements
//   uint16_t j=0;                                   // byte counter
//   digitalWrite(CS, LOW);                          // start new command sequence
//   _spi.write(READ);                             // send READ command
//   _spi.write((byte)(address >> 16));            // send high byte of address
//   _spi.write((byte)(address >> 8));             // send middle byte of address
//   _spi.write((byte)address);                    // send low byte of address
//   for(uint16_t i=0; i<(big*2); i++){
//     temp[i] = _spi.write(0x00);                 // read the data byte
//   }
//   digitalWrite(CS, HIGH);                         // set SPI slave select HIGH
//   for(uint16_t i=0; i<big; i++){
//     data[i]=((int)temp[j]<<8)+temp[j+1];          // data=high byte & low byte
//     j+=2;                                         // increment counter
//   }
// }

// /*********** Write and Read a Long (4 bytes) ************************/
// void SRAMsimple::WriteLong(uint32_t address, long data){  
//   SetMode(CS,Sequential);                         // set to send/receive multiple bytes of data
//   byte temp[4]; 				  // temporary array of bytes with 4 elements
//   temp[0]=(byte)(data >> 24);                     // high byte of integer
//   temp[1]=(byte)(data >> 16);                     // low byte of integer
//   temp[2]=(byte)(data >> 8);                      // low byte of integer
//   temp[3]=(byte)(data);                           // low byte of integer
//   digitalWrite(CS, LOW);                          // start new command sequence
//   _spi.write(WRITE);                            // send WRITE command
//   _spi.write((byte)(address >> 16));            // send high byte of address
//   _spi.write((byte)(address >> 8));             // send middle byte of address
//   _spi.write((byte)address);                    // send low byte of address
//   _spi.write(temp, 4);                          // transfer an array of data => needs array name & size (4 elements)
//   digitalWrite(CS, HIGH);                         // set SPI slave select HIGH
// }

// long SRAMsimple::ReadLong(uint32_t address){
//   SetMode(CS,Sequential);                         // set to send/receive multiple bytes of data
//   byte temp[4]; 				  // temporary array of bytes with 4 elements
//   long data=0;
//   digitalWrite(CS, LOW);                          // start new command sequence
//   _spi.write(READ);                             // send READ command
//   _spi.write((byte)(address >> 16));            // send high byte of address
//   _spi.write((byte)(address >> 8));             // send middle byte of address
//   _spi.write((byte)address);                    // send low byte of address
//   for(uint16_t i=0; i<4; i++){
//     temp[i] = _spi.write(0x00);                 // read the data byte
//   }
//   digitalWrite(CS, HIGH);                         // set SPI slave select HIGH
//   data=((long)temp[0]<<24)+((long)temp[1]<<16)+((long)temp[2]<<8)+temp[3];   // reassemble bytes into long
//   return data;
// }

// /*********** Write and Read a Long Array ************************/
// void SRAMsimple::WriteLongArray(uint32_t address, long *data, uint16_t big){
//   SetMode(CS,Sequential);                         // set to send/receive multiple bytes of data
//   byte temp[big*4]; 				  // temporary array of bytes with 2 elements
//   uint16_t j=0;                                   // byte counter
//   for(uint16_t i=0; i<big; i++){                  // take apart integers into high and low bytes
//     temp[j]=(byte)(data[i] >> 24);                // high byte of integer
//     temp[j+1]=(byte)(data[i] >> 16);              // low byte of integer
//     temp[j+2]=(byte)(data[i] >> 8);               // low byte of integer
//     temp[j+3]=(byte)data[i];                      // low byte of integer
//     j+=4;                                         // increment counter
//   }
//   digitalWrite(CS, LOW);                          // start new command sequence
//   _spi.write(WRITE);                            // send WRITE command
//   _spi.write((byte)(address >> 16));            // send high byte of address
//   _spi.write((byte)(address >> 8));             // send middle byte of address
//   _spi.write((byte)address);                    // send low byte of address
//   _spi.write(temp, big*4);                      // transfer an array of data => needs array name & size (2 elements)
//   digitalWrite(CS, HIGH);                         // set SPI slave select HIGH
// }

// void SRAMsimple::ReadLongArray(uint32_t address, long *data, uint16_t big){
//   SetMode(CS,Sequential);                         // set to send/receive multiple bytes of data
//   byte temp[big*4]; 				  // temporary array of bytes with 2 elements
//   uint16_t j=0;                                   // byte counter
//   digitalWrite(CS, LOW);                          // start new command sequence
//   _spi.write(READ);                             // send READ command
//   _spi.write((byte)(address >> 16));            // send high byte of address
//   _spi.write((byte)(address >> 8));             // send middle byte of address
//   _spi.write((byte)address);                    // send low byte of address
//   for(uint16_t i=0; i<(big*4); i++){
//     temp[i] = _spi.write(0x00);                 // read the data byte
//   }
//   digitalWrite(CS, HIGH);                         // set SPI slave select HIGH
//   for(uint16_t i=0; i<big; i++){
//     data[i]=((long)temp[j]<<24)+((long)temp[j+1]<<16)+((long)temp[j+2]<<8)+temp[j+3];  // put together 4 bytes
//     j+=4;                                         // increment counter
//   }
// }

// /*********** Write and Read an Unsigned Long (4 bytes) ************************/
// void SRAMsimple::WriteUnsignedLong(uint32_t address, unsigned long data){
//   SetMode(CS,Sequential);                         // set to send/receive multiple bytes of data
//   byte temp[4]; 			 	  // temporary array of bytes with 4 elements
//   temp[0]=(byte)(data >> 24);                     // high byte of integer
//   temp[1]=(byte)(data >> 16);                     // low byte of integer
//   temp[2]=(byte)(data >> 8);                      // low byte of integer
//   temp[3]=(byte)(data);                           // low byte of integer
//   digitalWrite(CS, LOW);                          // start new command sequence
//   _spi.write(WRITE);                            // send WRITE command
//   _spi.write((byte)(address >> 16));            // send high byte of address
//   _spi.write((byte)(address >> 8));             // send middle byte of address
//   _spi.write((byte)address);                    // send low byte of address
//   _spi.write(temp, 4);                          // transfer an array of data => needs array name & size (4 elements)
//   digitalWrite(CS, HIGH);                         // set SPI slave select HIGH
// }

// unsigned long SRAMsimple::ReadUnsignedLong(uint32_t address){
//   SetMode(CS,Sequential);                         // set to send/receive multiple bytes of data
//   byte temp[4]; 				  // temporary array of bytes with 4 elements
//   unsigned long data=0;
//   digitalWrite(CS, LOW);                          // start new command sequence
//   _spi.write(READ);                             // send READ command
//   _spi.write((byte)(address >> 16));            // send high byte of address
//   _spi.write((byte)(address >> 8));             // send middle byte of address
//   _spi.write((byte)address);                    // send low byte of address
//   for(uint16_t i=0; i<4; i++){
//     temp[i] = _spi.write(0x00);                 // read the data byte
//   }
//   digitalWrite(CS, HIGH);                         // set SPI slave select HIGH
//   data=((long)temp[0]<<24)+((long)temp[1]<<16)+((long)temp[2]<<8)+temp[3];   // reassemble bytes into long
//   return data;
// }

// /*********** Write and Read a Long Array ************************/
// void SRAMsimple::WriteUnsignedLongArray(uint32_t address, unsigned long *data, uint16_t big){
//   SetMode(CS,Sequential);                         // set to send/receive multiple bytes of data
//   byte temp[big*4]; 				  // temporary array of bytes with 2 elements
//   uint16_t j=0;                                   // byte counter
//   for(uint16_t i=0; i<big; i++){                  // take apart integers into high and low bytes
//     temp[j]=(byte)(data[i] >> 24);                // high byte of integer
//     temp[j+1]=(byte)(data[i] >> 16);              // low byte of integer
//     temp[j+2]=(byte)(data[i] >> 8);               // low byte of integer
//     temp[j+3]=(byte)data[i];                      // low byte of integer
//     j+=4;                                         // increment counter
//   }
//   digitalWrite(CS, LOW);                          // start new command sequence
//   _spi.write(WRITE);                            // send WRITE command
//   _spi.write((byte)(address >> 16));            // send high byte of address
//   _spi.write((byte)(address >> 8));             // send middle byte of address
//   _spi.write((byte)address);                    // send low byte of address
//   _spi.write(temp, big*4);                      // transfer an array of data => needs array name & size (2 elements)
//   digitalWrite(CS, HIGH);                         // set SPI slave select HIGH
// }

// void SRAMsimple::ReadUnsignedLongArray(uint32_t address, unsigned long *data, uint16_t big){
//   SetMode(CS,Sequential);                         // set to send/receive multiple bytes of data
//   byte temp[big*4]; 				  // temporary array of bytes with 2 elements
//   uint16_t j=0;                                   // byte counter
//   digitalWrite(CS, LOW);                          // start new command sequence
//   _spi.write(READ);                             // send READ command
//   _spi.write((byte)(address >> 16));            // send high byte of address
//   _spi.write((byte)(address >> 8));             // send middle byte of address
//   _spi.write((byte)address);                    // send low byte of address
//   for(uint16_t i=0; i<(big*4); i++){
//     temp[i] = _spi.write(0x00);                 // read the data byte
//   }
//   digitalWrite(CS, HIGH);                         // set SPI slave select HIGH
//   for(uint16_t i=0; i<big; i++){
//     data[i]=((long)temp[j]<<24)+((long)temp[j+1]<<16)+((long)temp[j+2]<<8)+temp[j+3];  // put together 4 bytes
//     j+=4;                                         // increment counter
//   }
// }

// /*********** Write and Read a Float (4 bytes) ************************/
// void SRAMsimple::WriteFloat(uint32_t address, float data){
//   SetMode(CS,Sequential);                         // set to send/receive multiple bytes of data
//   byte *temp=(byte *)&data;                       // split float into 4 bytes
//   digitalWrite(CS, LOW);                          // start new command sequence
//   _spi.write(WRITE);                            // send WRITE command
//   _spi.write((byte)(address >> 16));            // send high byte of address
//   _spi.write((byte)(address >> 8));             // send middle byte of address
//   _spi.write((byte)address);                    // send low byte of address
//   _spi.write(temp, 4);                          // transfer an array of data => needs array name & size (4 elements)
//   digitalWrite(CS, HIGH);                         // set SPI slave select HIGH
// }

// float SRAMsimple::ReadFloat(uint32_t address){
//   SetMode(CS,Sequential);                         // set to send/receive multiple bytes of data
//   byte temp[4]; 				  // temporary array of bytes with 4 elements
//   float data=0;
//   digitalWrite(CS, LOW);                          // start new command sequence
//   _spi.write(READ);                             // send READ command
//   _spi.write((byte)(address >> 16));            // send high byte of address
//   _spi.write((byte)(address >> 8));             // send middle byte of address
//   _spi.write((byte)address);                    // send low byte of address
//   for(uint16_t i=0; i<4; i++){
//     temp[i] = _spi.write(0x00);                 // read the data byte
//   }
//   digitalWrite(CS, HIGH);                         // set SPI slave select HIGH
//   data = *(float *)&temp;                         // 4 bytes to a float.
//   return data;                                    // https://www.microchip.com/forums/m590535.aspx
// }

// /*********** Write and Read a Float Array ************************/
// void SRAMsimple::WriteFloatArray(uint32_t address, float *data, uint16_t big){
//   byte holder[big*4]; 				  // temporary array of bytes with 2 elements
//   uint16_t j=0;
//   for(int i=0;i<big;i++){
//     byte *temp=(byte *)&data[i];                  // split float into 4 bytes
//     holder[j]=temp[0];
//     holder[j+1]=temp[1];
//     holder[j+2]=temp[2];
//     holder[j+3]=temp[3];
//     j+=4;
//   }
//   SetMode(CS,Sequential);                         // set to send/receive multiple bytes of data
//   digitalWrite(CS, LOW);                          // start new command sequence
//   _spi.write(WRITE);                            // send WRITE command
//   _spi.write((byte)(address >> 16));            // send high byte of address
//   _spi.write((byte)(address >> 8));             // send middle byte of address
//   _spi.write((byte)address);                    // send low byte of address
//   _spi.write(holder, big*4);                    // transfer an array of data => needs array name & size (4 elements)
//   digitalWrite(CS, HIGH);                         // set SPI slave select HIGH
// }

// void SRAMsimple::ReadFloatArray(uint32_t address, float *data, uint16_t big){
//   SetMode(CS,Sequential);                         // set to send/receive multiple bytes of data
//   byte holder[big*4]; 				  // temporary array of bytes
//   byte t[4];                                      // to hold 4 bytes
//   digitalWrite(CS, LOW);                          // start new command sequence
//   _spi.write(READ);                             // send READ command
//   _spi.write((byte)(address >> 16));            // send high byte of address
//   _spi.write((byte)(address >> 8));             // send middle byte of address
//   _spi.write((byte)address);                    // send low byte of address
//   for(uint16_t i=0; i<(big*4); i++){
//     holder[i] = _spi.write(0x00);               // read the data byte
//   }
//   digitalWrite(CS, HIGH);                         // set SPI slave select HIGH
//   uint16_t j=0;
//   for(int i=0;i<big;i++){ 
//     t[0]=holder[j];
//     t[1]=holder[j+1];
//     t[2]=holder[j+2];
//     t[3]=holder[j+3];
//     data[i] = *(float *)&t;                        // 4 bytes to a float.
//     j+=4;
//   }
// }
