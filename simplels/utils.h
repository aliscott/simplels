/*
 * Declarations of utility functions for the ls program
 * author: 060004694
 */

// defines boolean values
#define FALSE 0
#define TRUE (FALSE == FALSE)

/*
 * returns the input string in lower case
 */
char* strtolower(const char* s, char* t);

/*
 * returns the string length of an integer
 */
int intlen(int i);

/*
 * prints n spaces
 */
void printspaces(int n);
