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
#define H_COLS 10

#define CRC_KEY 0b10110111001

#define CRC_BLOCK_OFFSET_A 0b0011111100
#define CRC_BLOCK_OFFSET_B 0b0110011000
#define CRC_BLOCK_OFFSET_C 0b0101101000
#define CRC_BLOCK_OFFSET_D 0b0110110100

typedef struct{
  uint16_t message;  
  uint16_t crc;  // CRC will be only on lower 10 bits 
} InputMessage;

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
