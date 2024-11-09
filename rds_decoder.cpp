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
 * Order message and returns orderedData 
 */
void orderMessage(std::vector<InputMessage>& dataChunks, std::vector<uint16_t>& orderedData){
  
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
  bool is2A = false;
  bool is0A = false;
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

          aMessage = chunk.message; aUsed = true; continue;
        }
      }
      
      if (bUsed == false){
        invertedCrc = invertCRC(chunk.crc, CRC_BLOCK_OFFSET_B);
        concatenatedData = (std::bitset<H_ROWS>(chunk.message) << CRC_BITS) | std::bitset<H_ROWS>(invertedCrc.to_ulong());
        
        if (matrixMultiplication(concatenatedData) == true){
          cout << "good block" << endl;
          bMessage = chunk.message; bUsed = true;continue;
        }
      }
      
      if (cUsed == false){
        invertedCrc = invertCRC(chunk.crc, CRC_BLOCK_OFFSET_C);
        concatenatedData = (std::bitset<H_ROWS>(chunk.message) << CRC_BITS) | std::bitset<H_ROWS>(invertedCrc.to_ulong());
        
        if (matrixMultiplication(concatenatedData) == true){
          cout << "good block" << endl;
          cMessage = chunk.message; cUsed = true; continue;
        }
      }
      
      if (dUsed == false){
        invertedCrc = invertCRC(chunk.crc, CRC_BLOCK_OFFSET_D);
        concatenatedData = (std::bitset<H_ROWS>(chunk.message) << CRC_BITS) | std::bitset<H_ROWS>(invertedCrc.to_ulong());
        
        if (matrixMultiplication(concatenatedData) == true){
          cout << "good block" << endl;
          dMessage = chunk.message; dUsed = true; continue;
        }
      }
      goto error_wrong_message;
    }

    // Check the first 4 bits of bMessage if it's a "good block"
    std::string group = bMessage.to_string().substr(0, 4);
    if (group == "0000") {
      cout << "First 4 bits of bMessage are 0000" << endl;
      if (is2A == true){
        goto error_multiple_groups;
      }
      orderedData.push_back(static_cast<uint16_t>(aMessage.to_ulong()));
      orderedData.push_back(static_cast<uint16_t>(bMessage.to_ulong()));
      orderedData.push_back(static_cast<uint16_t>(cMessage.to_ulong()));
      orderedData.push_back(static_cast<uint16_t>(dMessage.to_ulong()));
      is0A = true;
    } else if (group == "0010") {
      // todo 2row last 4 bits 
      // decode the num and story to specific index
      cout << "First 4 bits of bMessage are 0010" << endl;
      if (is0A == true){
        goto error_multiple_groups;
      }
      is2A = true;
    } else {
      
      goto error_unsuported_format;
    }

  }

  return;

error_wrong_message:
  cerr << "Wrong message" << endl;
  exit(2);
error_multiple_groups:
  cerr << "Error multiple groups 0A 2A" << endl;
  exit(2);
error_unsuported_format:
  cerr << "Unsuported format" << endl;
  exit(2);

}

/**
 * Returns frequency from 8bits 
 */
float parseBinaryToFrequency(uint8_t binaryValue) {
  // Check if binaryValue is within the valid range (0-255)
  if (binaryValue > 255) {
    throw std::invalid_argument("Binary value out of range for frequency.");
  }

  // Calculate the frequency: divide by 10 and add 87.5 MHz
  float frequency = 87.5 + (binaryValue / 10.0);

  return frequency;
}

