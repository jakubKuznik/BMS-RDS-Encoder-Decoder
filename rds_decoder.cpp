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
  if (argc < 3) {
    std::cerr << "Error: Missing required arguments." << std::endl;
    goto errorArgs;
  }

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

int main(int argc, char** argv) {
  std::vector<InputMessage> dataChunks;

  // Parse command-line arguments
  parseArgs(argc, argv, dataChunks);

  // Output parsed data
  std::cout << "Parsed Data:" << std::endl;
  for (const auto& chunk : dataChunks) {
    std::cout << "Message: " << std::bitset<16>(chunk.message) 
    << ", CRC: " << std::bitset<10>(chunk.crc) << std::endl;
  }

  return 0;
}

