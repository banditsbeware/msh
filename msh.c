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

#define WHITESPACE " \t\n"      // Delimiters for command tokenization

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10    // Mav shell only supports five arguments

#define MAX_HISTORY_LENGTH 5   // Maximum number of commands to store in history

/*
node: struct to hold history data in linked list
head will point to the oldest command, and each node will point to a more recent command.
some commands (cd, pidlist) will have pid = NULL, in which case they will be skipped by pidlist.
*/
typedef struct node
{
  pid_t pid;
  char cmd[MAX_COMMAND_SIZE];
  struct node *next;
} node;

// store a new command in the history list starting at head
//   if, after adding a new command, the length of history exceeds MAX_HISTORY_LENGTH,
//   the oldest history entry will be freed and head will point to the second-oldest.
void store_command(node *head, node *new)
{
    // length starts at 1 since there's no way to 
    // run this method without issuing a command
    int length = 1;
    node *iter = head;
    while (iter->next != NULL) 
    {
        iter = iter->next;
        length++;
    }
    iter->next = new;

    if (length > MAX_HISTORY_LENGTH)
    {
        node *new_head = head->next;
        free(head);
        head = new_head->next;
        printf("head advanced to %s", head->cmd);
    }
}

// output history to console
//   info = 1 will print all commands
//   info = 0 will print commands which spawned child processes
void print_history(node *head, int info) 
{
    int i=0;
    node *iter = head;
    while (iter->next != NULL)
    {
        // iterate over linked list
        iter = iter->next;

        // print command string
        if (info) printf("%d: %s", i++, iter->cmd);

        // print pid (if nonzero)
        if (!info && iter->pid) printf("%d: %d\n", i++, iter->pid);
    }
}

int main()
{

  // cmd_str accepts input and is overwritten each iteration of the while loop
  char *cmd_str = (char*) malloc(MAX_COMMAND_SIZE);

  // head will always point to the oldest command in history
  node *head = (node*) malloc(sizeof(node));

  while(1)
  {
    // Print out the msh prompt
    printf ("[msh] >> ");

    // Read the command from the commandline. The maximum command that will be read
    while(!fgets(cmd_str, MAX_COMMAND_SIZE, stdin));
    
    // ignore a blank command
    if (cmd_str[0] == '\n') continue;

    // initialize new command information
    node *new = (node*) malloc(sizeof(node));
    pid_t new_entry_pid = 0;

    // Parse input
    char *token[MAX_NUM_ARGUMENTS];
    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token parsed by strsep
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
        // no arguments given, go to home directory
        if (token[1] == NULL) chdir(getenv("HOME")); 

        // too many arguments
        else if (token[2] != NULL) printf("cd: too many arguments\n\n");

        // token[1] not NULL and token[2] NULL -> valid cd command
        else chdir(token[1]);
        
        // copy command string into new's memory (includes newline)
        strcpy(new->cmd, cmd_str);

        // new_entry_pid is zero for commands didn't spawn a child process
        new->pid = 0;

        // store_command will handle history size
        store_command(head, new);
    }

    // handle history
    else if (strcmp(token[0], "history") == 0)
    {
        // too many arguments
        if (token[1] != NULL) printf("history: too many arguments\n\n");
        else
        {
            // copy command string into new's memory (includes newline)
            strcpy(new->cmd, cmd_str);

            // new_entry_pid is zero for commands didn't spawn a child process
            new->pid = 0;

            // store_command will handle history size
            store_command(head, new);

            // passing 1 to print command strings
            print_history(head, 1);
        }    
    }

    // handle listpids
    else if (strcmp(token[0], "listpids") == 0)
    {
        // too many arguments
        if (token[1] != NULL) printf("listpids: too many arguments\n\n");
        else
        {
            // copy command string into new's memory (includes newline)
            strcpy(new->cmd, cmd_str);

            // new_entry_pid is zero for commands didn't spawn a child process
            new->pid = 0;

            // store_command will handle history size
            store_command(head, new);

            // passing 0 to print PIDs
            print_history(head, 0);
        }
    }

    // all other commands handled by fork, wait, exec
    else
    {
        pid_t pid;
        int status;

        pid = fork();

        // fork failed
        if (pid < 0) printf("fork failed\n");

        // child process    
        else if (pid == 0) 
        {
            // store pid for history entry
            new_entry_pid = getpid();
            
            // if execvp() returns a nonzero value, it failed.
            if (execvp(token[0], token))
            {
                // command not found
                printf("%s: Command not found.\n\n", token[0]);
                exit(1);
            }
        }

        // parent process
        else 
        {
            // save the pid of the spawned process for history record
            new_entry_pid = pid;

            // wait for child process to complete
            while (wait(&status) != pid);
        }

        // copy command string into new's memory (includes newline)
        strcpy(new->cmd, cmd_str);

        // new_entry_pid is zero for commands didn't spawn a child process
        new->pid = new_entry_pid;

        // store_command will handle history size
        store_command(head, new);
    }
  }
  return 0;
}
