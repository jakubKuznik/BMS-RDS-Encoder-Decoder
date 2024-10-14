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
	cout << "Execution:" << endl;
	cout << "  ./program_encoder {-g {0A,2A}} [FLAGS]" << endl; 
	cout << "       Program reads from STDIN and print output to STDOUT.";
	cout << "  Common FLAGS" << endl;
	cout << "    -pi" << endl;
	cout << "        Program identification (16-bit unsigned integer.)" << endl;
	cout << "    -pty" << endl;
	cout << "        Program Type. (5-bit unsigned integer, values 0-31)" << endl;
	cout << "    -tp" << endl;
	cout << "        Traffic Program (boolean, 0 or 1)" << endl;
	cout << endl;
	cout << "  0A-Specific FLAGS" << endl;
	cout << "    -ms" << endl;
	cout << "        Music/Speech (boolean, 0 for Sepech, 1 for Music)" << endl;
	cout << "    -ta" << endl;
	cout << "        Traffic Annoucement (boolean, 0 or 1)" << endl;
	cout << "    -af" << endl;
	cout << "        Alterantive Frequencies (two comma-separated frequency values, e.g., -af 104.5,98.0). \
							Always float numers with precision to 0.1" << endl;
	cout << "    -ps" << endl;
	cout << "        Program Service (8-character string). If shorter, the encoder will add padding with spaces." << endl;
	cout << endl;
	cout << "  2A-Specific FLAGS" << endl;
	cout << "    -rt" << endl;
	cout << "        Radio Text (string up to 64 characters)" << endl;
	cout << "    -ab" << endl;
	cout << "        Radio text A/B flag (boolean, 0 or 1)." << endl;
	cout << endl;
	cout << "Example: " << endl;
    cout << "  ./rds_encoder -g 0A -pi 4660 -pty 5 -tp 1 -ms 0 -ta 1 -af 104.5,98.0 -ps \"RadioXYZ\"" << endl;
	exit(1);
}

/**
 * Parse input args
*/
void argParse(int argc, char **argv, ProgramConfig *config){

	
	// if there are no args 
	if (argc == 1){
		printHelp();
	}

	for (int i = 1; i < argc; i++){
		if (strcmp(argv[i],"-s") == 0){
			cout << "hi" << endl;
		}
	}

	return;

//errorArgs:
//	cerr << "Error: wrong arguments. Try run without args for help message!" << endl;
//	exit(2);
}


int main(int argc, char **argv){
	
	ProgramConfig config;
    cout << "Hello world" << endl;

	argParse(argc, argv, &config);



}