/* Author: D Yoan L Mekontchou Yomba
 *
 * Class: ECEC 353 - Systems Programming
 *
 * Project - Create a Shell
 *
 *  Date: 1/27/2018
 *
 *
 */


/*
 *
 *========================================ALGORITHM=========================================
 * 1. if the command inputed by the user is the only command, process the command regularly
 * 2. if the command inputed by the user is the first command and will be followed by other commands
 *    file descriptor table should look like the following 
 *    
 *    FD | 
 *    _______
 *    0  | STDIN
 *    1  | WRITE SIDE OF PIPE
 *    2  | STDERR
 * 3. if the command inputed by the user is not the first or last command currently present
 *    the file descriptor table should look like the following
 *     
 *    FD |
 *    _______
 *    0  | READ SIDE OF PREVIOUS PIPE
 *    1  | WRITE SIDE OF PIPE
 *    2  | STDERR
 * 4. if the command currenlty being processed is the last command file escriptor table should look 
 *    like the following
 *    FD |
 *    _______
 *    0  | READ SIDE OF PREVIOUS PIPE
 *    1  | STDOUT
 *    2  | STDERR
 *
 * * NOTE * at each iteration/ command process,
 * 1. create a new pipe if command being processed isnt the first one
 * 2. fork a process
 * 3. check if the command being processed is the first, middle, or last command
 * 4. alter file descriptor table respectively
 * 5. store the read side of the current pipe in pipehdl variable so process in next iteration can read
 *    from it
 * 6. continue
 *
 * ** TO BE Implemented
 * 1. Check for infile or outfile at each iteration
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <fcntl.h>
#include "builtin.h"
#include "parse.h"

/*******************************************
 * Set to 1 to view the command line parse *
 *******************************************/
#define DEBUG_PARSE 0
#define READ 0
#define WRITE 1

void print_banner ()
{
    printf ("                    ________   \n");
    printf ("_________________________  /_  \n");
    printf ("___  __ \\_  ___/_  ___/_  __ \\ \n");
    printf ("__  /_/ /(__  )_(__  )_  / / / \n");
    printf ("_  .___//____/ /____/ /_/ /_/  \n");
    printf ("/_/ Type 'exit' or ctrl+c to quit\n\n");
}


/* returns a string for building the prompt
 *
 * Note:
 *   If you modify this function to return a string on the heap,
 *   be sure to free() it later when appropirate!  */
static char* build_prompt ()
{
    char* directory;
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
       fprintf(stdout, "%s", cwd);
    else
       perror("getcwd() error");
    return  "$ ";
}


/* return true if command is found, either:
 *   - a valid fully qualified path was supplied to an existing file
 *   - the executable file was found in the system's PATH
 * false is returned otherwise */
static int command_found (const char* cmd)
{
    char* dir;
    char* tmp;
    char* PATH;
    char* state;
    char probe[PATH_MAX];

    int ret = 0;

    if (access (cmd, F_OK) == 0)
        return 1;

    PATH = strdup (getenv("PATH"));    

    for (tmp=PATH; ; tmp=NULL) {
        dir = strtok_r (tmp, ":", &state);
        if (!dir)
            break;

        strncpy (probe, dir, PATH_MAX);
        strncat (probe, "/", PATH_MAX);
        strncat (probe, cmd, PATH_MAX);
        
        if (access (probe, F_OK) == 0) {
            ret = 1;
            break;
        }
    }

    free (PATH);
    return ret;
}

// performs input file redirection by altering file descripor table
void infile(char* file){
    int fd;
    if((fd = open(file,O_RDONLY)) < 0){
        fprintf(stderr, "ERROR FAILED TO OPEN OUTPUT FILE\n");
        exit(0);
    }   
    if(dup2(fd, STDIN_FILENO) == -1){
        fprintf(stderr, "ERROR REDIRECTING OUTPUT TO FILE-infile\n");
        exit(0);
    }               
    close(fd);
}

