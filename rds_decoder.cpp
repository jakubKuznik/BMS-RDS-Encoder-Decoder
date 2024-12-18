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
  cerr << "Execution:" << endl;
  cerr << "  ./program_encoder {-b MESSAGE}" << endl; 
  cerr << "       Program reads from STDIN and prints output to STDOUT." << endl;
  cerr << "" << endl;
  cerr << "           MESSAGE" << endl;
  cerr << "             - Sequence of 0 and 1 that has to be a multiple of 26" << endl;
}

/**
 * Parse binary string and store in a vector of InputMessage
 */
void parseBinaryString(const string& binaryStr, vector<InputMessage>& dataChunks) {
  size_t length = binaryStr.length();

  // Ensure the input length is a multiple of 26 bits (16-bit message + 10-bit CRC)
  if (length % 26 != 0) {
    cerr << "Error: Binary string length must be a multiple of 26 bits." << endl;
    goto errorArgs;
  }

  // Parse 26-bit chunks (16-bit message + 10-bit CRC)
  for (size_t i = 0; i < length; i += 26) {
    // Extract 16-bit message
    bitset<16> messageBits(binaryStr.substr(i, 16));
    uint16_t message = static_cast<uint16_t>(messageBits.to_ulong());

    // Extract 10-bit CRC
    bitset<10> crcBits(binaryStr.substr(i + 16, 10));
    uint16_t crc = static_cast<uint16_t>(crcBits.to_ulong());

    // Store the message and CRC in the struct
    dataChunks.push_back({message, crc});
  }

  return;

errorArgs:
  cerr << "Error: Invalid binary string format." << endl;
  exit(2);
}

/**
 * Parse input args 
 */
void parseArgs(int argc, char **argv, vector<InputMessage>& dataChunks) {

  if (argc < 2){
    printHelp();
    exit(1);
  }

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      printHelp();
      exit(0);
    } 
    else if (strcmp(argv[i], "-b") == 0) {
      if (i + 1 < argc) {
        string binaryStr = argv[++i]; // Move to the next argument for binary input
        if (binaryStr.length() % 26 != 0) {
          cerr << "Error: Binary string length must be a multiple of 26 bits." << endl;
          goto errorArgs;
        }
        parseBinaryString(binaryStr, dataChunks);
      } 
      else {
        cerr << "Error: Missing binary string after -b." << endl;
        goto errorArgs;
      }
    } 
    else {
      cerr << "Error: Unknown argument '" << argv[i] << "'." << endl;
      goto errorArgs;
    }
  }

  return;

errorArgs:
  cerr << "Error: Wrong arguments. Try using {-h|--help}" << endl;
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
 * Invert CRC code
 */
uint16_t invertCRC(uint16_t data, uint16_t magicConst){
  return data ^ magicConst;
}

/** 
 * Returns true if there were no error find in data
 */
bool matrixMultiplication(const bitset<H_ROWS>& data) {
  
  for (int i = 1; i < H_COLS; ++i) {
    bitset<H_ROWS> oneMatrix = data & H_TRANSPOSED[i];
    // XOR all bits inside oneMatrix
    bool xorResult = 0; // Start with a 0
    for (int j = 0; j < H_ROWS; ++j) {
        xorResult ^= oneMatrix[j];  // XOR all the bits
    }

    // result should always be 0
    if (xorResult == true){
      return false;
    }
  }
  return true;
}

/** 
 * Order message in to correct order and returns orderedData 
 */
