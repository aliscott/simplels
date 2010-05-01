/*
 * Implementation of utility functions for the ls program
 * author: Ali Scott
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/*
 * returns the input string in lower case
 */
char* strtolower(const char* s, char* t) {
	strncpy(t, s, strlen(t) + 1);
	int i = 0;
	while(t[i] != '\0') {
		t[i] = tolower(t[i]);
		i++;
	}
	return t;
}

/*
 * returns the string length of an integer
 */
int intlen(int i) {
	if (i < 0)
		return 1 + intlen(-i);
	else if (i == 0)
		return 1;
	else
		return floor(log10((double) i)) + 1;
}

/*
 * prints n spaces
 */
void printspaces(int n) {
	int i;
	for (i = 0; i < n; i++)
		printf(" ");
}