/**
 *
 * Program Identification (PI): Output as a 16-bit unsigned integer.
 * Program Type (PTY): Output as a 5-bit unsigned integer.
 * Traffic Program (TP): Output as TP: 1 or TP: 0.
 * Music/Speech (MS): Output as MS == 1: Music or MS == 0: Speech.
 * Traffic Announcement (TA): Output as TA == 1: Active or TA == 0: Inactive.
 * Alternative Frequencies (AF): Output as two frequency values (e.g., AF: 104, 98).
 * Program Service (PS): Output the station name (8-character string).
 * 
 * PI: 4660
 * GT: 0A
 * TP: 1
 * PTY: 5
 * TA: Active
 * MS: Speech
 * DI: 1
 * AF: 104.5, 98.0
 * PS: "RadioXYZ"
*/
void decode0A(std::vector<uint16_t>& orderedData){

  MessageProperties messageProperties;

  // Loop over the messages in the current block (4 messages per block)
  auto& chunkA = orderedData[0]; auto& chunkB = orderedData[1];
  auto& chunkC = orderedData[2]; auto& chunkD = orderedData[3];

  // ROW 1 
  messageProperties.flagsCommon.pi = chunkA;
  
  // ROW 2  
  uint16_t group = chunkB >> 12; 
  if (group == 0b0000) {
    messageProperties.is0A = true;
  } 
  else{
    cerr << "Wrong data in blocks"; exit(2);
  }
  uint8_t ab = (chunkB >> 11) & 0b1;
  messageProperties.flagsCommon.tp = (chunkB >> 10) & 0b1;
  messageProperties.flagsCommon.pty = (chunkB >> 5) & 0b11111;
  messageProperties.flags0A.ta = (chunkB >> 4) & 0b1;
  messageProperties.flags0A.ms = (chunkB >> 3) & 0b1;
  uint8_t di = (chunkB >> 2) & 0b1;
  uint8_t blockIndex = chunkB & 0b11;

  // row 3 
  messageProperties.flags0A.af[0] = parseBinaryToFrequency((chunkC >> 8) & 0xff);
  messageProperties.flags0A.af[1] = parseBinaryToFrequency(chunkC & 0xff);

  // row 4 
  messageProperties.flags0A.ps[0] = (chunkD >> 8) & 0xff; 
  messageProperties.flags0A.ps[1] = chunkD & 0xff; 
  
  // Iterate over the data in blocks of 4 messages
  for (size_t i = 4; i < orderedData.size(); i += 4) {
    chunkA = orderedData[i];  chunkB = orderedData[i+1];
    chunkC = orderedData[i+2];chunkD = orderedData[i+3];

    
    

  
  }
  
  // Print the extracted bits from chunkB
  std::cout << "pi: " << messageProperties.flagsCommon.pi << std::endl;
  std::cout << "Group: " << std::bitset<4>(group) << std::endl;
  std::cout << "TP: " << std::bitset<1>(messageProperties.flagsCommon.tp) << std::endl;
  std::cout << "PTY: " << messageProperties.flagsCommon.pty << std::endl;
  std::cout << "TA: " << std::bitset<1>(messageProperties.flags0A.ta) << std::endl;
  std::cout << "MS: " << std::bitset<1>(messageProperties.flags0A.ms) << std::endl;
  std::cout << "DI: " << std::bitset<1>(di) << std::endl;
  std::cout << "AB: " << std::bitset<1>(ab) << std::endl;
  // Print `Freq` values in binary form
  std::cout << "Freq 1: " << std::bitset<8>((chunkC >> 8) & 0xff) << " (" << messageProperties.flags0A.af[0] << " MHz)" << std::endl;
  std::cout << "Freq 2: " << std::bitset<8>(chunkC & 0xff) << " (" << messageProperties.flags0A.af[1] << " MHz)" << std::endl;
  // Print `Block Index` in binary form

  // Print `Data` in binary form, as well as in character form
  std::cout << "Data: " 
    << std::bitset<8>(messageProperties.flags0A.ps[0]) << " " 
    << std::bitset<8>(messageProperties.flags0A.ps[1]) 
    << " (" << static_cast<char>(messageProperties.flags0A.ps[0]) 
    << static_cast<char>(messageProperties.flags0A.ps[1]) << ")" 
    << std::endl;

  return;

error_inconsisten_blocks: 
  cerr << "Inconsistent blocks" << endl;
  exit(2);

}

/**
 * 
 * Program Identification (PI)**: Output as a 16-bit unsigned integer.
 * Program Type (PTY)**: Output as a 5-bit unsigned integer.
 * Traffic Program (TP)**: Output as `TP: 1` or `TP: 0`.
 * Radio Text A/B flag (A/B)**: Output as `RT A/B: 0` or `RT A/B: 1`.
 * Radio Text (RT)**: Output the radio text string (64 characters maximum). If shorter, the encoder will add padding with spaces.
 * 
 * PI: 4660
 * GT: 2A
 * TP: 1
 * PTY: 5
 * A/B: 0
 * RT: "Now Playing: Song Title by Artist"
 * 
*/
void decode2A(std::vector<uint16_t>& orderedData){

}

/**
 * decode input message  
*/
void decodeMessage(std::vector<InputMessage>& dataChunks){
  
  std::vector<uint16_t> orderedData;

  orderMessage(dataChunks, orderedData);

  // The second block in the orderedData
  uint16_t secondValue = orderedData[1]; 
  uint16_t firstFourBits = secondValue >> 12; 
        
  // Check if the first 4 bits are 0000 or 0010
  if (firstFourBits == 0b0000) {
    decode0A(orderedData);
  } else if (firstFourBits == 0b0010) {
    decode2A(orderedData);
  } 

  return;
}

int main(int argc, char** argv) {
  std::vector<InputMessage> dataChunks;

  // Parse command-line arguments
  parseArgs(argc, argv, dataChunks);

  decodeMessage(dataChunks);

  return 0;
}

