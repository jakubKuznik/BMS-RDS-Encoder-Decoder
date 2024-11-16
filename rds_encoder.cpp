// Faculty: BUT FIT 
// Course: BMS 
// Project Name: RDS Encoder and Decoder Project Overview
// Name: Jakub Kuznik
// Login: xkuzni04
// Year: 2024

#include "rds_encoder.hpp"

using namespace std;

/**
 * Prints help and exit program with return code 0 
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
  exit(0);
}

/**
 * Parse bool arg 
 */
bool parseBoolArg(const char* arg) {
  try {
    int myInt = stoi(arg);
    if (myInt == 0) {
      return false;
    } 
    else if (myInt == 1) {
      return true;
    } 
    else {
      throw out_of_range("Invalid boolean value");
    }
  } 
  catch (...) {
    throw runtime_error("Invalid argument for boolean flag");
  }
}

/**
 * Parse int of whatever range
 */
template <typename T>
T parseUintArg(const char* arg, T min, T max) {
  try {
    int myInt = stoi(arg);
    if (myInt >= min && myInt <= max) {
      return static_cast<T>(myInt);
    } 
    else {
      throw out_of_range("Value out of range");
    }
  } 
  catch (...) {
    throw runtime_error("Invalid argument for unsigned int");
  }
}

/**
 * Check whenever float num (probably gained from -af) has exactly one decimal point 
 */
bool hasExactlyOneDecimal(const std::string& str) {
    // Split the string at the decimal point
    size_t decimal_pos = str.find('.');

    // Ensure there is exactly one decimal point
    if (decimal_pos == std::string::npos || str.find('.', decimal_pos + 1) != std::string::npos) {
        return false;
    }

    // Split into two parts: before and after the decimal point
    std::string before_decimal = str.substr(0, decimal_pos);
    std::string after_decimal = str.substr(decimal_pos + 1);

    // Check if there is exactly one digit after the decimal point
    if (after_decimal.length() != 1) {
        return false;
    }

    // Check if there are digits before the decimal point (non-empty)
    if (before_decimal.empty()) {
        return false;
    }

    return true;
}

/**
 * Function parses -af args (frequencies)
 */
void parseAfArg(const char* arg, float (&af)[AF_SIZE]) {
    std::string input(arg);

    // Find the comma and check if there's exactly one
    size_t comma_pos = input.find(',');
    if (comma_pos == std::string::npos) {
        throw std::invalid_argument("Input string must contain a comma separating two values");
    }

    size_t second_comma_pos = input.find(',', comma_pos + 1);
    if (second_comma_pos != std::string::npos) {
        throw std::invalid_argument("Input string must contain exactly one comma separating two values");
    }


    // Extract the parts before and after the comma
    std::string part1 = input.substr(0, comma_pos);
    std::string part2 = input.substr(comma_pos + 1);

    // Check if both parts contain exactly one decimal point and digits before and after it
    if (!hasExactlyOneDecimal(part1) || !hasExactlyOneDecimal(part2)) {
        throw std::invalid_argument("Both parts must contain exactly one decimal point and digits before and after it");
    }

    // Attempt to parse the two parts as floats
    std::istringstream iss1(part1), iss2(part2);
    float freq1, freq2;
    if (!(iss1 >> freq1) || !(iss2 >> freq2)) {
        throw std::invalid_argument("Both parts must be valid numbers");
    }

    // Set the frequencies to the af array
    af[0] = freq1;
    af[1] = freq2;
}

/**
 * Functon parses string args such as -ps -rt 
 */
