/*

david rademacher
1001469394

this program is a modified copy of CSE3320/Shell-Assignment/msh.c
for the "Shell" assignment

*/

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10    // Mav shell only supports five arguments

// struct to hold history data in linked list
typedef struct node
{
  pid_t pid;
  char * cmd[MAX_COMMAND_SIZE];
  struct node * prev_entry;
} node;

// add a new entry to history, return new length
int hist_add(node * head, int pid, char * cmd)
{
    
}

int main()
{

  char * cmd_str = (char*) malloc(MAX_COMMAND_SIZE);

  while(1)
  {
    // Print out the msh prompt
    printf ("[msh] >> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while(!fgets(cmd_str, MAX_COMMAND_SIZE, stdin));
    
    // ignore a blank command
    if (cmd_str[0] == '\n') continue;

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];
    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr;                                         
                                                           
    char *working_str  = strdup(cmd_str);                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input strings with whitespace used as the delimiter
    while (((argument_ptr = strsep(&working_str, WHITESPACE)) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup(argument_ptr, MAX_COMMAND_SIZE);
      if(strlen(token[token_count]) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }
    free(working_root);

    // handle exit, quit
    if (strcmp(token[0], "exit") == 0 
     || strcmp(token[0], "quit") == 0) exit(0);

    // handle cd
    if (strcmp(token[0], "cd") == 0)
    {
        if (token[1] == NULL)
        {
            // no arguments given, go to home directory
            chdir(getenv("HOME")); 
        }
        else if (token[2] != NULL)
        {
            // too many arguments
            printf("cd: too many arguments\n\n");
        }
        else
        {
            // token[1] not NULL and token[2] NULL -> valid cd command
            chdir(token[1]);
        }

        // return to loop in the new directory
        continue;
    }

    if (strcmp(token[0], "listpids") == 0)
    {
        if (token[1] != NULL)
        {
            // too many arguments
            printf("listpids: too many arguments\n\n");
        }

        // return to loop
        continue;
    }

    // Now print the tokenized input as a debug check
    int token_index  = 0;
    for(token_index = 0; token_index < token_count; token_index++) 
    {
      // printf("token[%d] = %s\n", token_index, token[token_index] );  
    }

    pid_t pid;
    int status;

    pid = fork();
    // fork failed
    if (pid < 0) 
    {
        printf("fork failed\n");
    } 
    // child process
    else if (pid == 0) 
    {
        // save the status of the issued command in case it fails
        int command_status;
        if ((command_status = execvp(token[0], token)) != 0)
        {
            // command not found
            printf("%s: Command not found.\n\n", token[0]);
            exit(1);
        }
        // command succeeded, return to parent
    }
    // parent process
    else 
    {
        // store the command and its pid in history
        // hist_push
        // wait for child process to complete
        while (wait(&status) != pid);
    }
  }
  return 0;
}
