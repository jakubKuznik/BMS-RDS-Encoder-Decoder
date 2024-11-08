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
  //std::cout << "Data   " << data << endl;
  //std::cout << "Matrix " << H_TRANSPOSED[0] << endl;
  //std::cout << "AND    " << result << std::endl;
  //std::cout << "XOR    " << result << std::endl;

  for (int i = 1; i < H_COLS; ++i) {
    std::bitset<H_ROWS> oneMatrix = data & H_TRANSPOSED[i];
    // XOR all bits inside oneMatrix
    bool xorResult = 0; // Start with a 0
    for (int j = 0; j < H_ROWS; ++j) {
        xorResult ^= oneMatrix[j];  // XOR all the bits
    }

    //std::cout << "Res    " << xorResult << std::endl;
    
    // result should always be 0
    if (xorResult == true){
      return false;
    }
    
  }
  return true;
}

/**
 * Decode input message and print it to output
- **Program Identification (PI)**: Output as a 16-bit unsigned integer.
- **Program Type (PTY)**: Output as a 5-bit unsigned integer.
- **Traffic Program (TP)**: Output as `TP: 1` or `TP: 0`.
- **Music/Speech (MS)**: Output as `MS == 1: Music` or `MS == 0: Speech`.
- **Traffic Announcement (TA)**: Output as `TA == 1: Active` or `TA == 0: Inactive`.
- **Alternative Frequencies (AF)**: Output as two frequency values (e.g., `AF: 104, 98`).
- **Program Service (PS)**: Output the station name (8-character string).



- **Program Identification (PI)**: Output as a 16-bit unsigned integer.
- **Program Type (PTY)**: Output as a 5-bit unsigned integer.
- **Traffic Program (TP)**: Output as `TP: 1` or `TP: 0`.
- **Radio Text A/B flag (A/B)**: Output as `RT A/B: 0` or `RT A/B: 1`.
- **Radio Text (RT)**: Output the radio text string (64 characters maximum). If shorter, the encoder will add padding with spaces.

 */
void decodeMessage(std::vector<InputMessage>& dataChunks){

  if (dataChunks.size() < 4) {
    std::cerr << "Error: Not enough data in the vector to decode." << std::endl;
    exit(2);
  }
  else if (dataChunks.size() % 4 != 0){
    std::cerr << "Error: Not enough data blocks." << std::endl;
    exit(2);
  }

  cout << "Data chunks: " << dataChunks.size() << endl;

  // blocks can be in wrong order 2. 1. 3. 4 
  // also the messages inside block ABCD, BACD, DBCA 
  // Process each block of 4 messages
  std::bitset<H_ROWS> concatenatedData;
  std::bitset<CRC_BITS> invertedCrc; 
  for (size_t i = 0; i < (dataChunks.size()/4);i++) {
    std::cout << "Message " << i << std::endl;
    
    // Loop over the messages in the current block
    std::bitset<H_ROWS> aMessage, bMessage, cMessage, dMessage; 
    bool aUsed = false, bUsed = false, cUsed = false, dUsed = false;
    for (size_t j = 0; j < 4 ; j++) {
      auto& chunk = dataChunks[(i*4) + j];
      cout << "data: " << (i*4) + j << " " << std::bitset<16>(chunk.message) << " " << std::bitset<10>(chunk.crc) << " ";

      if (aUsed == false){
        invertedCrc = invertCRC(chunk.crc, CRC_BLOCK_OFFSET_A);
        concatenatedData = (std::bitset<H_ROWS>(chunk.message) << CRC_BITS) | std::bitset<H_ROWS>(invertedCrc.to_ulong());
        
        if (matrixMultiplication(concatenatedData) == true){
          cout << "good block" << endl;
          aMessage = concatenatedData; aUsed = true; continue;
        }
      }
      
      if (bUsed == false){
        invertedCrc = invertCRC(chunk.crc, CRC_BLOCK_OFFSET_B);
        concatenatedData = (std::bitset<H_ROWS>(chunk.message) << CRC_BITS) | std::bitset<H_ROWS>(invertedCrc.to_ulong());
        
        if (matrixMultiplication(concatenatedData) == true){
          cout << "good block" << endl;
          bMessage = concatenatedData; bUsed = true;continue;
        }
      }
      
      if (cUsed == false){
        invertedCrc = invertCRC(chunk.crc, CRC_BLOCK_OFFSET_C);
        concatenatedData = (std::bitset<H_ROWS>(chunk.message) << CRC_BITS) | std::bitset<H_ROWS>(invertedCrc.to_ulong());
        
        if (matrixMultiplication(concatenatedData) == true){
          cout << "good block" << endl;
          cMessage = concatenatedData; cUsed = true; continue;
        }
      }
      
      if (dUsed == false){
        invertedCrc = invertCRC(chunk.crc, CRC_BLOCK_OFFSET_D);
        concatenatedData = (std::bitset<H_ROWS>(chunk.message) << CRC_BITS) | std::bitset<H_ROWS>(invertedCrc.to_ulong());
        
        if (matrixMultiplication(concatenatedData) == true){
          cout << "good block" << endl;
          dMessage = concatenatedData; dUsed = true; continue;
        }
      }
      goto error_wrong_message;
    }
  }


  return;

error_wrong_message:
  cerr << "Wrong message" << endl;
  exit(2);
    

}

int main(int argc, char** argv) {
  std::vector<InputMessage> dataChunks;

  // Parse command-line arguments
  parseArgs(argc, argv, dataChunks);

  decodeMessage(dataChunks);

  return 0;
}

