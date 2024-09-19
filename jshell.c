#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

int cmdlength = 0;

typedef struct {
    char *name;
    void (*func)(char **);
} builtin_t;


/*returns nothing*/
void builtin_cd(char **args){
    if (args[1] != NULL){
        // checking if cd is called with no arguments
        chdir(args[1]);
    } else {
        // checking user's environment variables for "$HOME"
        chdir(getenv("HOME"));
    }
}

void builtin_exit(char **args){
    exit(EXIT_SUCCESS);
}

void builtin_history(char **args){
    FILE *fptr;
    char file[512];
    char mystring[512];

    snprintf(file, 512, "%s/.jsh_history", getenv("HOME"));

    fptr = fopen(file, "r");

    while (fgets(mystring, 512, fptr)){
        printf("%s", mystring);
    }
    printf("\n");

}

builtin_t builtins[] = {
    {"cd", builtin_cd},
    {"exit", builtin_exit},
    {"history", builtin_history},
    {NULL, NULL}
};

/* returns 0 on sucess, 1 on error */
int jsh_logline(char **args){
    FILE *fptr;
    char file[512];

    sprintf(file, "%s/.jsh_history", getenv("HOME"));
    fptr = fopen(file, "a");
    fprintf(fptr, "\n");


    for(int i = 0; i < cmdlength; i++){
        fprintf(fptr, "%s ", args[i]);
    }

    fclose(fptr);
    return 0;
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
    cmdlength = 0;

    line[strcspn(line, "\n")] = 0;

    arg_array = (char**)malloc(sizeof(char*) * size);
    memset(arg_array, 0, 512);

    token = strtok(line, " ");
    while(token != NULL){
        arg_array[cmdlength] = token;
        cmdlength++;

        token = strtok(NULL, " ");
    }


    arg_array[cmdlength] = NULL;

#ifdef DEBUG
    for (int j = 0; j <= cmdlength; j++) {
        printf("array #%d: %s\n", j, arg_array[j] ? arg_array[j] : "NULL");
    }
#endif

    return arg_array;
}

void jsh_launch(char **args){
    pid_t pid, wpid;
    int status;
    pid = fork();

    if (pid == 0){
        /* if no error on forking, execute the first command in an array with args specified */
        if (execvp(args[0], args) == -1){
            perror("jsh");
        }
    } else if (pid < 0){
        // error forking
        perror("jsh");
    } else {
        do {
            /* wait until the command is terminated before starting a new one */
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}

int jsh_execute(char **args){
    int i;

    /* break if no command specified */
    if (args[0] == NULL){
        return 1;
    }

    for (i = 0; builtins[i].name != NULL; i++){
        if (!strcmp(args[0], builtins[i].name)){
            builtins[i].func(args);
            return 0;
        }
    }

    jsh_launch(args);
    return 0;
}

void jsh_loop(void){
    char *line;
    char **args;
    char cwd[PATH_MAX];
    int status;

    do {
        printf("$ ");
        line = jsh_getline();
        args = jsh_splitline(line);
        jsh_logline(args);
        status = jsh_execute(args);

        free(line);
        free(args);
    } while(!status);

}

int main(int argc, char **argv){
    jsh_loop();

    return 0;
}
