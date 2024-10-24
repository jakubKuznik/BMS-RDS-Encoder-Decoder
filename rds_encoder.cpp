// Faculty: BUT FIT 
// Course: BMS 
// Project Name: RDS Encoder and Decoder Project Overview
// Name: Jakub Kuznik
// Login: xkuzni04
// Year: 2024

#include "rds_encoder.hpp"

using namespace std;

// TODO delete
void printProgramConfig(const ProgramConfig &config) {
    cerr << "ProgramConfig: " << endl;
    
    // Print general flags
    cerr << "  is0A: " << boolalpha << config.is0A << endl;
    cerr << "  is2A: " << boolalpha << config.is2A << endl;

    // Print FlagsCommon
    cerr << "  FlagsCommon: " << endl;
    cerr << "    pi: " << config.flagsCommon.pi << endl;
    cerr << "    pty: " << config.flagsCommon.pty << endl;
    cerr << "    tp: " << boolalpha << config.flagsCommon.tp << endl;

    // Print Flags0A
    cerr << "  Flags0A: " << endl;
    cerr << "    ms: " << boolalpha << config.flags0A.ms << endl;
    cerr << "    ta: " << boolalpha << config.flags0A.ta << endl;
    cerr << "    af: [";
    for (int i = 0; i < AF_SIZE; ++i) {
        cerr << config.flags0A.af[i];
        if (i < AF_SIZE - 1) cerr << ", ";
    }
    cerr << "]" << endl;
    cerr << "    ps: " << config.flags0A.ps << endl;

    // Print Flags2A
    cerr << "  Flags2A: " << endl;
    cerr << "    rt: " << config.flags2A.rt << endl;
    cerr << "    ab: " << boolalpha << config.flags2A.ab << endl;
}


/**
 * Prints help and exit program with return code 1
*/
void printHelp(){
	cerr << "Execution:" << endl;
	cerr << "  ./program_encoder {-g {0A,2A}} [FLAGS]" << endl; 
	cerr << "       Program reads from STDIN and print output to STDOUT.";
	cerr << "  Common FLAGS" << endl;
	cerr << "    -pi" << endl;
	cerr << "        Program identification (16-bit unsigned integer.)" << endl;
	cerr << "    -pty" << endl;
	cerr << "        Program Type. (5-bit unsigned integer, values 0-31)" << endl;
	cerr << "    -tp" << endl;
	cerr << "        Traffic Program (boolean, 0 or 1)" << endl;
	cerr << endl;
	cerr << "  0A-Specific FLAGS" << endl;
	cerr << "    -ms" << endl;
	cerr << "        Music/Speech (boolean, 0 for Sepech, 1 for Music)" << endl;
	cerr << "    -ta" << endl;
	cerr << "        Traffic Annoucement (boolean, 0 or 1)" << endl;
	cerr << "    -af" << endl;
	cerr << "        Alterantive Frequencies (two comma-separated frequency values, e.g., -af 104.5,98.0). \
							Aerrys float numers with precision to 0.1" << endl;
	cerr << "    -ps" << endl;
	cerr << "        Program Service (8-character string). If shorter, the encoder will add padding with spaces." << endl;
	cerr << endl;
	cerr << "  2A-Specific FLAGS" << endl;
	cerr << "    -rt" << endl;
	cerr << "        Radio Text (string up to 64 characters)" << endl;
	cerr << "    -ab" << endl;
	cerr << "        Radio text A/B flag (boolean, 0 or 1)." << endl;
	cerr << endl;
	cerr << "Example: " << endl;
    cerr << "  ./rds_encoder -g 0A -pi 4660 -pty 5 -tp 1 -ms 0 -ta 1 -af 104.5,98.0 -ps \"RadioXYZ\"" << endl;
	exit(1);
}

bool parseBoolArg(const char* arg) {
    try {
        int myInt = std::stoi(arg);
        if (myInt == 0) {
            return false;
        } else if (myInt == 1) {
            return true;
        } else {
            throw std::out_of_range("Invalid boolean value");
        }
    } catch (...) {
        throw std::runtime_error("Invalid argument for boolean flag");
    }
}

template <typename T>
T parseUintArg(const char* arg, T min, T max) {
    try {
        int myInt = std::stoi(arg);
        if (myInt >= min && myInt <= max) {
            return static_cast<T>(myInt);
        } else {
            throw std::out_of_range("Value out of range");
        }
    } catch (...) {
        throw std::runtime_error("Invalid argument for unsigned int");
    }
}

