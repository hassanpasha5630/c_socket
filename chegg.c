//   Compile with:
//   $ gcc mathServer.c -o mathServer -lpthread

//---       Header file inclusion                   ---//

#include "mathClientServer.h"
#include <errno.h>   // For perror()
#include <pthread.h> // For pthread_create()
#include <string.h>
#include <sys/socket.h> //For socket()
#include <netinet/in.h> //For sockaddr_in and htons()
#include <signal.h>


//---       Definition of constants:               ---//

#define STD_OKAY_MSG "Okay"

#define STD_ERROR_MSG "Error doing operation"

#define STD_BYE_MSG "Good bye!"

#define THIS_PROGRAM_NAME "mathServer"

#define FILENAME_EXTENSION ".bc"

#define OUTPUT_FILENAME "out.txt"

#define ERROR_FILENAME "err.txt"

#define CALC_PROGNAME "/usr/bin/bc"

const int ERROR_FD = -1;

extern void *handleClient(void *vPtr);
extern int getServerFileDescriptor();

//---       Definition of functions:               ---//

// PURPOSE: To run the server by 'accept()'-ing client requests from
//   'listenFd' and doing them.
void doServer(int listenFd)
{
    /* Application validity check: */

    /* Server clients: */
    pthread_t threadId;
    pthread_attr_t threadAttr;
    int threadCount = 0;
    int *iPtr;

    /* YOUR CODE HERE: */
    while (1)

    {
        /* Accept connection to client: */
        int conffd = accept(listenFd, NULL, NULL);

        /* Malloc memory for two integers: */
        iPtr = (int *)calloc(2, sizeof(int));

        /*Put file descriptor in the first space: */
        iPtr[0] = listenFd; // or just listenFd not sure

        /* put threadCount into the second space and increment: */
        iPtr[1] = threadCount++;

        /* Creates detached thread for handleClient and passes the address of iPtr */
        pthread_attr_init(&threadAttr);
        pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
        pthread_create(&threadId, &threadAttr, handleClient, (void *)iPtr);

        pthread_join(threadId, NULL);
        pthread_attr_destroy(&threadAttr);
    }
}
void *handleClient(void *vPtr)
{
    /* Read command: */
    char buffer[BUFFER_LEN];
    char command;
    int fileNum;
    char text[BUFFER_LEN];
    int shouldContinue = 1;
    int threadNum;
    int fd;

    /* Cast void* vPtr back to an int */
    int *iPtr = (int *)vPtr;

    /* Assign file descriptor to a local value named 'fd'*/
    fd = iPtr[0];
    /* Assign thread number to local value named 'threadNum'*/
    threadNum = iPtr[1];

    free(iPtr);

    while (shouldContinue)
    {
        memset(buffer, '\0', BUFFER_LEN);
        memset(text, '\0', BUFFER_LEN);

        read(fd, buffer, BUFFER_LEN);
        printf("Thread %d received: %s\n", threadNum, buffer);
        sscanf(buffer, "%c %d \"%[^\"]\"", &command, &fileNum, text);

        /* YOUR CODE HERE: */

        if (command == DIR_CMD_CHAR)
        {
            /* 1. Open the current directory (named "."). If an error occurs then just send STD_ERROR_MSG back to the client: */
            DIR *dirPtr = opendir(".");
            struct dirent *entryPtr;

            /* If error occurs send STD_ERROR_MSG to client: */
            if ((dirPtr = opendir(".")) == NULL)
            {
                {
                    write(fd, STD_ERROR_MSG, sizeof(STD_ERROR_MSG));
                    //return(EXIT_FAILURE);
                }

                /* Read as many entries that will fit into BUFFER_LEN
put as many entries into the buffer and send the buffer to client
d_name=entryPtr into the bufffer using strcat_s,
make sure buffer starts empty
buffer[0]='\n';
add new line char using stringcat "\n"
make sure do not go over buffer lengh */

                if (dirPtr)
                {
                    while ((entryPtr = readdir(dirPtr)) != NULL)
                    {

                        buffer[0] = '\0';

                        int i;
                        int sizebuf = sizeof(buffer);

                        for (i = 0; i < sizebuf; i++)
                        {
                            strcat(buffer, entryPtr->d_name);
                            strcat(buffer, "\n");
                        }
                    }
                }
                /* 3. Close directory */
                closedir(dirPtr);
            }

            else if (command == READ_CMD_CHAR)
            {
                /* 1. Open the file with the number fileNum. Get the name from the number with: */
                fileNum = open(FILENAME_EXTENSION, O_RDONLY, 0); //Yeah, I think this is right, except with fileNum instead.

                // changed fd to fileNum

                /* If error occurs send STD_ERROR_MSG to client: */
                if (fileNum < 0)
                {
                    write(fileNum, STD_ERROR_MSG, sizeof(STD_ERROR_MSG)); // it looks like this is correct

                    exit(1);
                }

                /* 2. read() up to BUFFER_LEN into a buffer. Put a '\0' character at the end of the buffer: */
                char fileName[BUFFER_LEN];
                snprintf(fileName, BUFFER_LEN, "%d%s", fileNum, FILENAME_EXTENSION);

                fileNum = read(fileNum, fileName, BUFFER_LEN - 1);

                fileName[fileNum] = '\0';
                printf("Sending: %s", fileName);
                write(fileNum, fileName, fileNum);
                /* Close file: */
                close(fileNum);
            }

            else if (command == WRITE_CMD_CHAR)
            {
                /* 1. Open the file with the number fileNum in write mode: */
                fileNum = open(FILENAME_EXTENSION, O_WRONLY | O_CREAT | O_TRUNC, 0660);

                /* If error occurs send STD_ERROR_MSG to client: */
                if (fileNum < 0)
                {
                    write(fileNum, STD_ERROR_MSG, sizeof(STD_ERROR_MSG));
                }

                if (fileNum > 0)
                {
                    //fileNum and text have the file number and text respectively from the client.
                    //Save that text to the named file.
                    char fileName[BUFFER_LEN];
                    write(fileNum, fileName, sizeof(fileName));

                    //If you succeeded then send STD_OKAY_MSG back to the client
                    write(fileNum, STD_OKAY_MSG, sizeof(STD_OKAY_MSG));
                }

                /* Close file: */
                close(fileNum);
            }

            else if (command == CALC_CMD_CHAR)
            {
                /* Create new fork named calcPid: */
                int calcPid = fork();

                /* If fork() failed: */
                if (calcPid < 0)
                {
                    write(fd, STD_ERROR_MSG, sizeof(STD_ERROR_MSG));
                }
                /* Child writes files: */
                else if (calcPid == 0)
                {
                    char fileName[BUFFER_LEN];

                    snprintf(fileName, BUFFER_LEN, "%d%s", fileNum, FILENAME_EXTENSION);

                    int inFd = open(fileName, O_RDONLY, 0);
                    int outFd = open(OUTPUT_FILENAME, O_WRONLY | O_CREAT | O_TRUNC, 0660);
                    int errFd = open(ERROR_FILENAME, O_WRONLY | O_CREAT | O_TRUNC, 0660);

                    if ((inFd < 0) || (outFd < 0) || (errFd < 0))
                    {
                        fprintf(stderr, "Could not open one or more files\n");
                        exit(EXIT_FAILURE);
                    }

                    close(0);
                    dup(inFd);
                    close(1);
                    dup(outFd);
                    close(2);
                    dup(errFd);

                    /* Run CALC_PROGNAME, if fails generate output: */
                    execl(CALC_PROGNAME, CALC_PROGNAME, fileName, NULL);
                    fprintf(stderr, "Could not execl %s\n", CALC_PROGNAME);
                    exit(EXIT_FAILURE);
                }
                /* Parent reads files: */
                else
                {

                    /* Wait and check status of child process, if it crashes return STD_ERROR_MSG: */
                    int status;
                    pid_t return_pid = waitpid(calcPid, &status, WNOHANG);

                    if (return_pid == -1)
                        write(fd, STD_ERROR_MSG, sizeof(STD_ERROR_MSG));

                    /* If child process EXIT_SUCCESS, then run parent process: */
                    if (return_pid == calcPid)
                    {
                        char fileName[BUFFER_LEN];

                        snprintf(fileName, BUFFER_LEN, "%d%s", fileNum, FILENAME_EXTENSION);
                        int numBytes = 0;

                        int outFd = open(OUTPUT_FILENAME, O_RDONLY | O_CREAT, 0660);
                        int errFd = open(ERROR_FILENAME, O_RDONLY | O_CREAT, 0660);

                        if ((outFd < 0) || (errFd < 0))
                        {
                            fprintf(stderr, "Could not open one or more files\n");
                            exit(EXIT_FAILURE);
                        }

                        while ((numBytes = read(outFd, fileName, BUFFER_LEN)) > 0)
                            read(errFd, &fileName[numBytes], BUFFER_LEN);

                        close(0);
                        dup(outFd);
                        close(1);
                        dup(errFd);
                    }
                }
            }

            else if (command == DELETE_CMD_CHAR)
            {
                if (fileNum > 0)
                {
                    fileNum = unlink(buffer);
                    write(fd, STD_OKAY_MSG, sizeof(STD_OKAY_MSG));
                }
                else
                    write(fd, STD_ERROR_MSG, sizeof(STD_ERROR_MSG));
            }

            else if (command == QUIT_CMD_CHAR)
            {
                /* Send STD_BYE_MSG back to the client and set shouldContinue to 0 to quit the loop: */
                write(fd, STD_BYE_MSG, sizeof(STD_BYE_MSG));
                shouldContinue = 0;
            }
        }
    }

    printf("Thread %d quitting.\n", threadNum);
    return (NULL);
}

