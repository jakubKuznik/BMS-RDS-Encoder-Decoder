# Faculty: BUT FIT 
# Course: BMS 
# Project Name: RDS Encoder and Decoder
# Name: Jakub Kuznik
# Login: xkuzni04
# Year: 2024

CC = g++

SANITIZE_FLAGS = -fsanitize=address,leak 
CFLAGS = -g -Wpedantic -Wall -Wextra $(SANITIZE_FLAGS) 

all: rds_encoder rds_decoder

# Build rds_encoder executable
rds_encoder: rds_encoder.cpp rds_encoder.hpp
	$(CC) $(CFLAGS) -o rds_encoder rds_encoder.cpp

# Build rds_decoder executable
rds_decoder: rds_decoder.cpp rds_decoder.hpp
	$(CC) $(CFLAGS) -o rds_decoder rds_decoder.cpp

# Clean up object files and executables
clean:
	rm -f *.o rds_encoder rds_decoder

# Create a zip file for submission
zip: Makefile rds_encoder.cpp rds_encoder.hpp rds_decoder.cpp rds_decoder.hpp README.md xkuzni04.pdf
	zip xkuzni04.zip $^

run:
	./rds_encoder -g 0A -pi 4660 -pty 5 -tp 1 -ms 0 -ta 1 -af 104.5,98.0 -ps "RadioXYZ"