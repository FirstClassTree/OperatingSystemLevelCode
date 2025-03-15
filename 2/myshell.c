#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

/*handler that gets children after finish, intended for when & finishes*/
void sigchild_handler(int num){
    /*grabs zombie processes in compleition of their process*/
    while(waitpid(-1,NULL,WNOHANG) >0){}
}

 
int prepare(void){
    /*sigaction for ctrl + c to not terminating*/
    struct sigaction sa ={0};
    sa.sa_handler = SIG_IGN;
    if(sigaction(SIGINT, &sa , NULL) == -1){
        perror("Siginit handler setup failed");
        exit(EXIT_FAILURE);
    }
    /* sigaction for sigchild - for & to prevent zombies from background */
    struct sigaction sa_sigchild;
    sa_sigchild.sa_handler = sigchild_handler;
    /*catches zombies after processs finish, recomended flags from the internt for it to work as intended */
    sa_sigchild.sa_flags = SA_RESTART | SA_NOCLDSTOP; 
    if(sigaction(SIGCHLD, &sa_sigchild, NULL) == -1){
        perror("SIGCHLD handler setup failed");
        exit(EXIT_FAILURE);
    }
    return 0;
}
/* will return 1 upon sucess and 0 oterwise as stated in assignment*/
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
                exit(0);
            }
            else if(pid == 0){
                if(execvp(arglist[0],arglist) ==-1 ){
                    perror("Failure in excucting command");
                    exit(0);
                }
            }
        }
        else{
            pid_t pid = fork();
            if(pid == -1){
                perror("Failure in forking");
                exit(0);
            }
            if(pid == 0){
                if(execvp(arglist[0],arglist) ==-1 ){
                    perror("Failure in excucting command");
                    exit(0);
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
                exit(0);
            }
            pid_t pid = fork();
            if(pid == -1){
                perror("Failure in forking");
                exit(0);
            }
            else if(pid == 0){/*first child*/
                /*closing read end for first child*/
                close(fd[0]);
                /*duplicating the write of the pipe into stdout */
                dup2(fd[1],1);
                /*closeing write end after*/
                close(fd[1]);
                
                if(execvp(arglist[0],arglist) ==-1 ){
                    perror("Failure in excucting command");
                    exit(0);
                }    
            }
            int pid2 = fork();
            if(pid2 == -1){
                    perror("Failure in forking");
                    exit(0);
                }
            else if(pid2 == 0){
                /*closing write side of the second child*/
                close(fd[1]);
                dup2(fd[0],0);
                close(fd[0]);
                if(execvp(arglist[singlePiping+1] ,arglist + singlePiping +1 ) == -1){
                    perror("Failure in excucting command");
                    exit(0);
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
                exit(0);
            }
            pid_t pid = fork();
            if(pid == -1){
                perror("Failure in forking");
                exit(0);
            }
            else if(pid == 0){/*first child*/
                /*Redirecting the file to stdin of command*/
                if(dup2(file,0) == -1){
                    perror("Failed in dup2");
                    exit(0);
                }
                close(file);
                
                if(execvp(arglist[0] ,arglist) == -1){
                    perror("Failure in excucting command");
                    exit(0);
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
                exit(0);
            }
            pid_t pid = fork();
            if(pid == -1){
                perror("Failure in forking");
                exit(0);
            }
            else if(pid == 0){/*first child*/
                /*Redirecting the file to stdout of command*/
                if(dup2(file,1) == -1){
                    perror("Failed in dup2");
                    exit(0);
                }
                close(file);
                
                if(execvp(arglist[0] ,arglist) == -1){
                    perror("Failure in excucting command");
                    exit(0);
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
    /*restoring default signal*/
    struct sigaction sa ={0};
    sa.sa_handler = SIG_DFL;
    if(sigaction(SIGINT, &sa , NULL) == -1){
        perror("Siginit handler setup failed");
        exit(EXIT_FAILURE);
    }

    return 0;

}