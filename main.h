#define TRUE 1

#define TOKENMAX 256
#define MAXCHARPERLINE 1024

static char* currentDirectory;
extern char** environ;
int no_reprint_prmpt;
pid_t pid;
