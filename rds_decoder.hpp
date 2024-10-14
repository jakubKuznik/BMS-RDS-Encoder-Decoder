// Faculty: BUT FIT 
// Course: BMS 
// Project Name: RDS Encoder and Decoder Project Overview
// Name: Jakub Kuznik
// Login: xkuzni04
// Year: 2024


#include <iostream>
#include <list>
#include <string.h>
#include <algorithm>
#include <regex>
#include <iomanip> 

typedef struct {
    unsigned int pi;  
    unsigned int pty; 
    bool tp;          
} FlagsCommon;

typedef struct {
    bool ms;                    
    bool ta;                     
    std::vector<float> af;       
    std::string ps;              
} Flags0A;

typedef struct {
    std::string rt;   
    bool ab;          
} Flags2A;

typedef struct {
    bool is0A;       
    bool is2A;       
    FlagsCommon flagsCommon;
    Flags0A flags0A;  
    Flags2A flags2A;  
} ProgramConfig;

