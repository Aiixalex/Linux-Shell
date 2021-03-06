// Shell starter file
// You may make any changes to any part of this file.

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <errno.h>

#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)
#define PATH_MAX 100
#define HISTORY_DEPTH 10

char history[HISTORY_DEPTH][COMMAND_LENGTH];

int command_num = -1;

/**
 * Command Input and Processing
 */

/*
 * Tokenize the string in 'buff' into 'tokens'.
 * buff: Character array containing string to tokenize.
 *       Will be modified: all whitespace replaced with '\0'
 * tokens: array of pointers of size at least COMMAND_LENGTH/2 + 1.
 *       Will be modified so tokens[i] points to the i'th token
 *       in the string buff. All returned tokens will be non-empty.
 *       NOTE: pointers in tokens[] will all point into buff!
 *       Ends with a null pointer.
 * returns: number of tokens.
 */
int tokenize_command(char *buff, char *tokens[])
{
	int token_count = 0;
	_Bool in_token = false;
	int num_chars = strnlen(buff, COMMAND_LENGTH);
	for (int i = 0; i < num_chars; i++) {
		switch (buff[i]) {
		// Handle token delimiters (ends):
		case ' ':
		case '\t':
		case '\n':
			buff[i] = '\0';
			in_token = false;
			break;

		// Handle other characters (may be start)
		default:
			if (!in_token) {
				tokens[token_count] = &buff[i];
				token_count++;
				in_token = true;
			}
		}
	}
	tokens[token_count] = NULL;
	return token_count;
}

/**
 * Read a command from the keyboard into the buffer 'buff' and tokenize it
 * such that 'tokens[i]' points into 'buff' to the i'th token in the command.
 * buff: Buffer allocated by the calling code. Must be at least
 *       COMMAND_LENGTH bytes long.
 * tokens[]: Array of character pointers which point into 'buff'. Must be at
 *       least NUM_TOKENS long. Will strip out up to one final '&' token.
 *       tokens will be NULL terminated (a NULL pointer indicates end of tokens).
 * in_background: pointer to a boolean variable. Set to true if user entered
 *       an & as their last token; otherwise set to false.
 */
void read_command(char *buff, char *tokens[], _Bool *in_background)
{
	*in_background = false;

	// Read input
	int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);

	if ((length < 0) && (errno != EINTR)) {
		perror("Unable to read command from keyboard. Terminating.\n");
		exit(-1);
	}

	if ((length < 0) && (errno == EINTR)) {
		tokens[0] = NULL;
		return;
	}

	// Null terminate and strip \n.
	buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n') {
		buff[strlen(buff) - 1] = '\0';
	}

	// Tokenize (saving original command string)
	int token_count = tokenize_command(buff, tokens);
	if (token_count == 0) {
		return;
	}

	// Extract if running in background:
	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
		*in_background = true;
		tokens[token_count - 1] = 0;
	}
}

void pwd() {
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		write(STDOUT_FILENO, cwd, strlen(cwd));
		write(STDOUT_FILENO, "\n", strlen("\n"));
	}
}

void cd(char *path, char *prev, char *recent) {
	char home_dir[PATH_MAX] = "/home/";
	char *user_name = getpwuid(getuid())->pw_name;
	strcat(home_dir,user_name);

	if (path == NULL) {
		if (chdir(home_dir) != 0) {
			perror(home_dir);
		} else {
			strcpy(prev, recent);
			strcpy(recent, home_dir);
		}
	} else if(path[0] == '~'){
		strcat(home_dir, path+1);
		if (chdir(home_dir) != 0) {
			perror(home_dir);
		} else {
			strcpy(prev, recent);
			strcpy(recent, home_dir);
		}
	} else if(path[0] == '-') {
		if (chdir(prev) != 0) {
			perror(prev);
		} else {
			char temp[PATH_MAX];
			strcpy(temp, prev);
			strcpy(prev, recent);
			strcpy(recent, temp);
		}
	} else {
		if (chdir(path) != 0) {
			perror(path);
		} else {
			char abs_path[PATH_MAX];
			strcpy(prev, recent);
			strcpy(recent, getcwd(abs_path, sizeof(abs_path)));
		}
	}
}

