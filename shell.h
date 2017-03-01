#include <stdio.h>
#include "getword.h"
#include <unistd.h>			// chdir(), execvp()
#include <stdlib.h>			// exit()
#include <signal.h>			// killpg()
#include <sys/types.h>			// open()
#include <sys/stat.h>			// open()
#include <fcntl.h>			// open()
#include <sys/wait.h>
#include <errno.h>
#include "CHK.h"
#include <strings.h>
#include <dirent.h>

#define MAXITEM 100
#define MAXSTORAGE 25500

/* parse()
 *  This function calls getword() to read input from STDIN.  It passes getword() a char pointer that is initially set to argv.
 *  parse then analyses the chars that are written to that address and sets up a new array of char* to the addresses in argv.
 *  parse also handles metacharacters, setting the appropriate flags for each when encountered.
 *  return: -1 is return if parse discovers an EOF.
*/
int parse();

/* signalHandler()
 * Is an empty function for signal().
 */
void signalHandler();

/* cdHandler()
 *  Is called when "cd" is entered by the user.
 *  Accepts 0 or 1 argument.
 *  If no arguments are given, chdir is called to the HOME directory.
 *  If 1 argument is given, chdir is called to that directory. (If it exists)
 */
void cdHandler();

/* envHandler()
 *  Is called when "environ" is enter by the user.
 *  Accepts 1 or 2 arguments.
 *  If 1 argument is given, the function prints the given environment.
 *  If 2 arguments are given, the function sets the 1st environment to the 2nd.
 */
void envHandler();

/* execHandler()
 *  This code is mostly pulled from the course reader.
 *  The function is called when no Pipes are entered by the user.
 *  First, a child is forked.  Then input/output redirection is handled if necessary. 
 *  Finally, the child is exec to the given command.
 */
void execHandler();

/* pipeHandler()
 *  This function is called when the user enters 1-10 pipes.
 *  The first and last code portions are mostly reused code from p2.
 *  The middle commands iteratively redirect their STDIN & STDOUT to the previous and next command before execing.
 */
void pipeHandler();

/* closePipes()
 *  Interates through and closes all open pipes.
 */
void closePipes();
