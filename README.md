# RDS Encoder and Decoder Project Overview

## Objective:
The goal of this project is to develop a **Radio Data System (RDS) Encoder and Decoder** for the **Group 0A** and **2A** message types. The project involves simulating the encoding and decoding of RDS data and ensuring data integrity using **Cyclic Redundancy Check (CRC)**. The program must be written in **C++14** using only standard libraries, compiled with **GCC 11.5**.

## Project Overview:

In this project, you will:

- **Implement an Encoder** that encodes the provided input (station ID, program type, traffic information, etc.) into a specific Group message structure. Use offset and group address to specify the order of blocks and groups.
- **Implement a Decoder** that receives an encoded RDS Group message and decodes it back into the original fields.
  - The decoder must correctly identify the group type and restore the original message fields.
  - The order of incoming groups must be correctly identified based on the group address. Additionally, block synchronization within each group must be handled according to the standard offset words (A, B, C, or D) to correctly identify each block's position within the group.
- The encoder will generate multiple groups if necessary, and the decoder will be able to process inputs that are **104, 208, or more bits** long if received.
- Verify the integrity of the decoded message using **CRC** and exit with the appropriate error code if any errors are detected.

## Background on RDS Group 0A:

RDS Group 0A is used to transmit basic tuning and switching information in radio broadcasts. A Group 0A message consists of **4 blocks**, each **26 bits** long, for a total of **104 bits**. Each block contains:

- **16 bits of data** (Program Identification, Program Type, etc.)
- **10 bits of CRC** for error detection.

### The message transmits key data, such as:

- **Program Identification (PI)**: A unique identifier for the station.
- **Traffic Program (TP)**: Indicates whether the station broadcasts traffic announcements.
- **Program Type (PTY)**: Indicates the type of program.
- **Traffic Announcement (TA)**: If a traffic announcement is currently in progress.
- **Music/Speech (MS)**: Indicates whether the current broadcast is music or speech.
- **Decoder Identification (DI)**: Provides information to the radio receiver about how it should decode and interpret certain broadcast features. For this project, this will always be set to 0.
- **Alternative Frequencies (AF)**: A list of alternative frequencies where the same program can be received.
- **Program Service (PS)**: A short 8-character station name, transmitted 2 characters at a time. Expect only `[a-zA-Z0-9 ]*`.

## Background on RDS Group 2A:

RDS Group 2A is used to transmit **Radio Text (RT)**, which provides additional textual information, such as song titles, artist names, station messages, or other information broadcast by the station. A Group 2A message consists of **4 blocks**, each **26 bits** long, for a total of **104 bits**. Each block contains:

- **16 bits of data** (Radio Text, Program Identification, etc.)
- **10 bits of CRC** for error detection.

### The message transmits key data, such as:

- **Program Identification (PI)**: A unique identifier for the station.
- **Traffic Program (TP)**: Indicates whether the station broadcasts traffic announcements.
- **Program Type (PTY)**: Indicates the type of program.
- **Radio Text A/B flag (A/B)**: Used to signal when the text is updated.
- **Segment Address**: Identifies which segment of the Radio Text message is being transmitted.
- **Radio Text (RT)**: The actual text being transmitted, up to 64 characters long, sent in segments (8 characters per Group 2A message). Expect only `[a-zA-Z0-9 ]*`.

## Requirements:
### Part 1: RDS Encoder

- The specific group will be defined by the `-g` parameter:
  - `-g 0A`: Encode Group 0A.
  - `-g 2A`: Encode Group 2A.

# Common Flags:

- `-pi`: Program Identification (16-bit unsigned integer).
- `-pty`: Program Type (5-bit unsigned integer, values 0-31).
- `-tp`: Traffic Program (boolean, 0 or 1).

## Group 0A-Specific Flags:

- `-ms`: Music/Speech (boolean, 0 for Speech, 1 for Music).
- `-ta`: Traffic Announcement (boolean, 0 or 1).
- `-af`: Alternative Frequencies (two comma-separated frequency values, e.g., `-af 104.5,98.0`). Always float numbers with precision to 0.1.
- `-ps`: Program Service (8-character string). If shorter, the encoder will add padding with spaces.

## Group 2A-Specific Flags:

- `-rt`: Radio Text (string up to 64 characters).
- `-ab`: Radio Text A/B flag (boolean, 0 or 1).

## Example 1: Group 0A
`./rds_encoder -g 0A -pi 4660 -pty 5 -tp 1 -ms 0 -ta 1 -af 104.5,98.0 -ps "RadioXYZ"`
## Example 2: Group 2A
`./rds_encoder -g 2A -pi 4660 -pty 5 -tp 1 -rt "Now Playing: Song Title by Artist" -ab 0`

