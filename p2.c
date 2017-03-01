/*
	Program 4
	Nicholas Krisa, masc0069
	Professor Carroll, CS570
*/

	
#include "p2.h"

// Global Variables
char argv[MAXSTORAGE];
char *newargv[MAXITEM];
char *input, *output;
int argc;
int fildes[20];
int cmds[11];

// Flags & File Descriptors
int BG_FLAG = 0, INPUT_FLAG = 0, OUTPUT_FLAG = 0, PIPE_FLAG = 0, DS_FLAG;
int input_fd, output_fd, devnull_fd;		

int main ( ) {
	(void)signal(SIGTERM, signalHandler);

	for (;;) {
        // User Prompt
		(void)printf("p2: ");
		
		// Parse & EOF Check
		if (parse() == -1) break;
		
		// No input from stdin
		if (argc == 0) continue;
			
		if (newargv[0] == NULL) {
			(void)fprintf(stderr,"Invalid null command.\n");
			continue;
		}
		
		// CD Handler
		if (strcmp(newargv[0], "cd") == 0) {
			cdHandler();
			continue;
		}
		
		// Environ Handler
		if (strcmp(newargv[0], "environ") == 0) {
			envHandler();
			continue;
		}
			
		// Set Input FileDes
		if (INPUT_FLAG != 0) {
			if (INPUT_FLAG > 1) {
				(void)fprintf(stderr,"Ambiguous input redirect.\n");
				continue;
			}
			if (input == NULL) {
				(void)fprintf(stderr,"Missing name for redirect.\n");
				continue;
			}
			if ((input_fd = open(input, O_RDONLY)) < 0) {
				(void)fprintf(stderr, "Invalid input file.\n");
				continue;
			}

		}
		
		// Set Output FileDes
		if (OUTPUT_FLAG != 0) {
			int flags = O_CREAT | O_EXCL | O_RDWR;
			
			if (OUTPUT_FLAG > 1) {
				(void)fprintf(stderr,"Ambiguous output redirect.\n");
				continue;
			}
			if (output == NULL) {
				(void)fprintf(stderr,"Missing name for redirect.\n");
				continue;
			}
			if ((output_fd = open(output, flags, S_IRUSR | S_IWUSR)) < 0) {
				(void)fprintf(stderr,"Invalid output file.\n");
				continue;
			}

		}
        
        // Set /dev/null Filedes
        if(BG_FLAG != 0 && INPUT_FLAG == 0) {
            int flags = O_RDONLY;
            
            if ((devnull_fd = open("/dev/null",flags)) < 0){
                (void)fprintf(stderr, "Failed to open /dev/null.\n");
                exit(1);
            }
        }
		
		if (PIPE_FLAG == 0) {
			execHandler();
			continue;
		}
		
		if (PIPE_FLAG > 0 && PIPE_FLAG <= 10) {
			pipeHandler();
			continue;
		}	
	}
	
	(void)killpg( getpid(), SIGTERM );
	(void)printf("p2 terminated.\n");
	exit(0);
}

int parse( ) {	
	int wordLength = 0;
	char *p = argv;
	
	// Reset Global Variables
	input = NULL, output = NULL, newargv[0] = NULL;  
    argc = 0, BG_FLAG = 0, INPUT_FLAG = 0, OUTPUT_FLAG = 0, PIPE_FLAG = 0;
	
	for (;;) {
		// Get next word & save length
		wordLength = getword(p);
		
		// Newline or EOF is found
		if (wordLength == 0 || wordLength == -1)
			break;
		
		// Background Handler
		if (*p == '&' && wordLength == 1) {
			wordLength = getword(p);
			
			// Is last word
			if (wordLength == 0 || wordLength == -1) {
				BG_FLAG = 1;
				break;
			}
		}

		// Input/Output Redirection
		if (*p == '<' && wordLength == 1) {
			INPUT_FLAG++;
			wordLength = getword(p += 2);
			
			if (wordLength < -1) {
				if ((input = getenv(p+1)) == NULL) {
					(void)fprintf(stderr, "%s: Undefined variable.\n", p);
					argc = 0;
				}
			}
			else
				input = p;
		}

		else if (*p == '>' && wordLength == 1) {
			OUTPUT_FLAG++;
			wordLength = getword(p += 2);
			
			if (wordLength < -1) {
				if ((output = getenv(p+1)) == NULL) {
					(void)fprintf(stderr, "%s: Undefined variable.\n", p);
					argc = 0;
				}
			}
			else
				output = p;
		}
		
		else if (*p == '|' && wordLength == 1) {
			PIPE_FLAG++;
			newargv[argc++] = NULL;
            cmds[PIPE_FLAG] = argc;
		}
		
		else {
			if (wordLength < -1) {
				if ((newargv[argc++] = getenv(p+1)) == NULL) {
					(void)fprintf(stderr, "%s: Undefined variable.\n", p);
					argc = 0;
				}
			}
			else
				newargv[argc++] = p;
		}
		
		p += abs(wordLength)+1;
	}
	
	// Success
	newargv[argc] = NULL;
	return wordLength;

}



// Helper Methods
void signalHandler() {}

