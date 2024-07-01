#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

void SIGINTHandler(int signum){
    printf("Use ctrl + D to terminate");
    }

int prepare(void){
    /*Initalizing for process_argList*/
    struct sigaction newAction;
    newAction.sa_handler = SIGINTHandler;
    if(sigaction(SIGINT, &newAction , NULL) == -1){
        perror("Siginit handler regisitration failed");
        exit(EXIT_FAILURE);
    }
    return 0;
}

int process_arglist(int count, char **arglist){
    int singlePiping = -1;
    int inputRedirectingLeft = -1;
    int outputRedirecting = -1;
    
    for(int i = 0; i< count; i++){
        /*Running Both Processes with the ouput of the first being t input of the second using pipe and dup2*/
        if(strcmp(arglist[i],"|") == 0){
            singlePiping = i;
       }
        /*redirecting input that appears after the sign to be the stdin from the input file using dup2*/
        else if(strcmp(arglist[i],"<") == 0){
            inputRedirectingLeft = i;
        }
        /*redirecting the output that appears after the sign redirected to the output file dup2*/
        else if(strcmp(arglist[i],">>") == 0){
            outputRedirecting = i;
        }
    }
    if(singlePiping == -1 && inputRedirectingLeft == -1 && outputRedirecting == -1 ){
        /*If the last word is &, we run the child process in the background*/
        if(strcmp(arglist[count -1],"&") == 0){
            arglist[count-1] = NULL;
            pid_t pid = fork();
            if(pid == -1){
                perror("Failure in forking");
                exit(EXIT_FAILURE);
            }
            else if(pid == 0){
                signal(SIGINT, SIG_IGN);
                if(execvp(arglist[0],arglist) ==-1 ){
                    perror("Failure in excucting command");
                    exit(EXIT_FAILURE);
                }
            }
        }
        else{
            pid_t pid = fork();
            if(pid == -1){
                perror("Failure in forking");
                exit(EXIT_FAILURE);
            }
            if(pid == 0){
                signal(SIGINT, SIG_IGN);
                if(execvp(arglist[0],arglist) ==-1 ){
                    perror("Failure in excucting command");
                    exit(EXIT_FAILURE);
                }
            }
            else{
                int status;
                 pid = wait(&status);
            }

        }
    }
    else{
        if(singlePiping != -1){
            int fd[2];
            /*Spliting command into to with NULL*/
            arglist[singlePiping] = NULL;
            /*creating the pipe*/
            if(pipe(fd) == -1 ){
                perror("Failure in pipe");
                exit(EXIT_FAILURE);
            }
            pid_t pid = fork();
            if(pid == -1){
                perror("Failure in forking");
                exit(EXIT_FAILURE);
            }
            else if(pid == 0){/*first child*/
                signal(SIGINT, SIG_IGN);
                /*closing read end for first child*/
                close(fd[0]);
                /*duplicating the write of the pipe into stdout */
                dup2(fd[1],1);
                /*closeing write end after*/
                close(fd[1]);
                
                if(execvp(arglist[0],arglist) ==-1 ){
                    perror("Failure in excucting command");
                    exit(EXIT_FAILURE);
                }    
            }
            int pid2 = fork();
            if(pid2 == -1){
                    perror("Failure in forking");
                    exit(EXIT_FAILURE);
                }
            else if(pid2 == 0){
                signal(SIGINT, SIG_IGN);
                /*closing write side of the second child*/
                close(fd[1]);
                dup2(fd[0],0);
                close(fd[0]);
                if(execvp(arglist[singlePiping+1] ,arglist + singlePiping +1 ) == -1){
                    perror("Failure in excucting command");
                    exit(EXIT_FAILURE);
                }
            }
                /*parent needs to close both sides*/
                close(fd[0]);
                close(fd[1]);
                /*waiting for the first son*/
                int status1;
                pid = wait(&status1);
                int status2;
                pid2 = wait(&status2);
                    
                    
                
            }
        
        else if(inputRedirectingLeft != -1 ){
            arglist[inputRedirectingLeft] = NULL;

            int file = open(arglist[inputRedirectingLeft + 1],O_RDONLY);
            if(file == -1){
                perror("Invalid shell command:");
                exit(EXIT_FAILURE);
            }
            pid_t pid = fork();
            if(pid == -1){
                perror("Failure in forking");
                exit(EXIT_FAILURE);
            }
            else if(pid == 0){/*first child*/
                signal(SIGINT, SIG_IGN);
                /*Redirecting the file to stdin of command*/
                if(dup2(file,0) == -1){
                    perror("Failed in dup2");
                    exit(EXIT_FAILURE);
                }
                close(file);
                
                if(execvp(arglist[0] ,arglist) == -1){
                    perror("Failure in excucting command");
                    exit(EXIT_FAILURE);
                }
            }
            else{/*parent*/
                /*close read*/
                close(file);
                /*And wait for them to finish*/
                int status;
                pid = wait(&status);
            }
        }
        else if(outputRedirecting != -1){
            arglist[outputRedirecting] = NULL;

            /* opens or create file if it doesn't exist in the way I saw on the web*/
            int file = open(arglist[outputRedirecting+1],O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
            if(file == -1){
                perror("Failure in file Opening");
                exit(EXIT_FAILURE);
            }
            pid_t pid = fork();
            if(pid == -1){
                perror("Failure in forking");
                exit(EXIT_FAILURE);
            }
            else if(pid == 0){/*first child*/
                signal(SIGINT, SIG_IGN);
                /*Redirecting the file to stdout of command*/
                if(dup2(file,1) == -1){
                    perror("Failed in dup2");
                    exit(EXIT_FAILURE);
                }
                close(file);
                
                if(execvp(arglist[0] ,arglist) == -1){
                    perror("Failure in excucting command");
                    exit(EXIT_FAILURE);
                }
            }
            else{/*parent*/
                /*close read*/
                close(file);
                /*And wait for them to finish*/
                int status;
                 pid = wait(&status);
            }

        }
    }
    return 1;
}



int finalize(void){
    /*Code cleanup*/

    return 0;

}