#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    char *name;
    void (*func)(char **);
} builtin_t;


void builtin_cd(char **args){
    int error;

    if (args[1] != NULL){
        // checking if cd is called with no arguments
        if (chdir(args[1])){
            perror("error changing directories");
        }
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
    char file[PATH_MAX];
    int envlen = strlen(getenv("HOME"));
    int fnamelen = 15;

    if (snprintf(file, envlen + fnamelen, "%s/.jsh_history", getenv("HOME")) >= sizeof(file)){
        fprintf(stderr, "File path too long");
        exit(EXIT_FAILURE);
    }

    fptr = fopen(file, "r");
    if (!fptr){
        perror("error opening history file");
        exit(EXIT_FAILURE);
    }

    while (fgets(file, PATH_MAX, fptr)){
        printf("%s", file);
    }

    printf("\n");
}

/* returns 0 on success, 1 on error */
int jsh_logline(char **args){
    FILE *fptr;
    char file[PATH_MAX];
    int envlen = strlen(getenv("HOME"));
    int fnamelen = 15;

    if (snprintf(file, envlen + fnamelen, "%s/.jsh_history", getenv("HOME")) >= sizeof(file)){
        fprintf(stderr, "File path too long");
        exit(EXIT_FAILURE);
    }

    fptr = fopen(file, "a");
    if (!fptr){
        perror("error opening history file");
        exit(EXIT_FAILURE);
    }

    fprintf(fptr, "\n");

    for(int i = 0; args[i] != NULL; i++){
        fprintf(fptr, "%s ", args[i]);
    }

    fclose(fptr);
    return 0;
}

/*returns a pointer to a buffer containing the command entered and returns 0 on error*/
char *jsh_getline(){
    char *buf;
    int chars_read;
    size_t size = 16;

    buf = (char*)malloc(sizeof(char[size]));

    if (!buf){
        perror("Error allocating to buf");
        free(buf);
        exit(EXIT_FAILURE);
    }

    /* gnu getline automatically reallocates memory */
    chars_read = getline(&buf, &size, stdin);

    if (chars_read == -1) {
        if (feof(stdin)) {
            // End of file reached
            free(buf);
            return NULL;
        } else {
            perror("jsh: getline");
            free(buf);
            return NULL;
        }
    }

    return buf;
}

/*returns a pointer to an array of commands & arguments*/
char **jsh_splitline(char *line){
    int bufsize = LINE_MAX, position = 0;
    char **tokens = malloc(bufsize * sizeof(char));
    char *token;

    if (!tokens){
        perror("error allocating memory");
        exit(EXIT_FAILURE);
    }

    line[strcspn(line, "\n")] = 0;
    token = strtok(line, " ");
    while(token != NULL){
        tokens[position] = token;
        position++;

        if (position > bufsize){
            bufsize += 64;
            tokens = realloc(tokens, bufsize * sizeof(char*));

            if (!tokens){
                perror("error allocating memory");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, " ");
    }


    tokens[position] = NULL;
    return tokens;
}

int jsh_launch(char **args){
    pid_t pid, wpid;
    int status;
    pid = fork();

    if (pid == 0){
        /* if no error on forking, execute the first command in an array with args specified */
        if (execvp(args[0], args) == -1){
            perror("jsh");
            return 1;
        }
    } else if (pid < 0){
        // error forking
        perror("jsh");
        return 1;
    } else {
        do {
            /* wait until the command is terminated before starting a new one */
            waitpid(pid, &status, WUNTRACED);
            return 0;
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 0;
}

int jsh_execute(char **args){
    int i;
    builtin_t builtins[] = {
        {"cd", builtin_cd},
        {"exit", builtin_exit},
        {"history", builtin_history},
        {NULL, NULL}
    };

    /* don't break if no command specified */
    if (args[0] == NULL){
        return 0;
    }

    for (i = 0; builtins[i].name != NULL; i++){
        if (!strcmp(args[0], builtins[i].name)){
            builtins[i].func(args);
            return 0;
        }
    }

    return jsh_launch(args);
}

void jsh_loop(void){
    char *line;
    char **args;
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
