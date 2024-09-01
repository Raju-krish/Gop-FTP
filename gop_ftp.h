#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <linux/limits.h>


// PRE-processor variables
#define FAILURE   -1
#define SUCCESS   0
#define TWO_ARGS  2
#define MAX_SIZE  1024

#define MATCHED 0

// Function declarations
void list_files(char *dir, int cli_fd);
void list_server(int sockfd);
void send_file(int sockfd, char *cwd, char *file);
void get_file(int sockfd, char *file);

