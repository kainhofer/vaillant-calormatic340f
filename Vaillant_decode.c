#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

// Vaillant_decode.c Copyright 2017 Reinhold Kainhofer
// License: GNU GPL 3.0
// 
//
// Compile with:
//     gcc Vaillant_decode.c -o Vaillant_decode
// 
// Reads a string of 0 and 1 from STDIN, tries to decode it as Vaillant-encoded
// and prints the converted result to STDOUT (prefixed with current date/time).
// Decoding errors (i.e. no amplitude switch at the pulse mid-point) are
// indicated by a ?
//
// This app is purely pipe-through, i.e. its usage is:
//
//     cat signalfile.txt | Vaillant_decode > decoded_signalfile.txt
//
// To process an output file while it is still written to, you can use tail. This 
// will print the proper timestamps for the signals:
//
//     tail -n +1 -f signalfile.txt | ./Vaillant_decode

typedef int bool;
#define true 1
#define false 0

void main(int argc, char *argv[])
{
	setbuf(stdout, NULL);

	bool insideZeroes = true;
	int zeroes = 0;
	
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
			printf((ch == prev)?"0":"1");
		} else { // validPos == 0
			// If 0/1 does not change => INVALID, unless it stays 0, which indicates end of signal
			if (ch == prev) {
				if (ch == '0') {
					insideZeroes = true;
					zeroes = true;
					printf("\n");
				} else {
					printf("?");
				}
			}
		}
		// Switch validPos:
		validPos = (validPos + 1) % 2;
		prev = ch;
	}
	printf("\r\n");
}
