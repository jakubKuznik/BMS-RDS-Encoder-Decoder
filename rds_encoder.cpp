// Faculty: BUT FIT 
// Course: BMS 
// Project Name: RDS Encoder and Decoder Project Overview
// Name: Jakub Kuznik
// Login: xkuzni04
// Year: 2024

#include "rds_encoder.hpp"

using namespace std;

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
    
    af[0] = freq1;
    af[1] = freq2;
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

    for (int i = 0; i < argc; i++) {
        std::cerr << "num: " << i <<  " Arg: " << argv[i] << std::endl;

        // Ensure each argument has a corresponding value
        if (i + 1 == argc && argc != 1) {
            goto errorArgs;
        }

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
                parseStringArg(argv[i], config->flags2A.rt, RT_SIZE_PLUS_TERMINATOR, false);  // No padding, just limit to 64 characters
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


int main(int argc, char **argv){
	
	ProgramConfig config;

	argParse(argc, argv, &config);



}