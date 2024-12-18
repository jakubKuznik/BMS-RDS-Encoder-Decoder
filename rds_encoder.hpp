// Faculty: BUT FIT 
// Course: BMS 
// Project Name: RDS Encoder and Decoder Project Overview
// Name: Jakub Kuznik
// Login: xkuzni04
// Year: 2024

#include <iostream>
#include <algorithm>
#include <regex>
#include <cmath>
#include <iomanip>  // For std::fixed and std::setprecision

#define AF_SIZE 2
#define PS_SIZE 8 
#define RT_SIZE_PLUS_TERMINATOR 65

#define CRC_KEY 0b10110111001

#define CRC_BLOCK_OFFSET_A 0b0011111100
#define CRC_BLOCK_OFFSET_B 0b0110011000
#define CRC_BLOCK_OFFSET_C 0b0101101000
#define CRC_BLOCK_OFFSET_D 0b0110110100

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
 * Holds whole program configuration with specific flags. 
 */
typedef struct {
  bool is0A;       
  bool is2A;       
  FlagsCommon flagsCommon;
  Flags0A flags0A;  
  Flags2A flags2A;  
} ProgramConfig;
