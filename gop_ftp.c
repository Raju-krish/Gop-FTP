#include "gop_ftp.h"
#include <string.h>

char cwd[MAX_SIZE] = {0};

void handle_client(int sockfd)
{
    char buf[MAX_SIZE] = {0};
    char dummy[MAX_SIZE] = {0};
    int ret = 0;

    while (1)
    {
        printf("gop_ftp> ");
        memset(buf, 0, MAX_SIZE);
        memset(dummy, 0, MAX_SIZE);

        if (fgets(buf, MAX_SIZE, stdin) == NULL) {
            printf("User input  error\n");
        }
        buf[strlen(buf) - 1] = '\0';

        strncpy(dummy, buf, strlen(buf));

        char *token = strtok(dummy, " "); // Gets the command to execute.
        char *args = strtok(NULL, " "); // [Todo] As of now one Argument is supported need to increase.

        fflush(stdout);
        printf("Reads from cli : %s\n",buf);
        
        if(strncmp(token, "ls", strlen(token)) == MATCHED)
        {
            list_server(sockfd);
        }
        
        if(strncmp(token, "get", strlen(token)) == MATCHED)
        {
            if(args != NULL)
            {
                printf("Requesting file from server: %s\n", args);
                send(sockfd, buf, MAX_SIZE, 0);
                get_file(sockfd, args);
            }
        }
        
        if(strncmp(token, "put", strlen(token)) == MATCHED)
        {
            if(args != NULL)
            {
                printf(" Sending file to server: %s\n", args);
                send(sockfd, buf, MAX_SIZE, 0);
                send_file(sockfd, cwd, args);
            }
        }

        if(strncmp(token, "bye", strlen(token)) == MATCHED)
        {
            send(sockfd, buf, MAX_SIZE, 0);
            break;
        }
        
        if(strncmp(token, "cd", strlen(token)) == MATCHED)
        {
            ret = send(sockfd, buf, MAX_SIZE, 0);
            if(ret < 0)
                printf("Send error cd : %s\n",strerror(errno));
        }

    }
}
void handle_server(int new_sockfd)
{
    int ret = 0;
    char buf[MAX_SIZE] = {0};

    while(1)
    {
        memset(buf, 0, MAX_SIZE);
        ret = read(new_sockfd, buf, MAX_SIZE);
        printf("From client: %s\n", buf);
        char *token = strtok(buf, " "); // Gets the command to execute.
        char *args = strtok(NULL, " "); // [Todo] As of now one Argument is supported need to increase.

        if(ret < 0)
            printf("Error: read failed\n");

        if(strncmp(token, "bye", strlen(token)) == MATCHED)
            break;

        if(strncmp(token, "ls", strlen(token)) == MATCHED)
        {
            printf("received args : ls\n");
            list_files(cwd, new_sockfd);
        }

        if(strncmp(token, "cd", strlen(token)) == MATCHED)
        {
            if(args != NULL)
            {
                printf("received args for cd to %s\n", args);
                snprintf(cwd + strlen(cwd), MAX_SIZE - strlen(cwd), "/%s", args);
                printf("Final path after cd : %s\n",cwd);
            }
        }

        if(strncmp(token, "get", strlen(token)) == MATCHED)
        {
            if(args != NULL)
            {
                printf("client requests : %s\n", args);
                send_file(new_sockfd, cwd, args);
            }
        }
        
        if(strncmp(token, "put", strlen(token)) == MATCHED)
        {
            if(args != NULL)
            {
                printf("client sends : %s\n", args);
                get_file(new_sockfd, args);
            }
        }
    }
}
/*
 * Function     : run_client
 * Description  : starts client
 * return       : SUCCESS / FAILURE
 */
