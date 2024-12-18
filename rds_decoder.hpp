// Faculty: BUT FIT 
// Course: BMS 
// Project Name: RDS Encoder and Decoder Project Overview
// Name: Jakub Kuznik
// Login: xkuzni04
// Year: 2024

#include <iostream>
#include <stdexcept>
#include <vector>
#include <bitset>
#include <string>
#include <cstring>
#include <iomanip> 

#define DATA_BITS 16
#define CRC_BITS 10
#define BLOCK_BITS 26 
#define H_ROWS 26
#define H_COLS 10

#define CRC_KEY 0b10110111001

#define CRC_BLOCK_OFFSET_A 0b0011111100
#define CRC_BLOCK_OFFSET_B 0b0110011000
#define CRC_BLOCK_OFFSET_C 0b0101101000
#define CRC_BLOCK_OFFSET_D 0b0110110100

#define AF_SIZE 2
#define PS_SIZE 8 
#define RT_SIZE_PLUS_TERMINATOR 65

// Matrix for CRC evaluation
const std::bitset<H_ROWS> H_TRANSPOSED[H_COLS] = {
  0b10000000001001111101100111,
  0b01000000000100111110110011,
  0b00100000001011100010111110,
  0b00010000001100001100111000,
  0b00001000000110000110011100,
  0b00000100001010111110101001,
  0b00000010001100100010110011,
  0b00000001001111101100111110,
  0b00000000100111110110011111,
  0b00000000010011111011001111
};

/**
 * Struct holds raw input data and divide it into chunks where 
 * 16 bit is a message 
 * 10 bit is a CRC code
 */
typedef struct{
  uint16_t message;  
  uint16_t crc;  // CRC will be only on lower 10 bits 
} InputMessage;

/**
 * Holds flags that are the same for 0A and 2A 
 */
typedef struct {
  unsigned int pi;  
  unsigned int pty; 
  bool tp;          
} FlagsCommon;

/**
 * Holds only 0A flags. 
 */
typedef struct {
  bool ms;                    
  bool ta;                     
  float af[AF_SIZE];       
  char ps[PS_SIZE]; 
} Flags0A;

/**
 * Holds only 2A flags. 
 */
typedef struct {
  char rt[RT_SIZE_PLUS_TERMINATOR];   
  bool ab;          
} Flags2A;

/**
 * Holds whole message with its specific flags. 
 */
typedef struct {
  bool is0A;       
  bool is2A;       
  FlagsCommon flagsCommon;
  Flags0A flags0A;  
  Flags2A flags2A;  
} MessageProperties;


