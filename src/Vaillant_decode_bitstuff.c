#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

// Vaillant_decode_bitstuff.c Copyright 2017 Reinhold Kainhofer
// License: GNU GPL 3.0
// 
//
// Compile with:
//     gcc Vaillant_decode_bitstuff.c -o Vaillant_decode_bitstuff
// 
// Reads a string of 0 and 1 from STDIN, tries to decode it as Vaillant-encoded
// applies bit-unstaffing (i.e. if 5 consecutive one bits are followed by a 
// zero bit, the zero bit is discarded and prints the converted result to 
// STDOUT (prefixed with current date/time).
// Decoding errors (i.e. no amplitude switch at the pulse mid-point) are
// indicated by a ?
//
// This app is purely pipe-through, i.e. its usage is:
//
//     cat signalfile.txt | Vaillant_decode_bitstuff > decoded_signalfile.txt
//
// To process an output file while it is still written to, you can use tail. This 
// will print the proper timestamps for the signals:
//
//     tail -n +1 -f signalfile.txt | ./Vaillant_decode_bitstuff

typedef int bool;
#define true 1
#define false 0

void main(int argc, char *argv[])
{
	setbuf(stdout, NULL);

	bool insideZeroes = true;
	int zeroes = 0;
	
	int ones = 0; // Count the number of consecutive ones in the output (for bit-unstuffing)
	int pos = 0;  // Current output count in octet (to space bytes)
	
	int validPos = 0;
	int comment = 0;

	char ch, prev;
	while(read(STDIN_FILENO, &ch, 1) > 0) {
		// Everything starting with # is a comment until the end of line
		if (ch == '#') {
			comment = true;
			if (!insideZeroes) printf("\n");
			insideZeroes = 1;
			continue;
		}
		// Ignore all line breaks and spaces (so output can be formatted before running through this app):
		if (ch == '\n' || ch == '\r') {
			comment = false;
			continue;
		}
		// Ignore all spaces (so output can be formatted before running through this app) and comments:
		if (comment || ch == ' ') {
			continue;
		}

		// Keep track of how many zeroes we have (count at most up to 1000!)
		if (ch == '0') {
			if (zeroes++ > 1000) zeroes = 1000;
		} else {
			zeroes = 0;
		}
		// We have zeroes, so jump to the next char if we have another 0, reset otherwise
		if (insideZeroes) {
			if (ch == '0') {
				prev = ch;
				continue;
			} else {
				insideZeroes = false;
				validPos = 0;
				// A '1' after a long sequence of '0' means some kind of signal
				// (valid or invalid), so print the current date/time to start
				// a new line:
				struct timeval  tv;
				gettimeofday(&tv, NULL);
				int millisec = tv.tv_usec/1000;

				char fmt[64], s[64];
				struct tm       *tm;
				if((tm = localtime(&tv.tv_sec)) != NULL) {
					strftime(fmt, sizeof fmt, "%F %T", tm);
					snprintf(s, sizeof s, "%s.%03d", fmt, millisec);
					printf("%s  ", s); 
				}
			}
		}
		if (ch != '0' && ch != '1') {
			fwrite(&ch, 1, 1, stdout);
		}
		
		// Inside the pulse => if state changed, it's a 1, if it stayed, it's a 0
		if (validPos == 1) {
			// bit-unstuffing: Ignore a 0 after exactly five consecutive 1 bits:
			if (ch != prev) { // a transition indicates a one
				ones++;
				printf("1");
				pos++;
			} else { // no transition indicates a zero
				if (ones!=5) {
					printf("0");
					pos++;
				}
				ones = 0;
			}
		} else { // validPos == 0
			// If 0/1 does not change => INVALID, unless it stays 0, which indicates end of signal
			if (ch == prev) {
				if (ch == '0') {
					insideZeroes = true;
					zeroes = true;
					printf("\n");
					pos = 0;
				} else {
					printf("?");
					pos++;
				}
			}
		}
		if (pos == 8) { // A full byte was printed, so add a space
			pos = 0;
			printf(" ");
		}
		// Switch validPos:
		validPos = (validPos + 1) % 2;
		prev = ch;
	}
	printf("\r\n");
}
