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

/**
 * Parse input args
*/
void argParse(int argc, char **argv, ProgramConfig *config){

	config->is0A = false;
	config->is2A = false;

	bool commonPi = false, commonPty = false, commonTp = false;
	bool flags0Ams = false, flags0Ata = false, flags0Aaf = false, flags0Aps = false;
	bool flags2Art = false, flags2Aab = false;

	for (int i = 0; i < argc; i++){
		cerr << "num: " << i <<  " Arg: " << argv[i] << endl;
		// each argument should have space for another value only help is an exception 
		if (i+1 == argc && argc != 1){
			goto errorArgs;
		}
		if (strcmp(argv[i],"--help") == 0 || strcmp(argv[i], "-h") == 0){
			printHelp();
		}
		else if (strcmp(argv[i], "-g") == 0){
			i++;
			if (strcmp(argv[i], "0A") == 0){
				config->is0A = true;
			}
			else if (strcmp(argv[i], "2A") == 0){
				config->is2A = true;	
			}
			else {
				goto errorArgs;
			}
		}
		else if (strcmp(argv[i], "-pi") == 0){
			i++;
			try {
                int myInt = std::stoi(argv[i]); 
                if (myInt <= static_cast<int>(UINT16_MAX) && myInt >= 0) {
                    config->flagsCommon.pi = static_cast<uint16_t>(myInt);
                } else {
                    goto errorArgs; 
                }
                std::cerr << myInt << std::endl;
            } catch (...) { 
                goto errorArgs; 
            }
		}
	}
	
	if (config->is0A == true && config->is2A == true)
		goto errorArgs;
	if (config->is0A == false && config->is2A == false)
		goto errorArgs;
	// if (commonPi == false && commonPty == false && commonTp == false)
	// 	goto errorArgs;
	
	return;

errorArgs:
	cerr << "Error: wrong arguments. Try using {-h|--help}" << endl;
	exit(1);
}


int main(int argc, char **argv){
	
	ProgramConfig config;

	argParse(argc, argv, &config);



}