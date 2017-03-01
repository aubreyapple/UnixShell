
/*  Program 4
 *	getword.c
 *
 *	Nicholas Krisa, masc0069
 *	Professor Carroll, CS570, Due 9/21/16 
 *
 * 	This program reads words from stdin and writes the chars
 *	to the character array passed as a parameter.  It expands
 *	on Program 0, adding more functionality.
 */


#include "getword.h"

#define STORAGE 255

int getword( char *w )
{
    int iochar;
	int counter = 0; 
	int backslash = 0; 
	int dollarsign = 0;
	
	
    	while ((iochar = getchar()) != EOF) {
		
		// Array is full
		if (counter == STORAGE-1) {
			(void) ungetc(iochar, stdin);
			w[counter] = '\0';
			return dollarsign == 0 ? counter : -counter;
		}

		// Backslash
		else if (iochar == '\\' && backslash == 0) {
			backslash = 1;
			continue;
		}
		
		// Blank Character
		else if (iochar == ' ') {
			// Leading Blank -> Do nothing
			if ( counter == 0 ) 
				continue;
			
			// After a backslash
			else if (backslash == 1)
				w[counter++] = iochar;

			else {
				w[counter] = '\0';
				return dollarsign == 0 ? counter : -counter;
			}

		}

		// Newline Character
		else if (iochar == '\n') {
			if (counter != 0) {
				w[counter] = '\0';
				(void) ungetc(iochar, stdin);
				return dollarsign == 0 ? counter : -counter;
			}	
			else {
				w[counter] = '\0';
				return dollarsign == 0 ? counter: -counter;
			}

		}

		// Metacharacters 
		else if (iochar == '<' || iochar == '>' || iochar == '|' || iochar == '&') {
			
			// Backslash preceeds metacharacter
			if (backslash == 1) {
				w[counter++] = iochar;

			}
			// Return existing word
			else if (counter != 0) {
				(void) ungetc(iochar, stdin);
				w[counter] = '\0';
				return dollarsign == 0 ? counter : -counter;
			}

			// Normal Case
			else {
				w[counter++] = iochar;
				w[counter] = '\0';
				return dollarsign == 0 ? counter : -counter; 
			}

		}  
		
		// Dollarsign
		else if (iochar == '$') {
			if (counter == 0 && backslash == 0)
				dollarsign = 1;
			w[counter++] = iochar;
		}		

		// Regular Characters
		else { 
			w[counter++] = iochar;	
		}
		
		// Resets Backslash 
		backslash = 0;
	}

	// EOF 
	w[counter] = '\0';

	if (counter != 0) {
		return dollarsign == 0 ? counter : -counter;
	}

	return -1;
}