void help(char *arg) {
	if (arg == NULL) {
		char help[1000] =  "A simple Linux Shell, version 1.0.1-release\nThese shell commands are defined internally.\ncd [dir]         Change the current working directory.\nexit             Exit the shell program.\nhelp [command]   Display help information on internal commands.\npwd              Display the current working directory.\n";
		write(STDOUT_FILENO, help, strlen(help));
	} else {
		if (strcmp(arg, "cd") == 0) {
			write(STDOUT_FILENO, "'cd' is a builtin command for changing the current working directory.\n", 
					strlen("'cd' is a builtin command for changing the current working directory.\n"));
		} else if (strcmp(arg, "exit") == 0) {
			write(STDOUT_FILENO, "'exit' is a builtin command for exiting the shell program.\n", 
					strlen("'exit' is a builtin command for exiting the shell program.\n"));
		} else if (strcmp(arg, "help") == 0) {
			write(STDOUT_FILENO, "'help' is a builtin command for displaying help information on internal commands.\n", 
					strlen("'help' is a builtin command for displaying help information on internal commands.\n"));
		} else if (strcmp(arg, "pwd") == 0) {
			write(STDOUT_FILENO, "'pwd' is a builtin command for displaying the current working directory.\n", 
					strlen("'pwd' is a builtin command for displaying the current working directory.\n"));
		} else {
			char buf[50];
			snprintf(buf, sizeof buf, "%s%s%s%s", "'", arg, "'", " is an external command or application.\n");
			write(STDOUT_FILENO, buf, strlen(buf));
		}
	}
}

void write_history(int num, char *tokens[], bool in_background) {
	char command[COMMAND_LENGTH] = "";
	strcat(command, tokens[0]);
	int i=1;
	while (tokens[i] != NULL) {
		strcat(command, " ");
		strcat(command, tokens[i]);
		i++;
	}
	if (in_background) {
		strcat(command, " &");
	}
	if (command_num > 9) {
		for (int j=0; j<9; j++) {
			strcpy(history[j],history[j+1]);
		}
		strcpy(history[9], command);
	} else {
		strcpy(history[num], command);
	}
}

void print_history() {
	if (command_num >= 9) {
		for (int i=0; i<=9; i++) {
			char num[20];
			sprintf(num, "%-10d", command_num - i);
			write(STDOUT_FILENO, num, strlen(num));
			write(STDOUT_FILENO, history[9-i], strlen(history[9-i]));
			write(STDOUT_FILENO, "\n", strlen("\n"));
		}
	} else {
		int temp = command_num;
		while (temp >= 0) {
			char num[20];
			sprintf(num, "%-10d", temp);
			write(STDOUT_FILENO, num, strlen(num));
			write(STDOUT_FILENO, history[temp], strlen(history[temp]));
			write(STDOUT_FILENO, "\n", strlen("\n"));
			temp--;
		}
	}
}

void handle_SIGINT() {
	write(STDOUT_FILENO, "\n", strlen("\n"));
	help(NULL);
}

/**
 * Main and Execute Commands
 */