## Output:

The encoder should output the `104*(number of required groups)`-bit encoded message as a binary string to the standard output.

---

# Part 2: RDS Decoder

- Implement a C++ program that takes a binary string as input through the `-b` flag.
- The program will automatically determine the group type (0A or 2A) from the binary string and decode the message accordingly.

## Input Format:

The binary string should be exactly `104*(number of groups)` bits long, composed of '1's and '0's, representing the encoded RDS message.

## Example: Group 0A

```./rds_decoder -b 00010010001101000001101010000001001011000011111111101010101001101001000001101101010010011000011010101001000100100011010000011010100000010010110001100100011100000000000000000101101000011001000110100111110001100001001000110100000110101000000100101100100010001100000000000000000001011010000110111101011000010011101000010010001101000001101010000001001011001101001101010000000000000000010110100001011001010110100000100100```


---

## Output Format for Group 0A:

- **Program Identification (PI)**: Output as a 16-bit unsigned integer.
- **Program Type (PTY)**: Output as a 5-bit unsigned integer.
- **Traffic Program (TP)**: Output as `TP: 1` or `TP: 0`.
- **Music/Speech (MS)**: Output as `MS == 1: Music` or `MS == 0: Speech`.
- **Traffic Announcement (TA)**: Output as `TA == 1: Active` or `TA == 0: Inactive`.
- **Alternative Frequencies (AF)**: Output as two frequency values (e.g., `AF: 104, 98`).
- **Program Service (PS)**: Output the station name (8-character string).

### Example Output for Group 0A:

```
PI: 4660
GT: 0A
TP: 1
PTY: 5
TA: Active
MS: Speech
DI: 1
AF: 104.5, 98.0
PS: "RadioXYZ"
```

---

## Output Format for Group 2A:

- **Program Identification (PI)**: Output as a 16-bit unsigned integer.
- **Program Type (PTY)**: Output as a 5-bit unsigned integer.
- **Traffic Program (TP)**: Output as `TP: 1` or `TP: 0`.
- **Radio Text A/B flag (A/B)**: Output as `RT A/B: 0` or `RT A/B: 1`.
- **Radio Text (RT)**: Output the radio text string (64 characters maximum). If shorter, the encoder will add padding with spaces.


## Example: Group 2A

```./rds_decoder -b 00010010001101000001101010001001001010000011111011100100111001101111100111101101110111001000001100100100000100100011010000011010100010010010100001100101011101010000011011001000101010011000010111100111110101010001001000110100000110101000100100101000100010011100011010010110111010011110010110011100111010111110000100010010001101000001101010001001001010001101001001010010000001010011111000010001101111011011100001101101000100100011010000011010100010010010100100001011001101100111001000000011110011010101000110100111011000100001001000110100000110101000100100101001010100001010011101000110110010111101010110010100100000011001011100010010001101000001101010001001001010011011110000010110001001111001100110110100100000010000011110010101000100100011010000011010100010010010100111100111100001110010011101000010000001011010010111001100111111010001001000110100000110101000100100101010000011101101011101000010000010100111000010000000100000001101110000010010001101000001101010001001001010100101010101000010000000100000000000000000100000001000000011011100000100100011010000011010100010010010101010111001111100100000001000000000000000001000000010000000110111000001001000110100000110101000100100101010111000100110001000000010000000000000000010000000100000001101110000010010001101000001101010001001001010110011101100000010000000100000000000000000100000001000000011011100000100100011010000011010100010010010101101100000100100100000001000000000000000001000000010000000110111000001001000110100000110101000100100101011100011000010001000000010000000000000000010000000100000001101110000010010001101000001101010001001001010111101011110110010000000100000000000000000100000001000000011011100```

## Example Output for Group 2A:

```
PI: 4660
GT: 2A
TP: 1
PTY: 5
A/B: 0
RT: "Now Playing: Song Title by Artist"
```
# Part 3: Error Handling

## Error Handling in Encoder/Decoder for Invalid Arguments
- If the program receives invalid or missing command-line arguments, it should return exit code 1.

## Error Handling in Decoder:
- **CRC Verification**: Verify the CRC for each block during decoding.
- If any errors are detected, return a 2 error code.

# Deliverables:

## C++ Source Code:
- **Encoder** (`rds_encoder.cpp`, `rds_encoder.hpp`): Should accept command-line arguments as specified and output the encoded binary string.
- **Decoder** (`rds_decoder.cpp`, `rds_decoder.hpp`): Should accept the binary string via the -b flag and output the decoded fields or appropriate error codes.
- **Makefile** (`Makefile`): Will create binary files for `rds_encoder` and `rds_decoder`.
- **Documentation** (`xlogin00.pdf`): PDF file with project report.
- **Others**: Any additional `.hpp` and `.cpp` files that your program uses.
- **Archive** (`xlogin00.zip`): An archive file that will contain all files above in its root directory.

