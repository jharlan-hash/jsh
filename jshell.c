#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

char *jsh_getline(){
    char *buf;
    size_t size = 512;

    buf = (char*) malloc(sizeof(char[size]));
    getline(&buf, &size, stdin);

    return buf;
}

char **jsh_splitline(char *line){
    char **arg_array;
    size_t size = 512;
    int i = 0;

    arg_array = (char**)malloc(sizeof(char[size]));
    arg_array[i] = strtok(line, " ");

    while(arg_array[i] != NULL){
        arg_array[++i] = strtok(NULL, " ");
    }

    return arg_array;
}

int jsh_execute(char **args){
    pid_t pid, wpid;
    int status;

    pid = fork();
    execvp(args[0], args);
    waitpid(pid, &status, WUNTRACED);
    return 1;
}

void jsh_loop(void){
    char *line;
    char **args;
    int status;

    do {
        printf("$ ");
        line = jsh_getline();
        args = jsh_splitline(line);
        status = jsh_execute(args);

    } while(status);
}

int main(int argc, char **argv){
    system("clear");
    jsh_loop();

    return 0;
}