int main(int argc, char* argv[])
{
	/* set up the signal handler */
	struct sigaction handler;
	handler.sa_handler = handle_SIGINT;
	handler.sa_flags = 0;
	sigemptyset(&handler.sa_mask);
	sigaction(SIGINT, &handler, NULL);
	
	char input_buffer[COMMAND_LENGTH];
	char *tokens[NUM_TOKENS];
    char cwd[PATH_MAX];

	char prev[PATH_MAX];
	char recent[PATH_MAX];
	getcwd(prev, sizeof(prev));
	getcwd(recent, sizeof(recent));

	while (true) {
		

		// Get command
		// Use write because we need to use read() to work with
		// signals, and read() is incompatible with printf().
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            strcat(cwd, "$ ");
            write(STDOUT_FILENO, cwd, strlen(cwd));
        } else {
            write(STDOUT_FILENO, "$ ", strlen("$ "));
        }
		
		_Bool in_background = false;
		read_command(input_buffer, tokens, &in_background);

		// DEBUG: Dump out arguments:
		for (int i = 0; tokens[i] != NULL; i++) {
			write(STDOUT_FILENO, "   Token: ", strlen("   Token: "));
			write(STDOUT_FILENO, tokens[i], strlen(tokens[i]));
			write(STDOUT_FILENO, "\n", strlen("\n"));
		}
		if (in_background) {
			write(STDOUT_FILENO, "Run in background.\n", strlen("Run in background.\n"));
		}

		if (tokens[0] != NULL) {
			if (tokens[0][0] == '!') {
				char command[COMMAND_LENGTH] = "";
				int run_num = 0;
				if (tokens[1] != NULL || tokens[0][1] == '\0') {
					perror("Invalid argument");
					continue;
				} else if (tokens[0][1] == '!') {
					if(command_num >= 0) {
						run_num = command_num;
					} else {
						perror("Invalid argument");
						continue;
					}
				} else {
					bool invalid_command = 0;
					for (int i=1; tokens[0][i] != '\0'; i++) {
						if (tokens[0][i] <= '9' && tokens[0][i] >= '0') {
							run_num = run_num * 10 + tokens[0][i] - '0';
						} else {
							perror("Invalid argument");
							invalid_command = 1;
							break;
						}
					}
					if (invalid_command) { continue; }
				}

				if (run_num > command_num || run_num < command_num - 9) {
					perror("Invalid argument");
					continue;
				} else {
					int token_count;
					if (command_num > 9) {
						strcpy(command, history[9 - (command_num - run_num)%10]);
					} else {
						strcpy(command, history[run_num]);
					}

					write(STDOUT_FILENO, command, strlen(command));
					write(STDOUT_FILENO, "\n", strlen("\n"));

					token_count = tokenize_command(command, tokens);
					
					if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
						in_background = true;
						tokens[token_count - 1] = 0;
					}
				}
			}

			write_history(++command_num, tokens, in_background);

			if (strcmp(tokens[0], "pwd") == 0) {
				if (tokens[1] != NULL) {
					perror("Invalid argument");
				} else {
					pwd();
				}
			} else if (strcmp(tokens[0], "cd") == 0) {
				if(tokens[2] != NULL) {
					perror("Invalid argument");
				} else {
					cd(tokens[1], prev, recent);
				}
			} else if (strcmp(tokens[0], "exit") == 0) {
				if (tokens[1] != NULL) {
					perror("Invalid argument");
				} else {
					return 0;
				}
			} else if (strcmp(tokens[0], "help") == 0) {
				if(tokens[2] != NULL) {
					perror("Invalid argument");
				} else {
					help(tokens[1]);
				}
			} else if (strcmp(tokens[0], "history") == 0) {
				print_history();
			} else {
				/**
				 * Steps For Basic Shell:
				 * 1. Fork a child process
				 * 2. Child process invokes execvp() using results in token array.
				 * 3. If in_background is false, parent waits for
				 *    child to finish. Otherwise, parent loops back to
				 *    read_command() again immediately.
				 */

				pid_t pid;
				pid = fork();
				if (pid == -1) {
					perror("Fork failed");
				} else if (pid == 0) {
					if (execvp(tokens[0], tokens) == -1) {
						perror("Execvp failed");
					}
				} else {
					if (in_background == false) {
						waitpid(pid, NULL, 0);
					}
				}

				while (waitpid(-1, NULL, WNOHANG) > 0); // no nothing.
			}
		}
	}
	return 0;
}