void orderMessage(vector<InputMessage>& dataChunks, vector<uint16_t>& orderedData){
  
  if (dataChunks.size() < 4) {
    cerr << "Error: Not enough data in the vector to decode." << endl;
    exit(2);
  }
  else if (dataChunks.size() % 4 != 0){
    cerr << "Error: Not enough data blocks." << endl;
    exit(2);
  }

  orderedData.resize(dataChunks.size());

  // blocks can be in wrong order 2. 1. 3. 4 
  // also the messages inside block ABCD, BACD, DBCA 
  // Process each block of 4 messages
  bitset<H_ROWS> concatenatedData;
  bitset<CRC_BITS> invertedCrc;
  bool is2A = false;
  bool is0A = false;
  for (size_t i = 0; i < (dataChunks.size()/4);i++) {
    
    // Loop over the messages in the current block
    bitset<H_ROWS> aMessage, bMessage, cMessage, dMessage; 
    bool aUsed = false, bUsed = false, cUsed = false, dUsed = false;
    for (size_t j = 0; j < 4 ; j++) {
      auto& chunk = dataChunks[(i*4) + j];
      if (aUsed == false){
        invertedCrc = invertCRC(chunk.crc, CRC_BLOCK_OFFSET_A);
        concatenatedData = (bitset<H_ROWS>(chunk.message) << CRC_BITS) | bitset<H_ROWS>(invertedCrc.to_ulong());
        
        if (matrixMultiplication(concatenatedData) == true){
          aMessage = chunk.message; aUsed = true; 
          continue;
        }
      }
      
      if (bUsed == false){
        invertedCrc = invertCRC(chunk.crc, CRC_BLOCK_OFFSET_B);
        concatenatedData = (bitset<H_ROWS>(chunk.message) << CRC_BITS) | bitset<H_ROWS>(invertedCrc.to_ulong());
        
        if (matrixMultiplication(concatenatedData) == true){
          bMessage = chunk.message; bUsed = true;
          continue;
        }
      }
      
      if (cUsed == false){
        invertedCrc = invertCRC(chunk.crc, CRC_BLOCK_OFFSET_C);
        concatenatedData = (bitset<H_ROWS>(chunk.message) << CRC_BITS) | bitset<H_ROWS>(invertedCrc.to_ulong());
        
        if (matrixMultiplication(concatenatedData) == true){
          cMessage = chunk.message; cUsed = true; 
          continue;
        }
      }
      
      if (dUsed == false){
        invertedCrc = invertCRC(chunk.crc, CRC_BLOCK_OFFSET_D);
        concatenatedData = (bitset<H_ROWS>(chunk.message) << CRC_BITS) | bitset<H_ROWS>(invertedCrc.to_ulong());
        
        if (matrixMultiplication(concatenatedData) == true){
          dMessage = chunk.message; dUsed = true; 
          continue;
        }
      }
      goto error_wrong_message;
    }

    uint16_t help = static_cast<uint16_t>(bMessage.to_ulong());
    if (((help >> 12) & 0b1111) == 0b0000) {
      is0A = true;
      if (is2A == true){
        goto error_multiple_groups;
      }


      orderedData[((help & 0b11) * 4) + 0] = static_cast<uint16_t>(aMessage.to_ulong()); 
      orderedData[((help & 0b11) * 4) + 1] = static_cast<uint16_t>(bMessage.to_ulong()); 
      orderedData[((help & 0b11) * 4) + 2] = static_cast<uint16_t>(cMessage.to_ulong()); 
      orderedData[((help & 0b11) * 4) + 3] = static_cast<uint16_t>(dMessage.to_ulong()); 
    } else if (((help >> 12) & 0b1111) == 0b0010) {
      is2A = true;
      if (is0A == true){
        goto error_multiple_groups;
      }
      orderedData[((help & 0b1111) * 4) + 0] = static_cast<uint16_t>(aMessage.to_ulong()); 
      orderedData[((help & 0b1111) * 4) + 1] = static_cast<uint16_t>(bMessage.to_ulong()); 
      orderedData[((help & 0b1111) * 4) + 2] = static_cast<uint16_t>(cMessage.to_ulong()); 
      orderedData[((help & 0b1111) * 4) + 3] = static_cast<uint16_t>(dMessage.to_ulong()); 
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
  exit(1);
}

/**
 * Returns frequency from 8bits 
 */
float parseBinaryToFrequency(uint8_t binaryValue) {
  
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
void decode0A(vector<uint16_t>& orderedData){

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

    // row 1 
    if (messageProperties.flagsCommon.pi != chunkA)
      goto error_inconsistent_blocks;

    // row 2 
    if (group != (chunkB >> 12))
      goto error_inconsistent_blocks;
    if (ab != ((chunkB >> 11) & 0b1))
      goto error_inconsistent_blocks;
    if (messageProperties.flagsCommon.tp != (((chunkB >> 10) &0b1)))
      goto error_inconsistent_blocks;
    if (messageProperties.flagsCommon.pty != (((chunkB >> 5) &0b11111)))
      goto error_inconsistent_blocks;
    if (messageProperties.flags0A.ta != ((chunkB >> 4) &0b1))
      goto error_inconsistent_blocks;
    if (messageProperties.flags0A.ms != ((chunkB >> 3) &0b1))
      goto error_inconsistent_blocks;
    if (di != ((chunkB >> 2) &0b1))
      goto error_inconsistent_blocks;
    // block index should be + 1 
    if (blockIndex+1 != (chunkB &0b11))
      goto error_inconsistent_blocks;
    
    blockIndex = chunkB & 0b11;
    messageProperties.flags0A.ps[(i/4)*2]   = (chunkD >> 8) & 0xff;
    messageProperties.flags0A.ps[(i/4)*2+1] = chunkD & 0xff;
  
  }

  // Print the extracted bits from chunkB
  cout << "PI: " << messageProperties.flagsCommon.pi << endl;
  cout << "GT: 0A" << endl;
  cout << "TP: " << bitset<1>(messageProperties.flagsCommon.tp) << endl;
  cout << "PTY: " << messageProperties.flagsCommon.pty << endl;
  if (messageProperties.flags0A.ta == 1){
    cout << "TA: Active" << endl;
  } else {
    cout << "TA: Inactive" << endl;
  }
  if (messageProperties.flags0A.ms == 1){
    cout << "MS: Music" << endl;
  } else {
    cout << "MS: Speech" << endl;
  }
  cout << "DI: " << bitset<1>(di) << endl;
  std::cout << std::fixed << setprecision(1); // Ensure 1 decimal place
  std::cout << "AF: " << messageProperties.flags0A.af[0]
    << ", " << messageProperties.flags0A.af[1] << std::endl;
  cout << "PS: \"";
  for (size_t i = 0; i < orderedData.size()/2; i++){
    if (i+1 < orderedData.size()/2){
      if (static_cast<char>(messageProperties.flags0A.ps[i]) == ' '){
        // this could be problemm for message like "haha    ha"
        if (static_cast<char>(messageProperties.flags0A.ps[i+1] == ' ')){
          break;
        }
      }
    } 
    cout << static_cast<char>(messageProperties.flags0A.ps[i]);
  }
  cout << "\"" << endl;

  return;

error_inconsistent_blocks: 
  cerr << "Inconsistent blocks" << endl;
  exit(2);

}

/**
 * 
 * Program Identification (PI)**: Output as a 16-bit unsigned integer.
 * Program Type (PTY)**: Output as a 5-bit unsigned integer.
 * Traffic Program (TP)**: Output as `TP: 1` or `TP: 0`.
 * Radio Text A/B flag (A/B)**: Output as `RT A/B: 0` or `RT A/B: 1`.
 * Radio Text (RT)**: Output the radio text string (64 characters maximum). 
 *      If shorter, the encoder will add padding with spaces.
 * 
 * PI: 4660
 * GT: 2A
 * TP: 1
 * PTY: 5
 * A/B: 0
 * RT: "Now Playing: Song Title by Artist"
 * 
*/
void decode2A(vector<uint16_t>& orderedData){
  
  MessageProperties messageProperties;

  // Loop over the messages in the current block (4 messages per block)
  auto& chunkA = orderedData[0]; auto& chunkB = orderedData[1];
  auto& chunkC = orderedData[2]; auto& chunkD = orderedData[3];

  // ROW 1 
  messageProperties.flagsCommon.pi = chunkA;
  // ROW 2  
  uint16_t group = chunkB >> 12; 
  if (group == 0b0010) {
    messageProperties.is2A = true;
  } else{
    cerr << "Wrong data in blocks"; exit(2);
  }
  uint8_t ab = (chunkB >> 11) & 0b1;
  messageProperties.flagsCommon.tp = (chunkB >> 10) & 0b1;
  messageProperties.flagsCommon.pty = (chunkB >> 5) & 0b11111;
  messageProperties.flags2A.ab = (chunkB >> 4) & 0b1;
  uint8_t blockIndex = chunkB & 0b1111;

  // row 3-4
  messageProperties.flags2A.rt[0] = ((chunkC >> 8) & 0xff);
  messageProperties.flags2A.rt[1] = (chunkC & 0xff);
  messageProperties.flags2A.rt[2] = ((chunkD >> 8) & 0xff);
  messageProperties.flags2A.rt[3] = (chunkD & 0xff);
  
  // Iterate over the data in blocks of 4 messages
  for (size_t i = 4; i < orderedData.size(); i += 4) {
    chunkA = orderedData[i];  chunkB = orderedData[i+1];
    chunkC = orderedData[i+2];chunkD = orderedData[i+3];

    // row 1 
    if (messageProperties.flagsCommon.pi != chunkA)
      goto error_inconsistent_blocks;
    // row 2 
    if (group != (chunkB >> 12))
      goto error_inconsistent_blocks;
    if (ab != ((chunkB >> 11) & 0b1))
      goto error_inconsistent_blocks;
    if (messageProperties.flagsCommon.tp != ((chunkB >> 10) &0b1))
      goto error_inconsistent_blocks;
    if (messageProperties.flagsCommon.pty != ((chunkB >> 5) &0b11111))
      goto error_inconsistent_blocks;
    if (messageProperties.flags2A.ab != ((chunkB >> 4) &0b1))
      goto error_inconsistent_blocks;
    // block index should be + 1 
    if (blockIndex+1 != (chunkB &0b1111))
      goto error_inconsistent_blocks;

    blockIndex = chunkB & 0b1111;
    messageProperties.flags2A.rt[i]   = (chunkC >> 8) & 0xff;
    messageProperties.flags2A.rt[i+1] = chunkC & 0xff;
    messageProperties.flags2A.rt[i+2] = (chunkD >> 8) & 0xff;
    messageProperties.flags2A.rt[i+3] = chunkD & 0xff;
  }
  
  // Print the extracted bits from chunkB
  cout << "PI: " << messageProperties.flagsCommon.pi << endl;
  cout << "GT: 2A" << endl;
  cout << "TP: " << bitset<1>(messageProperties.flagsCommon.tp) << endl;
  cout << "PTY: " << messageProperties.flagsCommon.pty << endl;
  cout << "A/B: " << bitset<1>(messageProperties.flags2A.ab) << endl;
  cout << "RT: \"";
  for (size_t i = 0; i < orderedData.size(); i++){
    if (i+1 < orderedData.size()){
      if (static_cast<char>(messageProperties.flags2A.rt[i]) == ' '){
        // todo this could be problemm for message like "haha    ha"
        if (static_cast<char>(messageProperties.flags2A.rt[i+1] == ' ')){
          break;
        }
      }
    } 
    cout << static_cast<char>(messageProperties.flags2A.rt[i]);
  }
  cout << "\"" << endl;

  return;

error_inconsistent_blocks: 
  cerr << "Inconsistent blocks" << endl;
  exit(2);
}

/**
 * decode input message and print it to stdout
*/
void decodeMessage(vector<InputMessage>& dataChunks){
  
  vector<uint16_t> orderedData;

  orderMessage(dataChunks, orderedData);

  // Check if the first 4 bits are 0000 or 0010
  if (((orderedData[1] >> 12) & 0b1111) == 0b0000) {
    decode0A(orderedData);
  } else if (((orderedData[1] >> 12) & 0b1111) == 0b0010) {
    decode2A(orderedData);
  } 

  return;
}

int main(int argc, char** argv) {
  vector<InputMessage> dataChunks;

  // Parse command-line arguments
  parseArgs(argc, argv, dataChunks);

  decodeMessage(dataChunks);

  return 0;
}

