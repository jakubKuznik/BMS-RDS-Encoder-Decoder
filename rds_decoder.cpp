// Faculty: BUT FIT 
// Course: BMS 
// Project Name: RDS Encoder and Decoder Project Overview
// Name: Jakub Kuznik
// Login: xkuzni04
// Year: 2024

#include "rds_decoder.hpp"

using namespace std;

/**
 * Prints help and exit program with return code 1
 */
void printHelp(){
  std::cerr << "Execution:" << std::endl;
  std::cerr << "  ./program_encoder {-b MESSAGE}" << std::endl; 
  std::cerr << "       Program reads from STDIN and prints output to STDOUT." << std::endl;
  std::cerr << "" << std::endl;
  std::cerr << "           MESSAGE" << std::endl;
  std::cerr << "             - Sequence of 0 and 1 that has to be a multiple of 26" << std::endl;
  exit(1);
}

/**
 * Parse binary string and store in a vector of InputMessage
 */
void parseBinaryString(const std::string& binaryStr, std::vector<InputMessage>& dataChunks) {
  size_t length = binaryStr.length();

  // Ensure the input length is a multiple of 26 bits (16-bit message + 10-bit CRC)
  if (length % 26 != 0) {
    std::cerr << "Error: Binary string length must be a multiple of 26 bits." << std::endl;
    goto errorArgs;
  }

  // Parse 26-bit chunks (16-bit message + 10-bit CRC)
  for (size_t i = 0; i < length; i += 26) {
    // Extract 16-bit message
    std::bitset<16> messageBits(binaryStr.substr(i, 16));
    uint16_t message = static_cast<uint16_t>(messageBits.to_ulong());

    // Extract 10-bit CRC
    std::bitset<10> crcBits(binaryStr.substr(i + 16, 10));
    uint16_t crc = static_cast<uint16_t>(crcBits.to_ulong());

    // Store the message and CRC in the struct
    dataChunks.push_back({message, crc});
  }

  return;

errorArgs:
  std::cerr << "Error: Invalid binary string format." << std::endl;
  exit(1);
}

/**
 * Parse input args 
 */
void parseArgs(int argc, char **argv, std::vector<InputMessage>& dataChunks) {

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      printHelp();
    } 
    else if (strcmp(argv[i], "-b") == 0) {
      if (i + 1 < argc) {
        std::string binaryStr = argv[++i]; // Move to the next argument for binary input
        if (binaryStr.length() % 26 != 0) {
          std::cerr << "Error: Binary string length must be a multiple of 26 bits." << std::endl;
          goto errorArgs;
        }
        parseBinaryString(binaryStr, dataChunks);
      } 
      else {
        std::cerr << "Error: Missing binary string after -b." << std::endl;
        goto errorArgs;
      }
    } 
    else {
      std::cerr << "Error: Unknown argument '" << argv[i] << "'." << std::endl;
      goto errorArgs;
    }
  }

  return;

errorArgs:
  std::cerr << "Error: Wrong arguments. Try using {-h|--help}" << std::endl;
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
 * 
 */
uint16_t invertCRC(uint16_t data, uint16_t magicConst){
  return data ^ magicConst;
}

/** 
 * Returns true if there were no error find in data
 */
bool matrixMultiplication(const std::bitset<H_ROWS>& data) {
  
  std::bitset<H_ROWS> result = data & H_TRANSPOSED[0];
  std::cout << "Data   " << data << endl;
  std::cout << "Matrix " << H_TRANSPOSED[0] << endl;
  std::cout << "AND    " << result << std::endl;
  std::cout << "XOR    " << result << std::endl;

  for (int i = 1; i < H_COLS; ++i) {
    std::bitset<H_ROWS> oneMatrix = data & H_TRANSPOSED[i];
    // now in the result xor every bit 1001 = 0   1110 = 1
    // i mean here: 
    
    // XOR all bits inside oneMatrix
    bool xorResult = 0; // Start with a 0
    for (int j = 0; j < H_ROWS; ++j) {
        xorResult ^= oneMatrix[j];  // XOR all the bits
    }

    std::cout << "Res    " << xorResult << std::endl;
    
    // result should always be 0
    if (xorResult == true){
      return false;
    }
    
  }
  return true;
}

/**
 * Decode input message and print it to output
 */
void decodeMessage(std::vector<InputMessage>& dataChunks){

  if (dataChunks.size() < 4) {
    std::cerr << "Error: Not enough data in the vector to decode." << std::endl;
    return;
  }

  // blocks can be in wrong order 2. 1. 3. 4 
  // also the messages inside block ABCD, BACD, DBCA 
  // Process each block of 4 messages
  bool aUsed = false, bUsed = false, cUsed = false, dUsed = false;
  std::bitset<H_ROWS> concatenated_data;
  for (size_t i = 0; i < dataChunks.size(); i += 4) {
    std::cout << "Block " << (i / 4 + 1) << std::endl;

    // Loop over the messages in the current block
    for (int j = 0; j < 4 && (i + j) < dataChunks.size(); ++j) {
      auto& chunk = dataChunks[i + j];

      if (aUsed == false){
        std::bitset<CRC_BITS> inverted_crc_a = invertCRC(chunk.crc, CRC_BLOCK_OFFSET_A);
        std::bitset<H_ROWS> concatenated_data = 
          (std::bitset<H_ROWS>(chunk.message) << CRC_BITS) | std::bitset<H_ROWS>(inverted_crc_a.to_ulong());
        
        if (matrixMultiplication(concatenated_data) == true){
          cout << "good block" << endl;
        }
        else {
          cout << "bad block" << endl;
          break;
        }
      }
            
      // Compute the inverted CRCs with different offsets
      std::bitset<CRC_BITS> inverted_crc_b = invertCRC(chunk.crc, CRC_BLOCK_OFFSET_B);
      std::bitset<CRC_BITS> inverted_crc_c = invertCRC(chunk.crc, CRC_BLOCK_OFFSET_C);
      std::bitset<CRC_BITS> inverted_crc_d = invertCRC(chunk.crc, CRC_BLOCK_OFFSET_D);

      // Concatenate message and inverted CRC (using offset A) into a single 26-bit bitset

      // Print message, CRC, and each inverted CRC with its offset
      std::cout << "Message: " << std::bitset<BLOCK_BITS>(chunk.message)
        << ", CRC: " << std::bitset<CRC_BITS>(chunk.crc)
        //<< ", Inverted CRC A: " << inverted_crc_a
        << ", Inverted CRC B: " << inverted_crc_b
        << ", Inverted CRC C: " << inverted_crc_c
        << ", Inverted CRC D: " << inverted_crc_d << std::endl;
      
      // here order the data
      // Perform matrix multiplication with concatenated data (using inverted CRC A)
    }
  }
}

int main(int argc, char** argv) {
  std::vector<InputMessage> dataChunks;

  // Parse command-line arguments
  parseArgs(argc, argv, dataChunks);

  decodeMessage(dataChunks);

  return 0;
}

