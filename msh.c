/*

name: david rademacher
ID:   1001469394

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

#define WHITESPACE " \t\n"      // delimiters for command tokenization

#define MAX_COMMAND_SIZE 255    // the maximum command size

#define MAX_NUM_ARGUMENTS 10    // mav shell only supports five arguments

#define MAX_HISTORY_LENGTH 15   // maximum number of commands to store in history

// node: struct to hold history data in linked list
// - head will point to the oldest command, and each node 
//   will point to a more recent command.
// - some commands (cd, history, pidlist) will have pid = 0, in which case
//   they will be skipped by pidlist.
typedef struct node {
  pid_t pid;
  char cmd[MAX_COMMAND_SIZE];
  struct node *next;
} node;

// store a new command in the history list starting at head
// - if, after adding a new command, the length of history is MAX_HISTORY_LENGTH,
//   the oldest history entry will be freed and head will point to the second-oldest.
// - head is passed in as a double pointer so that the function can modify it.
void store_command(node **head, char *cmd, int pid) {

    // set up the new node with its own memory location
    node *new = (node*) malloc(sizeof(node));
    strcpy(new->cmd, cmd);  // copy command string into node memory
    new->pid = pid;         // copy pid
    new->next = NULL;       // set pointer to NULL to clear junk
    
    // if head is NULL, then history is empty
    // point head to the new entry
    if (*head == NULL) *head = new;

    // otherwise, add the new element to the end
    else { 
        // length starts at 1 because the 0 case is handled above
        int length = 1;
        node *iter = *head;
        
        // traverse the list & count entries
        while (iter->next != NULL) {
            iter = iter->next;
            length++;
        }

        // add the new entry to end of the list
        iter->next = new;

        if (length == MAX_HISTORY_LENGTH) {
            node *new_head = (*head)->next; // store second-oldest
            free(*head);                    // delete oldest command
            *head = new_head;               // reassign head
        }
    }
}

// output history to console
// - info = 1 will print all commands
// - info = 0 will print PIDs of commands which spawned child processes
void print_history(node *head, int info) {
    if (head == NULL) return; // do nothing if history is empty
    int i = 0;
    node *iter = head;
    while (iter != NULL) {
        // print command string
        if (info) printf("%d: %s", i++, iter->cmd);

        // print pid (if nonzero)
        if (!info && iter->pid) printf("%d: %d\n", i++, iter->pid);

        // iterate over linked list
        iter = iter->next;
    }
}

// retrieve a command from history
// - n corresponds to the printed number next to the desired command
//   as printed by print_history()
// - returns a pointer to the desired command string
char *retrieve_command(node *head, int n) {

    if (head == NULL) return NULL; // do nothing if history is empty
    
    int index = 0;
    node *iter = head;

    // command string will be copied into its own memory and should be freed after use
    char *cmd = (char*) malloc(MAX_COMMAND_SIZE);

    // iterate through history, stopping when the desired command is reached
    while (iter != NULL) {
        if (index++ == n) {
            strcpy(cmd, iter->cmd);
            return cmd;
        }
        iter = iter->next;
    }
    // if the loop makes it through the list, n was not found
    return NULL;
}

int main() {

  // cmd_str accepts input and is overwritten each iteration of the while loop
  char *cmd_str = (char*) malloc(MAX_COMMAND_SIZE);

  // head will always point to the oldest command in history
  node *head = NULL;

  while(1) {

    // Print out the msh prompt
    printf ("[msh] >> ");

    // Read the command from the commandline. The maximum command that will be read
    while(!fgets(cmd_str, MAX_COMMAND_SIZE, stdin));
    
    // ignore a blank command
    if (cmd_str[0] == '\n') continue;
    
    // look up command from history
    if (cmd_str[0] == '!') {

        // convert string to integer (unsafely?)
        int n = atoi(&cmd_str[1]);

        // store the pointer returned so it can be freed
        char *cmd = retrieve_command(head, n);

        // NULL return value means command was not found
        if (cmd == NULL) {
            printf("Command not in history.\n\n");
            continue;
        }

        // place retrieved command into cmd_str and continue to handling
        strcpy(cmd_str, cmd);

        // free the string pointed to by cmd
        free(cmd);
    }

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
              (token_count<MAX_NUM_ARGUMENTS)) {

        token[token_count] = strndup(argument_ptr, MAX_COMMAND_SIZE);

        if(strlen(token[token_count]) == 0 ) token[token_count] = NULL;

        token_count++;
    }

    free(working_root);

    // handle exit, quit
    if (strcmp(token[0], "exit") == 0 
     || strcmp(token[0], "quit") == 0) exit(0);

    // handle cd
    if (strcmp(token[0], "cd") == 0) {

        store_command(&head, cmd_str, 0);

        // no arguments given, go to home directory
        if (token[1] == NULL) chdir(getenv("HOME")); 

        // too many arguments
        else if (token[2] != NULL) printf("cd: too many arguments\n\n");

        // token[1] not NULL and token[2] NULL -> valid cd command
        else chdir(token[1]);
    }

    // handle history
    else if (strcmp(token[0], "history") == 0) {

        store_command(&head, cmd_str, 0);

        // too many arguments
        if (token[1] != NULL) printf("history: too many arguments\n\n");

        else print_history(head, 1); // passing 1 to print command strings
    }

    // handle listpids
    else if (strcmp(token[0], "listpids") == 0) {

        store_command(&head, cmd_str, 0);

        // too many arguments
        if (token[1] != NULL) printf("listpids: too many arguments\n\n");

        else print_history(head, 0); // passing 0 to print PIDs
    }

    // all other commands handled by fork, wait, exec
    else {

        pid_t pid;
        int status;

        pid = fork();

        // fork failed
        if (pid < 0) printf("fork failed\n");

        // child process    
        else if (pid == 0) {

            // if execvp() returns a nonzero value, it failed
            if (execvp(token[0], token)) {
                printf("%s: Command not found.\n\n", token[0]);
                exit(1);
            }
        }

        // parent process
        else {

            // in the parent process, pid stores the PID of the child spawned
            store_command(&head, cmd_str, pid);

            while (wait(&status) != pid); // wait for child process to complete
        }
    }
  }
  return 0;
}