int run_client()
{

    int sockfd = 0;
    int ret = 0;
    char buf[MAX_SIZE] = {0};
    char user_buf[MAX_SIZE] = {0};

    // struct to assign type, port,ip for ipV4 socket (AF_INET)
    struct sockaddr_in cli_addr;
    getcwd(cwd, MAX_SIZE);

    // Create a Socket 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        printf("Error : Unable to create socket [%d]", sockfd);
        return  FAILURE;
    }

    printf("Socket created successfully !!\n");

    // assigning sock_addr details
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_port = htons(6776);
    cli_addr.sin_addr.s_addr = INADDR_ANY;

    // connect the sockaddr with sockfd
    ret = connect(sockfd, (struct sockaddr*)&cli_addr, sizeof(cli_addr));
    if(ret < 0)
    {
        printf("Error : Unable to connect with socket [%d] \n", sockfd);
        return FAILURE;
    }

    printf("Socket connected  successfully with server !!\n");
    ret = read(sockfd, buf, MAX_SIZE);
    if(ret < 0)
        printf("Error: read failed\n");

    printf("read : %s\n", buf);

    handle_client(sockfd);
    close(sockfd);
    return SUCCESS;
}

/*
 * Function     : run_server
 * Description  : starts server
 * return       : SUCCESS / FAILURE
 */

int run_server()
{
    int sockfd, new_sockfd = 0;
    int ret = 0;
    char buf[MAX_SIZE] = {0};
    char *error;
    struct dirent *file = NULL;

    getcwd(cwd, MAX_SIZE);

    printf("Current working dir : %s\n",cwd);

    // struct to assign type, port,ip for ipV4 socket (AF_INET)
    struct sockaddr_in serv_addr, cli_addr;

    // Create a Socket 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        printf("Error : Unable to create socket [%d]", sockfd);
        return  FAILURE;
    }

    printf("Socket created successfully !!\n");

    // assigning sock_addr details
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(6776);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    // bind the sockaddr with sockfd
    ret = bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if(ret < 0)
    {
        printf("Error: Unable to bind the sockaddr with sockfd [%d]\n", ret);
        return  FAILURE;
    }
    printf("Binding to sockaddr successfull !!\n");

    // listening for connection at sockfd
    ret = listen(sockfd, 2);
    if(ret < 0)
    {
        printf("Error: Unable to listen the sockfd [%d]\n", ret);
        close(sockfd);
        return  FAILURE;
    }
    printf("listening to sockfd successfull !!\n");

    // accepting the connection in sockfd
    socklen_t size = sizeof(serv_addr);

    while(1)
    {
        printf("Server waits for connection......\n");
        strncpy(buf, "Hey, Server asks something", MAX_SIZE);
        new_sockfd = accept(sockfd, (struct sockaddr*)&serv_addr, &size);
        if (new_sockfd < 0) {
            printf("Error: Unable to accept the client connetcion[%d]\n", new_sockfd);
        }

        printf("Client connection success !!\n");
        ret = send(new_sockfd, buf, strlen(buf), 0);
        if(ret < 0)
            printf("Error write failed : %s\n", strerror(errno));

        handle_server(new_sockfd);
        close(new_sockfd);
    }
    close(sockfd);
    return SUCCESS;
}

int main(int argc, char *argv[])
{

    int ret = 0;
    // Argument to run FTP as server or client required
    if(argc < TWO_ARGS)
    {
        printf("[Gop_FTP] Addtional param to decide server/client required\n");
        return FAILURE;
    }

    // Trigger FTP client
    if( !strncmp(argv[1], "-c", 3) || !strncmp(argv[1], "client", 6))
    {
        printf("[Gop_FTP] Decided to run client\n");
        ret = run_client();
        if(ret < 0)
        {
            printf("[Gop_FTP] Unable to create client\n");
        }
    }

    //Trigger FTP server
    if( !strncmp(argv[1], "-s", 3) || !strncmp(argv[1], "server", 7))
    {
        printf("[Gop_FTP] Decided to run server\n");
        ret = run_server();
        if(ret < 0)
        {
            printf("[Gop_FTP] Unable to create server\n");
        }
    }

    return SUCCESS;
}