void cdHandler() {
	// Home Directory
	if (argc == 1) {
		if (chdir(getenv("HOME")) != 0)
			(void)fprintf(stderr, "Invalid home directory.\n");
	}
	
	// Given Directory
	else if (argc == 2) { 
		if (chdir(newargv[1]) != 0)
			(void)fprintf(stderr, "%s: No such file or directory. \n", newargv[1]);
	}
	
	// Error
	else 
		(void)fprintf(stderr, "Too many arguments.\n");
}

void envHandler() {
	char *env;
	
	// 1 Argument 
	if (argc == 2) {
		if ((env = getenv(newargv[1])) != 0)
			(void)printf("%s\n", env);
		else
			(void)printf("%s: Undefined variable.\n", newargv[1]);
	}
	
	// 2 Arguments
	else if (argc == 3) {
		if (setenv(newargv[1], newargv[2], 1) < 0 ) 
			(void)fprintf(stderr, "environ: Unable to set environment.\n");
	}
	
	// Error
	else
		(void)fprintf(stderr,"environ: Invalid input.\n");

}

void execHandler() {
	pid_t child_pid;
	
	// Flush
	(void)fflush(stderr);
	(void)fflush(stdout);
	
	
	// Fork Child
	if ((child_pid = fork()) < 0)
		fprintf(stderr, "Unsuccessful Fork.\n");
	
	// Child Code
	else if (child_pid == 0) {
		// Redirect Input to devnull
        if(BG_FLAG != 0 && INPUT_FLAG == 0) {
            CHK(dup2(devnull_fd,STDIN_FILENO));
            CHK(close(devnull_fd));
        }
		
		// Redirect Input to file
		if (INPUT_FLAG == 1) {
			CHK(dup2(input_fd, STDIN_FILENO));
			CHK(close(input_fd));
		}
				
		// Redirect Ouput to file		
		if (OUTPUT_FLAG == 1) {
			CHK(dup2(output_fd, STDOUT_FILENO));
			CHK(close(output_fd));
		}
				
		if (execvp(newargv[0], newargv) < 0) {
			(void)fprintf(stderr, "%s: Command not found.\n", newargv[0]);
			exit(1);
		}
	}
	
	// Parent Code	
	else {
        if (BG_FLAG != 0) {
            (void)printf("%s [%l]\n", newargv[0], child_pid);
            BG_FLAG = 0;
            //CHK(sleep(1));
        }
        
        else {
            for(;;) {
                pid_t pid;
                CHK(pid = wait(NULL));
                if (pid == child_pid)
                    break;
            }
        }
	}
}

void pipeHandler() {
	pid_t first, middle, last;
	
    int i;
    for (i = 0; i < PIPE_FLAG; i++) {
        CHK(pipe(fildes+(i*2)));
    }
	
    // First Command
    (void)fflush(stderr);
    (void)fflush(stdout);
    CHK(first = fork());
	
    if (first == 0) {
		CHK(dup2(fildes[1], STDOUT_FILENO));
		
		if (INPUT_FLAG == 1) {
			CHK(dup2(input_fd, STDIN_FILENO));
			CHK(close(input_fd));
		}
        
        if(BG_FLAG != 0 && INPUT_FLAG == 0) {
            CHK(dup2(devnull_fd,STDIN_FILENO));
            CHK(close(devnull_fd));
        }
		
        closePipes();
        
		if (execvp(newargv[0], newargv) < 0) {
			(void)fprintf(stderr, "%s: Command not found.\n", newargv[0]);
			exit(1);
		}
	}
	
	// Middle Command(s)
    int j;
    for (j = 0; j < PIPE_FLAG-1; j++) {
        (void)fflush(stderr);
        (void)fflush(stdout);
        CHK(middle = fork());
	
        if (middle == 0) {
            CHK(dup2(fildes[j*2], STDIN_FILENO));
            CHK(dup2(fildes[(j*2)+3], STDOUT_FILENO));
            
            closePipes();
		
            if (execvp(newargv[cmds[j+1]], newargv+cmds[j+1]) < 0) {
                (void)fprintf(stderr, "%s: Command not found.\n", newargv[cmds[j]]);
                exit(1);
            }
        }
    }
    
    // Last Command
    (void)fflush(stderr);
    (void)fflush(stdout);
    CHK(last = fork());
    
    if (last == 0) {
        CHK(dup2(fildes[(PIPE_FLAG-1)*2], STDIN_FILENO));
        
        if (OUTPUT_FLAG == 1) {
            CHK(dup2(output_fd, STDOUT_FILENO));
            CHK(close(output_fd));
        }
        
        closePipes();
        
        if (execvp(newargv[cmds[PIPE_FLAG]], newargv+cmds[PIPE_FLAG]) < 0) {
            (void)fprintf(stderr, "%s: Command not found.\n", newargv[cmds[PIPE_FLAG]]);
            exit(1);
        }
    }
    
	
	
    // Parent Code
	closePipes();

    if (BG_FLAG != 0) {
        (void)printf("%s [%l]\n", newargv[0], first);
        BG_FLAG = 0;
    }
    
    else {
        for(;;) {
            pid_t pid;
            CHK(pid = wait(NULL));
            if (pid == last)
                break;
        }
    }
}

void closePipes() {
    int i;
    for (i = 0; i < PIPE_FLAG*2; i++)
        CHK(close(fildes[i]));
}

