#define TRUE 1

#define TOKENMAX 256
#define MAXCHARPERLINE 1024
#define HISTORY_FILE "history.txt"

static char* currentDirectory;
extern char** environ;
int no_reprint_prmpt;
pid_t pid;
