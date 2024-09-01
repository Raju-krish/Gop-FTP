#include "gop_ftp.h"
#include <bits/types/FILE.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_SIZE_DIR  128


void get_file(int sockfd, char *file)
{
    int file_fd = 0;
    int ret = 0;
    int bytes = 0;
    char buffer[MAX_SIZE] = {0};

    if(file == NULL)
    {
        printf("File is NULL \n");
        return;
    }
    
    file_fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if(file_fd < 0)
    {
        printf("Unable to open file : %s : error : %s\n", file, strerror(errno));
        close(file_fd);
        return;
    }
    while( (bytes = recv(sockfd, buffer, MAX_SIZE, 0)) > 0)
    {
        if(strncmp(buffer, "##END gop_ftp\r\n", bytes) == 0)
            break;
        printf("bytes received  = %d\n",bytes);
        if(write(file_fd, buffer, bytes) != bytes)
        {
            printf("Error in writing the bffer to file : %s : errno :%s\n", file,strerror(errno));
        }
    }
    printf("Read from file compltes\n");
}

void send_file(int sockfd, char *cwd, char *file)
{
    int file_fd = 0;
    int ret = 0;
    struct stat file_stat;
    off_t offset = 0;
    char buffer[MAX_SIZE] = {0};

    if(file == NULL)
    {
        printf("File is NULL \n");
        return;
    }
    
    file_fd = open(file, O_RDONLY);
    if(file_fd < 0)
    {
        printf("Unable to open file : %s : error : %s\n", file, strerror(errno));
        close(file_fd);
        return;
    }

    if (fstat(file_fd, &file_stat) < 0) {
        printf("Unable to get file stats for  %s : %s\n", file, strerror(errno));
        close(file_fd);
        return;
    }
    printf("File Size of %s : %d\n",file, (int)file_stat.st_size);

    ret = sendfile(sockfd, file_fd, &offset, file_stat.st_size);
    if(ret < 0)
    {
        printf("Unable to send file  %s : %s\n", file, strerror(errno));
        close(file_fd);
        return;
    }

    sleep(2);
    strncpy(buffer, "##END gop_ftp\r\n", sizeof(buffer));
    ret = send(sockfd, buffer, strlen(buffer), 0);
    if (ret < 0)
        printf("Unable to send file  %s : %s\n", file, strerror(errno));
    
    close(file_fd);
    return;
}

void list_server(int sockfd)
{
    int ret = 0;
    char buf[MAX_SIZE] = {0};
    ret = send(sockfd, "ls", MAX_SIZE, 0);

    printf("Reading from Server ls \n");

    while( recv(sockfd, buf, MAX_SIZE, 0) >= 0)
    {
        if(strstr(buf,"ls over") != NULL)
            break;
        if(strstr(buf, "DIR"))
            printf("\e[1;34m %s \e[0m\n",buf);
        else 
            printf("%s\n", buf);
    }
}

void list_files(char *dir_name, int cli_fd)
{

    DIR *fd = NULL;
    char dir[MAX_SIZE_DIR] = ".";
    char buf[MAX_SIZE] = {0};
    struct dirent *file = NULL;

    if(dir_name != NULL)
    {
        strncpy(dir, dir_name, MAX_SIZE_DIR);
    }

    /* Open the directory */
    fd = opendir(dir);

    if(fd == NULL)
    {
        printf("Failed to open %s: %s\n", dir, strerror(errno));
        return;
    }

    
    /* If Dir is not empty */
    while((file = readdir(fd)) != NULL)
    {
        memset(buf, 0, MAX_SIZE);
        switch (file->d_type)
        {
            case DT_DIR:
                printf("DIRECTORY : %s\n", file->d_name);
                snprintf(buf, 1024, "%s\t: %s", "DIR", file->d_name);
                break;
            case DT_REG:
            default:
                printf("FILE      : %s\n", file->d_name);
                snprintf(buf, 1024, "%s\t: %s", "FILE", file->d_name);
                break;
        }
        send(cli_fd, buf, 1024, 0);
    }
    send(cli_fd, "ls over", 1024, 0);

    closedir(fd);
    return;
}
