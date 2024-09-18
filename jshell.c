#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

/*returns nothing*/
void builtin_cd(char *dest){
    if (dest != NULL){
        // checking if cd is called with no arguments
        chdir(dest);
    } else {
        // checking user's environment variables for "$HOME"
        chdir(getenv("HOME"));
    }
}

/*returns a pointer to a buffer containing the command entered and returns 0 on error*/
char *jsh_getline(){
    char *buf;
    int glret;
    size_t size = 512;

    buf = (char*)malloc(sizeof(char[size]));

    if (!buf){
        perror("Error allocating to buf");
    }

    glret = getline(&buf, &size, stdin);

    if (!glret){
        perror("Error getting line in jsh_getline");
        return 0;
    } else {
        return buf;
    }
}

/*returns a pointer to an array of commands & arguments*/
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

    #ifdef DEBUG
    for (int j = 0; j <= i; j++) {
        printf("array #%d: %s\n", j, arg_array[j] ? arg_array[j] : "NULL");
    }
    #endif

    return arg_array;
}

int jsh_execute(char **args){
    pid_t pid, wpid;
    int status;

    if (args[0] == NULL){
        return 1;
    }


    /*checks whether the command is cd*/
    if (!strcmp(args[0], "cd")){
        builtin_cd(args[1]);
    } else if (!strcmp(args[0], "exit")){
        return 1;
    } else {
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
    }


    return 0;
}

void jsh_loop(void){
    char *line;
    char **args;
    char cwd[PATH_MAX];
    int status;

    do {
        getcwd(cwd, sizeof(cwd));
        printf("%s\n$ ", cwd);
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