void parseAfArg(const char* arg, float (&af)[AF_SIZE]) {
    std::string input(arg);
    std::replace(input.begin(), input.end(), ',', ' ');  
    std::istringstream iss(input);
    float freq1, freq2;
    
    if (!(iss >> freq1) || !(iss >> freq2) || !iss.eof()) {
        throw std::invalid_argument("Invalid alternative frequencies");
    }

    // Set the frequencies to the af array
    af[0] = freq1;
    af[1] = freq2;

    // Output the frequencies with one decimal precision, ensuring `.0` is printed
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "f1 " << af[0] << " f2 " << af[1] << std::endl;
}


void parseStringArg(const char* arg, char* dest, size_t maxLength, bool padWithSpaces = false) {
    size_t len = std::strlen(arg);
    
    if (len > maxLength) {
        throw std::invalid_argument("Input string is too long (must be " + std::to_string(maxLength) + " characters or fewer)");
    }

    std::strncpy(dest, arg, maxLength);

    if (padWithSpaces && len < maxLength) {
        // Pad with spaces if the string is shorter and padding is required
        for (size_t i = len; i < maxLength; ++i) {
            dest[i] = ' ';
        }
    }

    // Null-terminate the string if padding is not required (when input is shorter than maxLength)
    if (!padWithSpaces) {
        dest[len] = '\0';
    }
}

/**
 * Parse input args
 */
