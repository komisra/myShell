#include <stdlib.h>
#include <stdio.h> 
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_TOKENS 8

#define PATH_MAX 1024

#define MAX_ALIAS 10
/* Every simple command has one of these associated with it */
struct simple {
  char* token[MAX_TOKENS]; /* tokens of the command */
  int count; /* the number of tokens */
}cmd_inst;

struct simple *parseSequence();

int main(int argc, char *argv[]) {
  struct simple *command;
  struct simple *aliasTable[MAX_ALIAS];
  int aliasCount = 0;
  
  /* insert your code here */
  int seenExit = 0; /* flag to determine when to exit program */
  while (seenExit == 0) {
    printf("> ");
    fflush(stdout);

    // Read user input
    char line[PATH_MAX];
    fgets(line, PATH_MAX, stdin);

    // Parse user input and store in pointer simple struct

    command = malloc(sizeof(struct simple));
    command = parseSequence(line);

    // exit command
    if(strcmp("exit", command->token[0]) == 0) {
      seenExit = -1;
      exit(EXIT_SUCCESS);
    // cd command
    } else if(strcmp("cd", command->token[0]) == 0) {
      // expects 1 argument + NULL TERMINATOR
      if(command->count == 2) {
        chdir("..");
      } else if(command->count == 3) {
        int status = chdir(command->token[1]);
        if(status == -1) {
          fprintf(stdout, "Invalid command\n");
        }
      } else {
        fprintf(stdout, "Invalid command\n");
      }
    // pwd command
    } else if(strcmp("pwd", command->token[0]) == 0) {
      char cwd[PATH_MAX];
      if(getcwd(cwd, sizeof(cwd)) == NULL) {
        fprintf(stderr, "Error printing directory\n");
      } else {
        printf("%s\n", cwd);
      }
    // alias command
    } else if(strcmp("alias", command->token[0]) == 0) {
      if(command->count == 2) {
        // print out alias table
        for(int i = 0; i < aliasCount; i++) {
          for(int j = 0; j < aliasTable[i]->count - 1; j++) {
            // Print formatted string of alias commands
            printf("%5s", aliasTable[i]->token[j]);
          }
          printf("\n");
        }

      } else if(command->count == 3) {
        // search alias table for command[1]
        // and print out matching alias
        for(int i = 0; i < aliasCount; i++) {
          if(strcmp(aliasTable[i]->token[0], command->token[1]) == 0) {
            // Alias found!
            for(int j = 1; j < aliasTable[i]->count - 1; j++) {
              // Print formatted string of matching alias command
              printf("%s ", aliasTable[i]->token[j]);
            }
            printf("\n");
          }
        }

      } else {
        // TODO: 
        // create alias for command
        
        aliasTable[aliasCount] = (struct simple *) malloc(sizeof(struct simple));
        
        aliasTable[aliasCount]->count = command->count - 1;

        for(int i = 1; i < aliasTable[aliasCount]->count; i++) {
          aliasTable[aliasCount]->token[i-1] = malloc( sizeof(command->token[i]));
          strcpy(aliasTable[aliasCount]->token[i-1], command->token[i]);
        }
        aliasCount++;
      }
    } else {
      // TODO:
      // Check alias commands, and run execvp on matching aliases
      for(int i = 0; i < aliasCount; i++) {
        // compare first token from command with first token in alias table

        if(strcmp(command->token[0], aliasTable[i]->token[0]) == 0) {
          // Alias found!
          // Take slice of token array from aliasTable[i].token[1] to aliasTable[i].token[aliasTable[i].count]
          //char *slice = *(aliasTable[i].token + 1);

          pid_t pid = fork();
          if(pid == -1) {
            fprintf(stderr, "Can't create child process");
            exit(EXIT_FAILURE);
          }

          execvp(aliasTable[i]->token[1], aliasTable[i]->token);


          // wait for child process to terminate
          wait(NULL);

          // Leave the for loop
          break;
        }
      }

      // Run execvp on othercommands
      pid_t pid = fork();
      if(pid == -1) {
        fprintf(stderr, "Can't create child process");
        exit(EXIT_FAILURE);
      }

      if(pid == 0) {
        execvp(command->token[0], command->token);
      }

      // wiat for child process to terminate
      wait(NULL);
    }

    /**
    * After reading user input, the steps are:
    * (1) fork a child process using fork()
    * (2) the child process will invoke execvp()
    * (3) parent will invoke wait() unless command included &
    */
  }

  return 0; /* exit success */
}

/*
"parseSequence" function is used to parse the char line got from the
standard input into the simple structure to pass arguments into system
calls later.
*/
struct simple *parseSequence(char * line) {
  int i, t;
  struct simple *c = &cmd_inst;
  memset(c, 0, sizeof(struct simple));

  t = 0;

  i = 0;
  while (isspace(line[i]))
    i++;
  c->token[t] = &line[i];

  while (line[i] != '\0' && t < MAX_TOKENS - 1) {
    t++;

    while (!isspace(line[i]) && line[i] != '\0')
      i++;

    while (isspace(line[i])) {
      line[i] = '\0';
      i++;
    }

    c->token[t] = &line[i];
  }
  c->count = t + 1;
  c->token[t] = NULL;

  return c;
}