// performs output file redirection by altering file descriptor table
void outfile(char* file){
    int fd;
    if((fd = open(file,O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0){
        fprintf(stderr, "ERROR FAILED TO OPEN OUTPUT FILE-outfile\n");
        exit(0);
    }
    if(dup2(fd, STDOUT_FILENO) == -1){
        fprintf(stderr, "ERROR REDIRECTING OUTPUT TO FILE-outfile\n");
        exit(0);
    }   
    close(fd);
}


/* Called upon receiving a successful parse.
 * This function is responsible for cycling through the
 * tasks, and forking, executing, etc as necessary to get
 * the job done! */
void execute_tasks (Parse* P)
{
    unsigned int t;
    
    // define file decriptor
    int fd[2];
    
    // process id declaration
    pid_t pid;

    // is to handle / store address of pipes from ith -1 iterations
    int pipehdl = 0;

    for (t = 0; t < P->ntasks; t++) {
        // for each command create a pipe at each iteratio except last on
        if( t != P->ntasks - 1){
            if(pipe(fd) == -1){
                fprintf(stderr, "Failed to create a pipe\n");
                exit(0);
            }
        else{
            printf("cannot create pipe as it's the ith-1 iteration\n");
            }
        }

        if (is_builtin (P->tasks[t].cmd)) {
            if(P->outfile != NULL){
                builtin_execute (P->tasks[t], P->outfile);
            } 
            else{
                builtin_execute (P->tasks[t], NULL);

            }
        }
        else if (command_found (P->tasks[t].cmd)) {
            // implement logic for shell
            // create child process
            pid = fork();
            if(pid < 0){
                fprintf(stderr, "Failed to create child process \n");
                exit(0);
            }
            else if(pid > 0){
                // parent logic
                int child_t;
                // close the write side
                // wait on child to finish process
                wait(&child_t);
                if( t == 0 && t == P->ntasks-1){
                    continue;
                }
                else if (t != P-> ntasks-1 || ( t == 0 && t != P->ntasks-1)){
                    // close the write side of the pipe
                    close(fd[WRITE]);
                    // pass the read end of the pipe into the variable to be used at further iterations
                    pipehdl = fd[READ];

                }
                else{
                }

                
            }
            else{
                // child logic
                // check for the case where only once command is specified by user
                if(t == 0 && t == P->ntasks - 1){
                    if(P->infile == NULL && P->outfile == NULL){
                        // execute command
                        execvp(P->tasks[t].cmd, P->tasks[t].argv);
                    }
                    else if(P->infile != NULL && P->outfile == NULL){    
                        infile(P->infile);
                        // execute command
                       if(execvp(P->tasks[t].cmd, P->tasks[t].argv) <0){
				printf("EXECVP FAILED \n");       
		       }
                    }
                    else if(P->outfile != NULL && P->infile == NULL){
                        outfile(P->outfile);
                        // execute command
                        execvp(P->tasks[t].cmd, P->tasks[t].argv);
                    }
                    else if(P->outfile != NULL && P->infile != NULL){
                        outfile(P->outfile);
                        infile(P->infile);
                        execvp(P->tasks[t].cmd, P->tasks[t].argv);
                    }
                }
                
                // check for the case where the first command is not the only command
                if(t == 0 && t != P->ntasks -1){
                 if(P->infile == NULL && (P->outfile == NULL || P->outfile != NULL)){
                    // close the readside of the pipe since have no need for it
                    close(fd[READ]);
                    //reroute command output
                    if(dup2(fd[WRITE], 1) == -1){
                        fprintf(stderr, "Failed To Reroute STDOUT");
                        exit(0);
                    }
                    // close the write file descriptor
                    close(fd[WRITE]);
                    // process command
                    pipehdl = fd[READ];
                    execvp(P->tasks[t].cmd, P->tasks[t].argv);
                    fprintf(stderr, "ERROR FAILED TO EXECUTE COMMAND \n");
                    }
                else if(P->infile != NULL){
                    infile(P->infile);
                        
                    // close the readside of the pipe since have no need for it
                    close(fd[READ]);
                    //reroute command output
                    if(dup2(fd[WRITE], 1) == -1){
                        fprintf(stderr, "Failed To Reroute STDOUT");
                        exit(0);
                    }  
                    // close the write file descriptor
                    close(fd[WRITE]);
                    // execute command
                    execvp(P->tasks[t].cmd, P->tasks[t].argv);
                    }
                }
                // check if were are the last command in the sequence
                if(t != 0 && t == P->ntasks-1){
                    if(P->outfile == NULL){    
                        // close the write side of the pipe
                        //close(fd[WRITE]);
                        if(dup2(pipehdl, STDIN_FILENO) == -1){
                            fprintf(stderr, "Failed TO Redirect input from stdin to old pipes read side\n");
                            exit(0);
                        }
                        // close the end of the previous pipe
                        close(pipehdl);
                        // close the read side of the current pipe
                        //close(fd[READ]);
                        // exeecute command
                        execvp(P->tasks[t].cmd, P->tasks[t].argv);
                    }
                    else {
                         outfile(P->outfile);
                         // close the write side of the pipe
                         if(dup2(pipehdl, STDIN_FILENO) == -1){
                             fprintf(stderr, "Failed TO Redirect input from stdin to old pipes read side\n");
                             exit(0);
                         }
                         // close the end of the previous pipe
                          close(pipehdl);
                         // exeecute command
                         execvp(P->tasks[t].cmd, P->tasks[t].argv);
                    }
                }

                if( t != 0 && t != P->ntasks-1){
                    if(dup2(pipehdl,STDIN_FILENO) == -1){
                        fprintf(stderr, "Failed TO Redirect input from stdin to old pipes read side\n");
                        exit(0);
                    }
                    // close the read end of the past pipe  
                    close(pipehdl);
                    // close the read side of the pipe
                    close(fd[READ]);
                    if(dup2(fd[WRITE],STDOUT_FILENO) == -1){
                        fprintf(stderr, "Failed to redirect output to current pipe \n");
                        exit(0);
                    }
                    // close the write side of the pipe
                    close(fd[WRITE]);
                    execvp(P->tasks[t].cmd, P->tasks[t].argv);
                }
            
            }
            
        }
        else {
            break;
            }
    }
}


int main (int argc, char** argv){
    char* cmdline;
    Parse* P;

    print_banner ();

    while (1) {
        cmdline = readline (build_prompt());
        if (!cmdline)       /* EOF (ex: ctrl-d) */
            exit (EXIT_SUCCESS);

        P = parse_cmdline (cmdline);
        if (!P)
            goto next;

        if (P->invalid_syntax) {
            printf ("pssh: invalid syntax\n");
            goto next;
        }

#if DEBUG_PARSE
        parse_debug (P);
#endif

        execute_tasks (P);

    next:
        parse_destroy (&P);
        free(cmdline);
    }
}
