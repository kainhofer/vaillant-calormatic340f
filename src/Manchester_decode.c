#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

// Manchester_decode.c Copyright 2017 Reinhold Kainhofer, reinhold@kainhofer.com
// License: GNU GPL 3.0
//
// Compile with:
//     gcc Manchester_decode.c -o Manchester_decode
// 
// Reads a string of 0 and 1 from STDIN, tries to decode it as Manchester-encoded
// and prints the converted result to STDOUT (prefixed with current date/time).
// Decoding errors (i.e. no amplitude switch at the pulse mid-point) are
// indicated by a ?
//
// This app is purely pipe-through, i.e. its usage is:
//
//     cat signalfile.txt | Manchester_decode > decoded_signalfile.txt
//
// To process an output file while it is still written to, you can use tail. This 
// will print the proper timestamps for the signals:
//
//     tail -n +1 -f signalfile.txt | ./Manchester_decode


typedef int bool;
#define true 1
#define false 0

void main(int argc, char *argv[])
{
	setbuf(stdout, NULL);

	bool insideZeroes = true;
	int zeroes = 0;
	int pulsePos = 0;
	int comment = 0;
	
	char ch = '0', prev = '0';
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
				pulsePos = 0;
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

		if (pulsePos == 0) { // middle of the pulse => needs to switch amplitude!
			
			if (ch == prev) {
				if (ch=='0' && (zeroes > 2)) {
					// End of Signal, receiving just zeroes, i.e. waiting for next signal
					insideZeroes = true;
					printf("\r\n");
				} else {
					printf("?");
				}
				// Keep the pulse position (TODO: Check if it is better for error recovery to switch!)
				continue;
				
			} else { // Amplitude switch was detected
				
				// Switch 0->1 means 0, while 1->0 means 1
				if (prev=='0' && ch=='1') {
					printf("0");
				} else if (prev=='1' && ch=='0') {
					printf("1");
				} else {
					// TODO: Maybe we should not switch pulsePos in this case?
					printf("(%c)", ch);
				}
			}
		} else {
			// Between pulses, amplitude can change or not (does not matter)
		}

		// Switch pulsePos:
		pulsePos = (pulsePos + 1) % 2;
		prev = ch;
	}
	printf("\r\n");
}