void argParse(int argc, char **argv, ProgramConfig *config) {
    config->is0A = false;
    config->is2A = false;

    bool commonPi = false, commonPty = false, commonTp = false;
    bool flags0Ams = false, flags0Ata = false, flags0Aaf = false, flags0Aps = false;
    bool flags2Art = false, flags2Aab = false;

    for (int i = 1; i < argc; i++) {

        if (strcmp(argv[i],"--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printHelp();
        } else if (strcmp(argv[i], "-g") == 0) {
            i++;
            if (strcmp(argv[i], "0A") == 0) {
                config->is0A = true;
            } else if (strcmp(argv[i], "2A") == 0) {
                config->is2A = true;
            } else {
                goto errorArgs;
            }
        } else if (strcmp(argv[i], "-pi") == 0) {
            i++; commonPi = true;
            try {
                config->flagsCommon.pi = parseUintArg<uint16_t>(argv[i], 0, UINT16_MAX); 
            } catch (...) {
                goto errorArgs;
            }
        } else if (strcmp(argv[i], "-pty") == 0) {
            i++; commonPty = true;
            try {
                config->flagsCommon.pty = parseUintArg<uint8_t>(argv[i], 0, 31); 
            } catch (...) {
                goto errorArgs;
            }
        } else if (strcmp(argv[i], "-tp") == 0) {
            i++; commonTp = true;
            try {
                config->flagsCommon.tp = parseBoolArg(argv[i]); 
            } catch (...) {
                goto errorArgs;
            }
        // SPECIFIC 0A
        } else if (strcmp(argv[i], "-ms") == 0) {
            i++; flags0Ams = true;
            try {
                config->flags0A.ms = parseBoolArg(argv[i]); 
            } catch (...) {
                goto errorArgs;
            }
        } else if (strcmp(argv[i], "-ta") == 0) {
            i++; flags0Ata = true;
            try {
                config->flags0A.ta = parseBoolArg(argv[i]); 
            } catch (...) {
                goto errorArgs;
            }
        } else if (strcmp(argv[i], "-af") == 0) {
            i++; flags0Aaf = true;
            try {
                parseAfArg(argv[i], config->flags0A.af); 
            } catch (...) {
                goto errorArgs;
            }
        } else if (strcmp(argv[i], "-ps") == 0) {
            i++; flags0Aps = true;
            try {
                parseStringArg(argv[i], config->flags0A.ps, PS_SIZE, true); 
            } catch (...) {
                goto errorArgs;
            }
        // SPECIFIC 2A
        } else if (strcmp(argv[i], "-rt") == 0) {
            i++; flags2Art = true;
            try {
                parseStringArg(argv[i], config->flags2A.rt, RT_SIZE_PLUS_TERMINATOR, true);  
            } catch (...) {
                goto errorArgs;
            }
        } else if (strcmp(argv[i], "-ab") == 0) {
            i++; flags2Aab = true;
            try {
                config->flags2A.ab = parseBoolArg(argv[i]); 
            } catch (...) {
                goto errorArgs;
            }
        } else {
            goto errorArgs;
        } 

    }

    // TODO delete 
    printProgramConfig(*config);

    // error check
    if (config->is0A && config->is2A) {
        cerr << "Cannot choose both -g 0A and 2A" << endl;
        goto errorArgs;
    }
    if (!config->is0A && !config->is2A) {
        cerr << "Must choose either -g 0A or -g 2A" << endl;
        goto errorArgs;
    }
    if (!commonPi && !commonPty && !commonTp) {
        cerr << "Common parameter missing {-pi, -pty, -tp}" << endl;
        goto errorArgs;
    }
    if (config->is0A && (flags2Aab || flags2Art)) {
        cerr << "0A parameter missing {-ab, -rt}" << endl;
        goto errorArgs;
    }
    if (config->is2A && (flags0Ams || flags0Ata || flags0Aaf || flags0Aps)) {
        cerr << "Cannot specify 0A flags with -g 2A" << endl;
        goto errorArgs;
    }
    if (config->is0A && (!flags0Ams || !flags0Ata || !flags0Aaf || !flags0Aps)) {
        cerr << "Missing required 0A flags {-ms, -ta, -af, -ps}" << endl;
        goto errorArgs;
    }
    if (config->is2A && (!flags2Aab || !flags2Art)) {
        cerr << "Missing required 2A flags {-ab, -rt}" << endl;
        goto errorArgs;
    }

    return;
errorArgs:
    cerr << "Error: wrong arguments. Try using {-h|--help}" << endl;
    exit(1);
}

/**
 * Count CRC of data
 * @param data: The input data for CRC calculation
 * @return: The computed CRC value (last 10 bits as uint16_t)
 */
uint16_t countCRC(uint16_t data, uint16_t magicConst) {
    
    // Step 1: Append 10 zeros to the input data (shift left by 10 bits)
    uint32_t extendedData = (static_cast<uint32_t>(data) << 10);

    int divisorShift = 15;     
    uint16_t currentBit = 25;

    // Loop through the bits of extendedData
    for (int i = 0; i < 16; i++) {
        uint32_t shiftedDivisor = CRC_KEY << divisorShift; // Shift the divisor
        
        // Check if the n-th bit is set to 1
        if ((extendedData & (1 << currentBit--)) == 0) {
            //std::cerr << "SKIP" << std::endl;
            divisorShift--;
            continue;
        }

        extendedData = extendedData ^ shiftedDivisor;
        divisorShift--;
    }

    extendedData = extendedData ^ magicConst;

    // Return the computed CRC value (last 10 bits as uint16_t)
    return static_cast<uint16_t>(extendedData & 0x3FF); 
}

/**
 *  0000 0001 is 87.6
 *  0000 0010 is 87.7
 *  0000 0011 is 87.8
 *  0000 0100 is 87.9
 */
uint8_t parseFrequencyToBinary(float frequency) {
    
    // Normalize the frequency: Subtract 87.5 MHz and multiply by 10
    float step_float = (frequency - 87.5) * 10;
    int step = static_cast<int>(round(step_float));
    
    // Check if the step is within the valid range (0-255 for 8 bits)
    if (step < 0 || step > 255) {
        throw std::invalid_argument("Frequency out of range for binary representation.");
    }
    cout << "freq: " << frequency << " " << bitset<8>(step) << endl;

    // Return the step as an 8-bit unsigned integer
    return static_cast<uint8_t>(step);
}

/** 
 * Generate 0A flags (part of the mesage)
 * 
 * frame_number - 00 -> 01 -> 10 -> 11
 * 
 */
void generateOutput0a(ProgramConfig *config){
    
    uint16_t row2 = 0;  
    uint16_t row3 = 0;
    uint16_t row4 = 0;
    
    // SECOND ROW - FLAGS
    uint8_t frame_num = 0; // frame num we set in loop
    row2 |= (0 << 12);  // First 4 bits (0000 represents 0A)
    row2 |= (0 << 11);  // A/B (next bit, assumed as 0 here)
    row2 |= (config->flagsCommon.tp << 10);  // TP (1 bit)
    row2 |= (config->flagsCommon.pty << 5);  // PTY (5 bits)
    row2 |= (config->flags0A.ta << 4);       // TA (1 bit)
    row2 |= (config->flags0A.ms << 3);       // M/S (1 bit)
    row2 |= (0 << 2);                        // DI (Always 0, 1 bit)
        
    // THIRD ROW - Frequencies  
    row3 |= ((parseFrequencyToBinary(config->flags0A.af[0])) << 8);
    row3 |= (parseFrequencyToBinary(config->flags0A.af[1]));
    
    for (int i = 0; i < 4; i++){
        // FIRST ROW - FLAGS
        cout << "" << bitset<16>(config->flagsCommon.pi);
        cout << " " << bitset<10>(countCRC(config->flagsCommon.pi, CRC_BLOCK_OFFSET_A)) << endl;
        
        // SECOND ROW - FLAGS
        // clear last 2 bits 
        row2 &= 0b1111111111111100; // Clear last two bits (set them to 0)
        row2 |= (frame_num & 0b11);
        frame_num++;
        cout << bitset<16>(row2);
        cout << " " << bitset<10>(countCRC(row2, CRC_BLOCK_OFFSET_B)) << endl;
    
        // THIRD ROW - Frequencies  
        cout << bitset<16>(row3);
        cout << " " << bitset<10>(countCRC(row3, CRC_BLOCK_OFFSET_C)) << endl;
        row3 = 0;

        // FOURTH ROW - MESSAGE 
        row4 = 0;
        row4 |= (static_cast<uint8_t>(config->flags0A.ps[(i*2)]) << 8);
        row4 |= (static_cast<uint8_t>(config->flags0A.ps[(i*2)+1]));
    
        cout << bitset<16>(row4);
        cout << " " << bitset<10>(countCRC(row4, CRC_BLOCK_OFFSET_D)) << endl << endl;
    }
}

/** 
 * Generate 2A flags (part of the mesage)
 */
void generateOutput2a(ProgramConfig *config){
    
    // todo check it should be maximum 16*4char  
    // todo maybe hardcode to RT_SIZE_PLUS_TERMINATOR
    uint16_t blocks = (strlen(config->flags2A.rt) + 4 / 2) / 4;

    uint16_t row2 = 0;  
    uint16_t row3 = 0;
    uint16_t row4 = 0;
    
    uint8_t text_segment = 0; // frame num we set in loop

    // SECOND ROW - FLAGS
    row2 |= (2 << 12);  // First 4 bits (0010 represents 2A)
    row2 |= (0 << 11);  // A/B (next bit, assumed as 0 here)
    row2 |= (config->flagsCommon.tp << 10);  // TP (1 bit)
    row2 |= (config->flagsCommon.pty << 5);  // PTY (5 bits)
    row2 |= (config->flags2A.ab << 4);       // AB (1 bit)

    for (int i = 0; i < blocks; i++){

        // ROW 1        
        cout << "" << bitset<16>(config->flagsCommon.pi);
        cout << " " << bitset<10>(countCRC(config->flagsCommon.pi, CRC_BLOCK_OFFSET_A)) << endl;
        
        // ROW 2       
        // text sexment 
        // 0000 -> 0001 -> 0010
        row2 &= ~0xF; // Clear the lower 4 bits of row2 (0xF is 00001111 in binary)
        row2 |= (text_segment & 0xF);  // Set the lower 4 bits of row2 to the lower 4 bits of text_segment
        text_segment++;
    
        cout << bitset<16>(row2);
        cout << " " << bitset<10>(countCRC(row2, CRC_BLOCK_OFFSET_B)) << endl;
        
        // ROW 3       
        row3 = 0;
        row3 |= (static_cast<uint8_t>(config->flags2A.rt[(i*4)]) << 8);
        row3 |= (static_cast<uint8_t>(config->flags2A.rt[(i*4)+1]));
        
        cout << bitset<16>(row3);
        cout << " " << bitset<10>(countCRC(row3, CRC_BLOCK_OFFSET_C)) << endl;
    
        // ROW 4       
        row4 = 0;
        row4 |= (static_cast<uint8_t>(config->flags2A.rt[(i*4)+2]) << 8);
        row4 |= (static_cast<uint8_t>(config->flags2A.rt[(i*4)+3]));

        cout << bitset<16>(row4);
        cout << " " << bitset<10>(countCRC(row4, CRC_BLOCK_OFFSET_D)) << endl << endl;
    }
}

/** 
 * Whole message with CRC
 */
void generateOutput(ProgramConfig *config){

    if (config->is0A){
        // 0 means first frame 
        generateOutput0a(config);
    } else if (config->is2A){
        generateOutput2a(config);
    }
    cout << endl;
} 

int main(int argc, char **argv){

    ProgramConfig config;

    argParse(argc, argv, &config);

    generateOutput(&config);

}