#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

char *jsh_getline(){
    char *buf;
    size_t size = 512;

    buf = (char*)malloc(sizeof(char[size]));
    getline(&buf, &size, stdin);

    return buf;
}

char **jsh_splitline(char *line){
    char **arg_array;
    char *token;
    size_t size = 512;
    int i = 0;

    line[strcspn(line, "\n")] = 0;

    arg_array = (char**)malloc(sizeof(char*) * size);
    memset(arg_array, 0, 512);

    token = strtok(line, " ");
	while(token != NULL){
        arg_array[i] = token;
        i++;

        token = strtok(NULL, " ");
    }


    arg_array[i] = NULL;

    // Debug print
    for (int j = 0; j <= i; j++) {
        printf("array #%d: %s\n", j, arg_array[j] ? arg_array[j] : "NULL");
    }

    return arg_array;
}

int jsh_execute(char **args){
    pid_t pid, wpid;
    int status;

    if (args[0] == NULL){
        return 1;
    }

    pid = fork();
    if (pid == 0){
        // child process
        if (execvp(args[0], args) == -1){
            perror("jsh");
        }
    } else if (pid < 0){
        // error forking
        perror("jsh");
    } else {
        do {

            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 0;
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

    } while(!status);

    free(line);
    free(args);
}

int main(int argc, char **argv){
    system("clear");
    jsh_loop();

    return 0;
}


