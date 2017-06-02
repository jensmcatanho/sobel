#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(int argc, char **argv){
    pid_t pid;
    pid = fork();
    if(pid == -1){
      perror("fork");
      exit(1);
    } else if(pid == 0){
      pid = fork();
      if(pid == -1){
        perror("fork");
        exit(1);
      } else if(pid == 0){
        printf("Sou o processo C\n");
        exit(0);
      } else {
        waitpid(pid, NULL, 0);
        printf("Sou o processo B\n");
        exit(0);
      }
    } else {
      waitpid(pid, NULL, 0);
      printf("Sou o processo A\n");
    }
}
