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

typedef struct{
  uint16_t message;  
  uint16_t crc;  // CRC will be only on lower 10 bits 
} InputMessage;