## Project Report:
A PDF report that includes:
- **Introduction**: An overview of the RDS system and Groups 0A and 2A.
- **Implementation**: A description of the C++ encoder and decoder logic.
- **Error Detection**: An explanation of how CRC is used for error detection, and how your program handles errors.
- **Challenges**: Any challenges you encountered during implementation and how you solved them.
- **Testing**: Include screenshots or logs demonstrating both successful decoding and error detection.

# Important Submission Notice:

Before submitting your project, you must verify that the content of your zip file meets the required structure by using the provided `check_zip.sh` script. This ensures that all necessary files are included and correctly named.

## Steps to Verify:
- Ensure your zip file is named according to your student login, following this format: `xlogin00.zip`.
- Run the following script to verify your submission:
  ```bash
  ./check_project.sh xlogin00.zip

- Warning: If the submitted zip file is missing any required files or if the contents of the zip file differ from what is expected, the project will automatically be graded 0 points.
Tools & Constraints:

## Tools & Constraints:

- **Programming Language:** C++ using only standard libraries (no external libraries allowed).
- **CRC-10 Implementation:** Must be done manually using bitwise operations.
- **Error Simulation:** Implemented within the encoder program as specified.
- **Modularity:** Ensure modularity in the code (e.g., separate functions for encoding, decoding, CRC calculation, and error simulation).
- **Input Validation:** Programs should validate all input parameters and handle invalid inputs gracefully, providing helpful error messages.

## Evaluation Criteria:

- **Correctness:**
    - The encoder correctly encodes the input fields into the Group format.
    - The decoder accurately extracts the fields from the encoded message.
    - CRC correctly detects errors in corrupted transmissions.

- **Error Handling:**
    - The decoder properly detects errors in the input message using CRC and returns the appropriate exit codes.

- **Code Quality:**
    - Clean, well-commented, and modular code.
    - Appropriate use of standard libraries and C++ features.

- **Documentation:**
    - The project report should clearly explain the implementation and any challenges encountered.
    - Include evidence of testing, such as screenshots or logs showing both correct decoding and error detection.

## Warnings:

- Use of non-standard libraries will result in **0 points**.
- Any unauthorized cooperation or plagiarism will result in **0 points**.
- If the submitted zip file is missing any required files or if the contents of the zip file differ from what is expected, the project will automatically be graded **0 points**.
- The final project must be fully executable on the reference machine Merlin (merlin.fit.vutbr.cz).

## Deadline:

Please submit your source code and report by **10.12.2024**.

## Consultations:

For any questions or issues, consultations are provided by **Ing. Filip Pleško** (iplesko@fit.vut.cz) and **Ing. Milan Šalko** (isalko@fit.vut.cz).

## Notes and Clarifications:

- **Alternative Frequencies (-af):**
    - The `-af` flag expects two comma-separated frequency values (e.g., `-af 104.5,98.0`), representing two alternative frequencies.
    - Example of conversion to decimal for transmission: `((104.5 * 10) - 875) = 170`.

- **Command-Line Argument Parsing:**
    - Ensure that your programs can handle missing or extra arguments gracefully.
    - Provide usage information if the user provides invalid arguments (e.g., `./rds_encoder --help`).

- **Binary String Input for Decoder:**
    - The `-b` flag should accept a string of exactly `104*(number of groups)` bits (consisting of '0's and '1's). The program should validate the length and content of this string.

- **Error Codes in Decoder:**
    - The decoder should exit with the specified exit code immediately upon detecting an error in any block, without printing any output.

- **Block Offsets:**
    - A: `0011111100` - 252
    - B: `0110011000` - 408
    - C: `0101101000` - 360
    - D: `0110110100` - 436

- **CRC Calculation:**
    - Use the standard CRC-10 polynomial used in RDS: `x^10 + x^8 + x^7 + x^5 + x^4 + x^3 + 1`.
    - Implement the CRC calculation manually using bitwise operations; do not use any built-in functions for CRC.

- **Group Type Determination:**
    - The decoder should determine the group type (0A or 2A) from the appropriate bits in the received message.

- **Testing:**
    - Test your encoder and decoder thoroughly, including cases with and without errors.

- **Literature:**
    - [EN50067 RDS Standard](http://www.interactive-radio-system.com/docs/EN50067_RDS_Standard.pdf)


 