// PURPOSE: To decide a port number, either from the command line arguments
//   'argc' and 'argv[]', or by asking the user. Returns port number.
int getPortNum(int argc,
               char *argv[])
{
    // I. Application validity check:

    // II. Get listening socket:
    int portNum;

    if (argc >= 2)
        portNum = strtol(argv[1], NULL, 0);
    else
    {
        char buffer[BUFFER_LEN];

        printf("Port number to monopolize? ");
        fgets(buffer, BUFFER_LEN, stdin);
        portNum = strtol(buffer, NULL, 0);
    }

    // III. Finished:
    return (portNum);
}

// PURPOSE: To attempt to create and return a file-descriptor for listening
//   to the OS telling this server when a client process has connect()-ed
//   to 'port'. Returns that file-descriptor, or 'ERROR_FD' on failure.
int getServerFileDescriptor(int port)
{
    // I. Application validity check:

    // II. Attempt to get socket file descriptor and bind it to 'port':
    // II.A. Create a socket
    int socketDescriptor = socket(AF_INET,     // AF_INET domain
                                  SOCK_STREAM, // Reliable TCP
                                  0);

    if (socketDescriptor < 0)
    {
        perror(THIS_PROGRAM_NAME);
        return (ERROR_FD);
    }

    // II.B. Attempt to bind 'socketDescriptor' to 'port':
    // II.B.1. We'll fill in this datastruct
    struct sockaddr_in socketInfo;

    // II.B.2. Fill socketInfo with 0's
    memset(&socketInfo, '\0', sizeof(socketInfo));

    // II.B.3. Use TCP/IP:
    socketInfo.sin_family = AF_INET;

    // II.B.4. Tell port in network endian with htons()
    socketInfo.sin_port = htons(port);

    // II.B.5. Allow machine to connect to this service
    socketInfo.sin_addr.s_addr = INADDR_ANY;

    // II.B.6. Try to bind socket with port and other specifications
    int status = bind(socketDescriptor, // from socket()
                      (struct sockaddr *)&socketInfo,
                      sizeof(socketInfo));

    if (status < 0)
    {
        perror(THIS_PROGRAM_NAME);
        return (ERROR_FD);
    }

    // II.B.6. Set OS queue length:
    listen(socketDescriptor, 5);

    // III. Finished:
    return (socketDescriptor);
}

int main(int argc,
         char *argv[])
{
    // I. Application validity check:

    // II. Do server:
    int port = getPortNum(argc, argv);
    int listenFd = getServerFileDescriptor(port);
    int status = EXIT_FAILURE;

    if (listenFd >= 0)
    {
        doServer(listenFd);
        close(listenFd);
        status = EXIT_SUCCESS;
    }

    // III. Finished:
    return (status);
}