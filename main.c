#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "main.h"


void mysh_start(char **args, int background) {
    int error = -1;

    if ((pid = fork()) == -1) {
        printf("Child process not created\n");
        return;
    }

    if (pid == 0) {
        signal(SIGINT, SIG_IGN);
        setenv("parent",getcwd(currentDirectory, 1024),1);
        if (execvp(args[0],args) == error) {
            printf("Command not exist");
            kill(getpid(),SIGTERM);
        }
    }

    if (background == 0) {
        waitpid(pid,NULL,0);
    } else {
        printf("New process PID : %d\n",pid);
    }
}

// Manage all environment variables commands
int mysh_env(char * args[], int option) {
    char **env_aux;
    switch(option) {
        case 0: // list variables
            for(env_aux = environ; *env_aux != 0; env_aux ++) {
                printf("%s\n", *env_aux);
            }
            break;
        case 1: // set variable
            if ((args[1] == NULL) && args[2] == NULL) {
                printf("%s","Not enough input arguments\n");
                return -1;
            }
            if (getenv(args[1]) != NULL) {
                printf("%s", "The variable has been overwritten\n");
            } else {
                printf("%s", "The variable has been created\n");
            }

            if (args[2] == NULL) {
                setenv(args[1], "", 1);
            } else {
                setenv(args[1], args[2], 1);
            }
            break;
        case 2: // unset variable
            if(args[1] == NULL) {
                printf("%s","Missing arguments\n");
                return -1;
            }
            if (getenv(args[1]) != NULL) {
                unsetenv(args[1]);
                printf("%s", "The variable has been deleted\n");
            } else {
                printf("%s", "Variable not found\n");
            }
            break;
    }
    return 0;
}

void mysh_IO(char * args[], char* inputFile, char* outputFile, int option) {
    int error = -1;
    int fileDescriptor;

    if ((pid=fork()) == -1) {
        printf("Child not created\n");
        return;
    }
    if (pid == 0) {

        if (option == 0) {
            fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
            dup2(fileDescriptor, STDOUT_FILENO);
            close(fileDescriptor);
        } else if (option == 1) {
            fileDescriptor = open(inputFile, O_RDONLY, 0600);
            dup2(fileDescriptor, STDIN_FILENO);
            close(fileDescriptor);
            fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
            dup2(fileDescriptor, STDOUT_FILENO);
            close(fileDescriptor);
        }

        setenv("parent",getcwd(currentDirectory, 1024),1);

        if (execvp(args[0],args) == error) {
            printf("error");
            kill(getpid(),SIGTERM);
        }
    }
    waitpid(pid,NULL,0);
}

int mysh_cd(char* args[]) {
    if (args[1] == NULL) {
        chdir(getenv("HOME"));
        return 1;
    } else {
        if (chdir(args[1]) == -1) {
            printf(" %s: no such directory in file\n", args[1]);
            return -1;
        }
    }
    return 0;
}

void mysh_perriPiperHandler(char *args[]) {
    int filedes[2];
    int filedes2[2];

    char *command[256];

    pid_t pid;

    int error = -1;

    int commandsNumber, end, i, j, k, l = 0;

    while (args[l] != NULL) {
        if (strcmp(args[l],"|") == 0) {
            commandsNumber++;
        }
        l++;
    }
    commandsNumber++;

    while (args[j] != NULL && end != 1) {
        k = 0;

        while (strcmp(args[j],"|") != 0) {
            command[k] = args[j];
            j++;
            if (args[j] == NULL) {

                end = 1;
                k++;
                break;
            }
            k++;
        }

        command[k] = NULL;
        j++;

        if (i % 2 != 0) {
            pipe(filedes);
        } else {
            pipe(filedes2);
        }

        pid = fork();

        if (pid == -1) {
            if (i != commandsNumber - 1) {
                if (i % 2 != 0) {
                    close(filedes[1]);
                } else {
                    close(filedes2[1]);
                }
            }
            printf("Creation of child process failed\n");
            return;
        }
        if (pid == 0) {
            if (i == 0) {
                dup2(filedes2[1], STDOUT_FILENO);
            } else if (i == commandsNumber - 1) {
                if (commandsNumber % 2 != 0) {
                    dup2(filedes[0],STDIN_FILENO);
                } else {
                    dup2(filedes2[0],STDIN_FILENO);
                }
            } else {
                if (i % 2 != 0) {
                    dup2(filedes2[0],STDIN_FILENO);
                    dup2(filedes[1],STDOUT_FILENO);
                } else {
                    dup2(filedes[0],STDIN_FILENO);
                    dup2(filedes2[1],STDOUT_FILENO);
                }
            }

            if (execvp(command[0],command) == error) {
                kill(getpid(),SIGTERM);
            }
        }

        if (i == 0) {
            close(filedes2[1]);
        } else if (i == commandsNumber - 1) {
            if (commandsNumber % 2 != 0) {
                close(filedes[0]);
            } else {
                close(filedes2[0]);
            }
        } else {
            if (i % 2 != 0) {
                close(filedes2[0]);
                close(filedes[1]);
            } else {
                close(filedes[0]);
                close(filedes2[1]);
            }
        }
        waitpid(pid,NULL,0);
        i++;
    }
}

