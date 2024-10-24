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

#define DATA_BITS 16
#define CRC_BITS 10
#define BLOCK_BITS 26 
#define H_ROWS 26

#define CRC_KEY 0b10110111001

#define CRC_BLOCK_OFFSET_A 0b0011111100
#define CRC_BLOCK_OFFSET_B 0b0110011000
#define CRC_BLOCK_OFFSET_C 0b0101101000
#define CRC_BLOCK_OFFSET_D 0b0110110100

typedef struct{
  uint16_t message;  
  uint16_t crc;  // CRC will be only on lower 10 bits 
} InputMessage;

const std::bitset<H_ROWS> H[H_ROWS] = {
  0b1000000000,
  0b0100000000,
  0b0010000000,
  0b0001000000,
  0b0000100000,
  0b0000010000,
  0b0000001000,
  0b0000000100,
  0b0000000010,
  0b0000000001,
  0b1011011100,
  0b0101101110,
  0b0010110111,
  0b1010000111,
  0b1110011111,
  0b1100010011,
  0b1101010101,
  0b1101110110,
  0b0110111011,
  0b1000000001,
  0b1111011100,
  0b0111101110,
  0b0011110111,
  0b1010100111,
  0b1110001111,
  0b1100011011,
};
