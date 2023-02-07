#define TRUE 1

#define TOKENMAX 256
#define MAXCHARPERLINE 1024
#define HISTORY_FILE "history.txt"

static char* currentDirectory;
extern char** environ;
pid_t pid;
