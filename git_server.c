/*-------------------------------------------------------------------------*
 *---                                                                   ---*
 *---                          mathServer.c                             ---*
 *---                                                                   ---*
 *---    This file defines a C program that gets file-sys commands      ---*
 *---from client via a socket, executes those commands in their own     ---*
 *---threads, and returns the corresponding output back to the          ---*
 *---client.                                                            ---*
 *---                                                                   ---*
 *---  ----  ----  ---- ----  ----  ----  ----   ----                   ---*
 *---                                                                   ---*
 *---Version 1.02017 February 20Joseph Phillips                         ---*
 *---                                                                   ---*
 *-------------------------------------------------------------------------*/

//Compile with:
//$ gcc mathServer.c -o mathServer -lpthread

//---Header file inclusion---//

#include "mathClientServer.h"
#include <errno.h>   // For perror()
#include <pthread.h> // For pthread_create()

//---Definition of constants:---//

#define STD_OKAY_MSG "Okay"

#define STD_ERROR_MSG "Error doing operation"

#define STD_BYE_MSG "Good bye!"

#define THIS_PROGRAM_NAME "mathServer"

#define FILENAME_EXTENSION ".bc"

#define OUTPUT_FILENAME "out.txt"

#define ERROR_FILENAME "err.txt"

#define CALC_PROGNAME "/usr/bin/bc"

extern void *handleClient(void *vPtr);
extern void *dirCommand(int fd);
extern void *readCommand(int clientFd, int fileNum);
extern void *writeCommand(int clientFd, int fileNum, char text);

const int ERROR_FD = -1;

//---Definition of functions:---//

//  PURPOSE:  To run the server by 'accept()'-ing client requests from
//'listenFd' and doing them.
void doServer(int listenFd)
{
    //  I.  Application validiity check:

    //  II.  Server clients:
    pthread_t threadId;
    pthread_attr_t threadAttr;
    int threadCount = 0;
    int *iPtr;

    // YOUR CODE HERE

    listen(listenFd, 5);

    //  pthread_create(&threadId,NULL,mathClient,NULL);
    pthread_attr_init(&threadAttr);
    while (1)
    {
        printf("pre connectDesc \n");
        int fd = accept(listenFd, NULL, NULL);
        printf("connectionDescriptor = %d\n", fd);
        if (fd < 0)
        {
            perror("Error on accept attempt\n");
            exit(EXIT_FAILURE);
        }

        printf("doing things in doServer\n");

        iPtr = (int *)calloc(2, sizeof(int *));
        iPtr[0] = fd;
        threadId = getpid();
        iPtr[1] = threadId;
        //    printf("In doServer, fd = %d, and threadCount = %d \n",fd, &threadId);
        printf("In doServer, iPtr[0] = %d, and iPtr[1] = %d \n", iPtr[0], iPtr[1]);
        threadCount++;
        //    printf("threadcount after ++ing it = %d\n", threadId);

        pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
        pthread_create(&threadId, &threadAttr, handleClient, (void *)iPtr);
    }
}

void *handleClient(void *vPtr)
{
    int *iPtr = (int *)vPtr;
    int fd = iPtr[0];
    int *threadId = &iPtr[1];
    printf("iPtr[0] (conDescriptor) = %d \n", fd);
    printf("iPtr[0] (conDesc again) = %d \n", fd);
    printf("actual iPtr[0] = %d \n", iPtr[0]);
    printf("actual iPtr[1] = %d \n", iPtr[1]);
    printf("iPtr[1] (threadId)   = %d \n", *threadId);

    free(vPtr);

    //  II.B.  Read command:
    char buffer[BUFFER_LEN];
    char command;
    int fileNum;
    char text[BUFFER_LEN];
    int shouldContinue = 1;
    char *textPtr;

    while (shouldContinue)
    {
        memset(buffer, '\0', BUFFER_LEN);
        memset(text, '\0', BUFFER_LEN);
        printf("inHandleClient, before the read, fd = %d \n", fd);
        read(fd, buffer, BUFFER_LEN);
        printf("Thread %d received: %s\n", *threadId, buffer);
        printf("inHandleClient, command = %s, fileNum = %d, text = %s, textLen = %d \n", &command, fileNum, text, strlen(text));
        sscanf(buffer, "%c %d \"%[^\"]\"", &command, &fileNum, text);

        // YOUR CODE HERE
        //    dirCommand();
        printf("buffer = %s \n", buffer);
        printf("text = %s \n", text);
        printf("Command = %s \n", &command);

        //    dirCommand();
        if (command == DIR_CMD_CHAR)
        {
            dirCommand(fd);
            printf("entered DIR_CMD_CHAR \n");
            //        shouldContinue=0;
        }
        else if (command == READ_CMD_CHAR)
        {
            printf("entered READ_CMD_CHAR \n");
            readCommand(fd, fileNum);
        }
        else if (command == WRITE_CMD_CHAR)
        {
            printf("entered WRITE_CMD_CHAR, text = %s \n", text); // need to figure out how to pass full text
                                                                  //        textPtr = (char*)malloc(sizeof(char) * strlen(text));
                                                                  //        textPtr = *text;
            writeCommand(fd, fileNum, text);
        }
    }
    printf("Thread %d quitting. \n", *threadId);
    return (NULL);
    //  }
}