int mysh_commands(char * args[]) {
    int i, j, background = 0;
    int fileDescriptor;
    int standardOut;
    int aux;
    char *args_aux[256];

    while (args[j] != NULL) {
        if ((strcmp(args[j],"<") == 0)
        || (strcmp(args[j],">") == 0)
        || (strcmp(args[j],"&") == 0)) {
            break;
        }
        args_aux[j] = args[j];
        j++;
    }


    if (strcmp(args[0],"exit") == 0) {
        exit(0);
    } else if (strcmp(args[0],"pwd") == 0) {
        if (args[j] != NULL) {
            if ((strcmp(args[j],">") == 0) && (args[j+1] != NULL)) {
                fileDescriptor = open(args[j+1], O_CREAT | O_TRUNC | O_WRONLY, 0600);
                standardOut = dup(STDOUT_FILENO);
                dup2(fileDescriptor, STDOUT_FILENO);
                close(fileDescriptor);
                printf("%s\n", getcwd(currentDirectory, 1024));
                dup2(standardOut, STDOUT_FILENO);
            }
        } else {
            printf("%s\n", getcwd(currentDirectory, 1024));
        }
    } else if (strcmp(args[0],"clear") == 0) {
        system("clear");
    } else if (strcmp(args[0],"cd") == 0) {
        mysh_cd(args);
    } else if (strcmp(args[0],"listenv") == 0) {
        if (args[j] != NULL) {
            if ((strcmp(args[j],">") == 0) && (args[j+1] != NULL)) {
                fileDescriptor = open(args[j+1], O_CREAT | O_TRUNC | O_WRONLY, 0600);
                standardOut = dup(STDOUT_FILENO);
                dup2(fileDescriptor, STDOUT_FILENO);
                close(fileDescriptor);
                mysh_env(args, 0);
                dup2(standardOut, STDOUT_FILENO);
            }
        } else {
            mysh_env(args, 0);
        }
    } else if (strcmp(args[0],"setenv") == 0) {
        mysh_env(args, 1);
    } else if (strcmp(args[0],"unsetenv") == 0) {
        mysh_env(args, 2);
    } else {
        while (args[i] != NULL && background == 0) {
            if (strcmp(args[i],"&") == 0) {
                background = 1;
            } else if (strcmp(args[i],"|") == 0) {
                mysh_perriPiperHandler(args);
                return 1;
            } else if (strcmp(args[i],"<") == 0) {
                aux = i + 1;
                if (args[aux] == NULL || args[aux+1] == NULL || args[aux+2] == NULL) {
                    printf("Not enough input arguments\n");
                    return -1;
                } else {
                    if (strcmp(args[aux+1],">") != 0){
                        printf("Usage: Expected '>' and found %s\n",args[aux+1]);
                        return -2;
                    }
                }
                mysh_IO(args_aux, args[i + 1], args[i + 3], 1);
                return 1;
            } else if (strcmp(args[i],">") == 0) {
                if (args[i+1] == NULL) {
                    printf("Not enough input arguments\n");
                    return -1;
                }
                mysh_IO(args_aux, NULL, args[i + 1], 0);
                return 1;
            }
            i++;
        }
        args_aux[i] = NULL;
        mysh_start(args_aux, background);
    }
    return 1;
}

int main(int argc, char *argv[], char **env) {
    char line[MAXCHARPERLINE];
    char *tokens[TOKENMAX];
    int numTokens;

    no_reprint_prmpt = 0;
    pid = -10;
    environ = env;

    setenv("shell",getcwd(currentDirectory, 1024), 1);

    while (TRUE) {
        if (no_reprint_prmpt == 0) {
            char hostn[1204] = "";
            gethostname(hostn, sizeof(hostn));
            printf("%s@%s %s > ", getenv("LOGNAME"), hostn, getcwd(currentDirectory, 1024));
        }
        no_reprint_prmpt = 0;

        memset(line, '\0', MAXCHARPERLINE);
        fgets(line, MAXCHARPERLINE, stdin);

        if ((tokens[0] = strtok(line," \n\t")) == NULL) {
            continue;
        }

        numTokens = 1;
        while ((tokens[numTokens] = strtok(NULL, " \n\t")) != NULL) {
            numTokens++;
        }
        mysh_commands(tokens);
    }

    exit(0);
}