void parseStringArg(const char* arg, char* dest, size_t maxLength, bool padWithSpaces = false) {
    // Regular expression to validate the input string
    std::regex valid_regex("^[a-zA-Z0-9 ]*$");

    // Check if the argument matches the regex pattern
    if (!std::regex_match(arg, valid_regex)) {
        throw std::invalid_argument("Input string contains invalid characters. Only alphanumeric and spaces are allowed.");
    }

    size_t len = strlen(arg);
  
    if (len > maxLength) {
        throw std::invalid_argument("Input string is too long (must be " + std::to_string(maxLength) + " characters or fewer)");
    }

    strncpy(dest, arg, maxLength);

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
    } 
    else if (strcmp(argv[i], "-g") == 0) {
      i++;
      if (strcmp(argv[i], "0A") == 0) {
        config->is0A = true;
      } 
      else if (strcmp(argv[i], "2A") == 0) {
        config->is2A = true;
      } 
      else {
        goto errorArgs;
      }
    } 
    else if (strcmp(argv[i], "-pi") == 0) {
      i++; commonPi = true;
      try {
        config->flagsCommon.pi = parseUintArg<uint16_t>(argv[i], 0, UINT16_MAX); 
      } catch (...) {
        goto errorArgs;
      }
    } 
    else if (strcmp(argv[i], "-pty") == 0) {
      i++; commonPty = true;
      try {
        config->flagsCommon.pty = parseUintArg<uint8_t>(argv[i], 0, 31); 
      } 
      catch (...) {
        goto errorArgs;
      }
    } 
    else if (strcmp(argv[i], "-tp") == 0) {
      i++; commonTp = true;
      try {
        config->flagsCommon.tp = parseBoolArg(argv[i]); 
      } 
      catch (...) {
        goto errorArgs;
      }
      // SPECIFIC 0A
    } 
    else if (strcmp(argv[i], "-ms") == 0) {
      i++; flags0Ams = true;
      try {
        config->flags0A.ms = parseBoolArg(argv[i]); 
      } 
      catch (...) {
        goto errorArgs;
      }
    } 
    else if (strcmp(argv[i], "-ta") == 0) {
      i++; flags0Ata = true;
      try {
        config->flags0A.ta = parseBoolArg(argv[i]); 
      } 
      catch (...) {
        goto errorArgs;
      }
    } 
    else if (strcmp(argv[i], "-af") == 0) {
      i++; flags0Aaf = true;
      try {
        parseAfArg(argv[i], config->flags0A.af); 
      } 
      catch (...) {
        goto errorArgs;
      }
    } 
    else if (strcmp(argv[i], "-ps") == 0) {
      i++; flags0Aps = true;
      try {
        parseStringArg(argv[i], config->flags0A.ps, PS_SIZE, true); 
      } 
      catch (...) {
        goto errorArgs;
      }
    // SPECIFIC 2A
    } 
    else if (strcmp(argv[i], "-rt") == 0) {
      i++; flags2Art = true;
      try {
        parseStringArg(argv[i], config->flags2A.rt, RT_SIZE_PLUS_TERMINATOR, true);  
      } 
      catch (...) {
        goto errorArgs;
      }
    } 
    else if (strcmp(argv[i], "-ab") == 0) {
      i++; flags2Aab = true;
      try {
        config->flags2A.ab = parseBoolArg(argv[i]); 
      } 
      catch (...) {
        goto errorArgs;
      }
    } 
    else {
        goto errorArgs;
    } 
  }

  // error check
  if (config->is0A && config->is2A) {
    cerr << "Cannot choose both -g 0A and 2A" << endl;
    goto errorArgs;
  }
  if (!config->is0A && !config->is2A) {
    cerr << "Must choose either -g 0A or -g 2A" << endl;
    goto errorArgs;
  }
  if (!commonPi || !commonPty || !commonTp) {
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
    cerr << "Frequency out of range for binary representation." << endl;
    exit(1);
  }

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
    cout <<  bitset<16>(config->flagsCommon.pi) << bitset<10>(countCRC(config->flagsCommon.pi, CRC_BLOCK_OFFSET_A));
        
    // SECOND ROW - FLAGS
    // clear last 2 bits 
    row2 &= 0b1111111111111100; // Clear last two bits (set them to 0)
    row2 |= (frame_num & 0b11);
    frame_num++;
    cout << bitset<16>(row2) << bitset<10>(countCRC(row2, CRC_BLOCK_OFFSET_B));
    
    // THIRD ROW - Frequencies  
    cout << bitset<16>(row3) << bitset<10>(countCRC(row3, CRC_BLOCK_OFFSET_C));
    row3 = 0;

    // FOURTH ROW - MESSAGE 
    row4 = 0;
    row4 |= (static_cast<uint8_t>(config->flags0A.ps[(i*2)]) << 8);
    row4 |= (static_cast<uint8_t>(config->flags0A.ps[(i*2)+1]));
    
    cout << bitset<16>(row4) << bitset<10>(countCRC(row4, CRC_BLOCK_OFFSET_D));
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
    cout << bitset<16>(config->flagsCommon.pi) << bitset<10>(countCRC(config->flagsCommon.pi, CRC_BLOCK_OFFSET_A));
        
    // ROW 2       
    // text sexment 
    // 0000 -> 0001 -> 0010
    row2 &= ~0xF; // Clear the lower 4 bits of row2 (0xF is 00001111 in binary)
    row2 |= (text_segment & 0xF);  // Set the lower 4 bits of row2 to the lower 4 bits of text_segment
    text_segment++;
    
    cout << bitset<16>(row2) << bitset<10>(countCRC(row2, CRC_BLOCK_OFFSET_B));
        
    // ROW 3       
    row3 = 0;
    row3 |= (static_cast<uint8_t>(config->flags2A.rt[(i*4)]) << 8);
    row3 |= (static_cast<uint8_t>(config->flags2A.rt[(i*4)+1]));
        
    cout << bitset<16>(row3) << bitset<10>(countCRC(row3, CRC_BLOCK_OFFSET_C));
    
    // ROW 4       
    row4 = 0;
    row4 |= (static_cast<uint8_t>(config->flags2A.rt[(i*4)+2]) << 8);
    row4 |= (static_cast<uint8_t>(config->flags2A.rt[(i*4)+3]));

    cout << bitset<16>(row4) << bitset<10>(countCRC(row4, CRC_BLOCK_OFFSET_D));
  }
}

/** 
 * It generates whole message with CRC on stdout
 */
void generateOutput(ProgramConfig *config){

  if (config->is0A){
    // 0 means first frame 
    generateOutput0a(config);
  }
  else if (config->is2A){
    generateOutput2a(config);
  }
} 

int main(int argc, char **argv){

  ProgramConfig config;

  argParse(argc, argv, &config);

  generateOutput(&config);
}