void *dirCommand(int fd)
{
    DIR *dirPtr = opendir(".");

    if (dirPtr == NULL)
    {
        fprintf(stderr, STD_ERROR_MSG);
        exit(EXIT_FAILURE);
    }

    struct dirent *entryPtr;
    char buffer[BUFFER_LEN];
    char *filename;
    printf("in dirCommand, about to enter while \n");
    while ((entryPtr = readdir(dirPtr)) != NULL)
    {
        filename = entryPtr->d_name;
        strncat(buffer, filename, BUFFER_LEN);
        strncat(buffer, "\n", BUFFER_LEN);
    }
    write(fd, buffer, strlen(buffer));
    closedir(dirPtr);
}

void *readCommand(int clientFd,
                  int fileNum)
{
    char fileName[BUFFER_LEN];
    snprintf(fileName, BUFFER_LEN, "%d%s", fileNum, FILENAME_EXTENSION);
    printf("fileName = %s \n", fileName);
    printf("in readCommand: clientFd = %d, fileNum = %d \n", clientFd, fileNum);
    char buffer[BUFFER_LEN];
    int fileFd = open(fileName, O_RDONLY, 0440); //
    printf("fileFd = %d \n", fileFd);
    if (fileFd == -1)
    {
        fprintf(stderr, STD_ERROR_MSG);
    }

    read(fileFd, buffer, BUFFER_LEN);
    strcat(buffer, "\0");
    write(clientFd, buffer, strlen(buffer));
    close(fileFd);
}

void *writeCommand(int clientFd,
                   int fileNum,
                   char text)
{
    char fileName[BUFFER_LEN];
    snprintf(fileName, BUFFER_LEN, "%d%s", fileNum, FILENAME_EXTENSION);
    printf("writeCmd, before sizing text: text = %s \n", &text); //Experimental
    size_t textLen = strlen(text);
    int numWritten;
    printf("clientFd = %d, fileNum = %d, text = %s, textLen = %d \n", clientFd, fileNum, text, textLen);

    int fileFd = open(fileName, O_WRONLY | O_CREAT, 0660);
    if (textLen <= BUFFER_LEN)
    {
        printf("writeCmd: entered textLen <= buffLen cond, textLen = %d \n", textLen);
        numWritten = write(fileFd, &text, textLen);
    }
    else
    {
        printf("writeCmd: entered textLen > buffLen cond, textLen = %d \n", textLen);
        numWritten = write(fileFd, &text, BUFFER_LEN);
    }
    if (numWritten != -1 && fileFd != -1)
    {
        printf("writeCmd: no errors \n");
        fprintf(stdout, STD_OKAY_MSG);
    }
    else
    {
        printf("writeCmd: there was an error");
        fprintf(stderr, STD_ERROR_MSG);
    }
    close(fileFd);
}

//void* 		writeCommand(int 	fileNum
//			     char	text)
//{

//}

//  PURPOSE:  To decide a port number, either from the command line arguments
//'argc' and 'argv[]', or by asking the user.  Returns port number.
int getPortNum(int argc,
               char *argv[])
{
    //  I.  Application validity check:

    //  II.  Get listening socket:
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

    //  III.  Finished:
    return (portNum);
}

//  PURPOSE:  To attempt to create and return a file-descriptor for listening
//to the OS telling this server when a client process has connect()-ed
//to 'port'.  Returns that file-descriptor, or 'ERROR_FD' on failure.
int getServerFileDescriptor(int port)
{
    //  I.  Application validity check:

    //  II.  Attempt to get socket file descriptor and bind it to 'port':
    //  II.A.  Create a socket
    int socketDescriptor = socket(AF_INET,     // AF_INET domain
                                  SOCK_STREAM, // Reliable TCP
                                  0);

    if (socketDescriptor < 0)
    {
        perror(THIS_PROGRAM_NAME);
        return (ERROR_FD);
    }

    //  II.B.  Attempt to bind 'socketDescriptor' to 'port':
    //  II.B.1.  We'll fill in this datastruct
    struct sockaddr_in socketInfo;

    //  II.B.2.  Fill socketInfo with 0's
    memset(&socketInfo, '\0', sizeof(socketInfo));

    //  II.B.3.  Use TCP/IP:
    socketInfo.sin_family = AF_INET;

    //  II.B.4.  Tell port in network endian with htons()
    socketInfo.sin_port = htons(port);

    //  II.B.5.  Allow machine to connect to this service
    socketInfo.sin_addr.s_addr = INADDR_ANY;

    //  II.B.6.  Try to bind socket with port and other specifications
    int status = bind(socketDescriptor, // from socket()
                      (struct sockaddr *)&socketInfo,
                      sizeof(socketInfo));

    if (status < 0)
    {
        perror(THIS_PROGRAM_NAME);
        return (ERROR_FD);
    }

    //  II.B.6.  Set OS queue length:
    listen(socketDescriptor, 5);

    //  III.  Finished:
    return (socketDescriptor);
}

int main(int argc,
         char *argv[])
{
    //  I.  Application validity check:

    //  II.  Do server:
    int port = getPortNum(argc, argv);
    int listenFd = getServerFileDescriptor(port);
    int status = EXIT_FAILURE;

    if (listenFd >= 0)
    {
        doServer(listenFd);
        close(listenFd);
        status = EXIT_SUCCESS;
    }

    //  III.  Finished:
    return (status);
}