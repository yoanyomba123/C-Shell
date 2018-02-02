#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "builtin.h"
#include "parse.h"

static char* builtin[] = {
    "exit",   /* exits the shell */
    "which",  /* displays full path to command */
    NULL
};


int is_builtin (char* cmd)
{
    int i;

    for (i=0; builtin[i]; i++) {
        if (!strcmp (cmd, builtin[i]))
            return 1;
    }

    return 0;
}


void builtin_execute (Task T, char* file)
{
    if (!strcmp (T.cmd, "exit")) {
        exit (EXIT_SUCCESS);
    }
    else {
        if(strcmp(T.argv[1], "which")==0){
		 printf("which: shell built-in command\n");
	}
	else if(strcmp(T.argv[1], "exit")==0){
		 printf("exit: shell built-in command\n");
	 }
	else{

	char* dir;
        char* tmp;
        char* PATH;
        char* state;
        char probe[PATH_MAX];
        int out;
        int fd;
        // check if command matches 'which'
        if(!strcmp(T.cmd , "which")){
            PATH = strdup (getenv("PATH"));           
            for (tmp=PATH; ; tmp=NULL) {
                dir = strtok_r (tmp, ":", &state);
                if (!dir)
                    break;
                strncpy (probe, dir, PATH_MAX);
                strncat (probe, "/", PATH_MAX);
                strncat (probe, T.argv[1], PATH_MAX);
                out = dup(1);
                if (access (probe, F_OK) == 0) {
                if(file != NULL){
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
                if(strcmp(T.argv[1], "which")==0){
                    printf("which: shell built-in command\n");
                }
                else if(strcmp(T.argv[1], "exit")==0){
                    printf("exit: shell built-in command\n");
                }
                else{
                    printf("%s\n", probe);
                }
            }
            if(dup2(out,1) == -1){
                fprintf(stderr, "ERROR REDIRECTING OUTPUT TO FILE\n");
                exit(0);
            }
            dup2(out, 1);
            close(out); 
            }
        }}
    }
}
