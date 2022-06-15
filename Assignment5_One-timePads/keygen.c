// Assignment 5: One-time Pads
// Author: Andrea Tongsak
// Program: keygen.c
// Purpose: Create a key file of specified length

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// keygen function: creates key file of specified length
int main (int argc, char * argv[]) {
	srand(time(NULL));
	int randNum;
	int keyInput = atoi(argv[1]);

	// generate a key that is mixing up the letters
	for (int i = 0; i < keyInput; i++) {
		randNum = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[rand()%27];
        if (randNum == 'A' + 26) {
            randNum = ' ';
        }
        fprintf(stdout, "%c", randNum);
	}
    fprintf(stdout, "\n");
    return 0;
}
