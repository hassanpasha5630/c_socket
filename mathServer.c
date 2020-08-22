/*-------------------------------------------------------------------------*
 *---									---*
 *---		mathServer.c						---*
 *---									---*
 *---	    This file defines a C program that gets file-sys commands	---*
 *---	from client via a socket, executes those commands in their own	---*
 *---	threads, and returns the corresponding output back to the	---*
 *---	client.								---*
 *---									---*
 *---	----	----	----	----	----	----	----	----	---*
 *---									---*
 *---	Version 1.0					Joseph Phillips	---*
 *---									---*
 *-------------------------------------------------------------------------*/

//	Compile with:
//	$ gcc mathServer.c -o mathServer -lpthread

//---		Header file inclusion					---//

#include "mathClientServer.h"
#include <errno.h>   // For perror()
#include <pthread.h> // For pthread_create()

//---		Definition of constants:				---//

#define STD_OKAY_MSG "Okay"

#define STD_ERROR_MSG "Error doing operation"

#define STD_BYE_MSG "Good bye!"

#define THIS_PROGRAM_NAME "mathServer"

#define FILENAME_EXTENSION ".bc"

#define OUTPUT_FILENAME "out.txt"

#define ERROR_FILENAME "err.txt"

#define CALC_PROGNAME "/usr/bin/bc"

const int ERROR_FD = -1;

void *handleClient(void *vPtr)
{
    //  I.  Application validity check:

    //  II.  Handle client:
    //  II.A.  Get file descriptor:
    int *intArray = (int *)vPtr;
    int fd = intArray[0];
    int threadNum = intArray[1];

    free(vPtr);
    printf("Thread %d starting.\n", threadNum);

    //  II.B.  Read command:
    char buffer[BUFFER_LEN];
    char command;
    int shouldContinue = 1;
    // to dynamically get cwd
    char cwd[MAX_FILE_NUM];
    // file buffer 
    char file_list[BUFFER_LEN];
    // file name 
    char *file ;
    DIR *folder_to_read;
    struct dirent *dir;
    
    //Write_CMD_Char
    char output_file[BUFFER_LEN];
    //number for file 
    char fileNum;
    // text for file
    char text[BUFFER_LEN];
    // Client Folder 
    int client_folder;

    // reading file char 
    char input_file[BUFFER_LEN];
    char read_buffer[BUFFER_LEN];



    //delete file 
    char delete_file[BUFFER_LEN];

    while (shouldContinue)
    {
        read(fd, buffer, BUFFER_LEN);
        printf("Thread %d received: %s\n", threadNum, buffer);
        command = buffer[0];
        fileNum = buffer[2];
        // printf("command: %c",command);

       

    switch (command)
    {


    case QUIT_CMD_CHAR:
      printf("Thread %d quitting.\n", threadNum);
      write(fd, STD_BYE_MSG, sizeof(STD_BYE_MSG));
      shouldContinue = 0;
      return 0;
    case DIR_CMD_CHAR:
    // creating a buffer for a file 
    // instead of making this hardcoded we made it dynamic this way this code can work with any directory 
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
             folder_to_read = opendir(cwd);      
             if(folder_to_read){
                   //  II.A.  Create awk process and have it run awk:
                 while((dir = readdir(folder_to_read)) != NULL){
                      file = dir->d_name;    
                      strncat(file_list,file,BUFFER_LEN);
                      strncat(file_list,"\n",BUFFER_LEN);      
                 }
                 write(fd, file_list, sizeof(file_list));
        
                 break;

             }   
             
        } 
        else {
            // if not able to get the working folder send message 
            write(fd, STD_ERROR_MSG, sizeof(STD_ERROR_MSG));
           
            break;
        }
   
        break;
    case WRITE_CMD_CHAR:
        snprintf(output_file, BUFFER_LEN, "%c%s", fileNum, FILENAME_EXTENSION);
        int file_number_written;
        int fileFd = open(output_file, O_WRONLY | O_CREAT, 0660);
        file_number_written = write(fileFd, &text, BUFFER_LEN);
        
        if (file_number_written == -1 && fileFd == -1)
        {
            write(fd, STD_ERROR_MSG, sizeof(STD_ERROR_MSG));
          
            break;

        }
        close(fileFd);
        write(fd, "Success", sizeof("Success"));
     
        break;
    case READ_CMD_CHAR:
        printf("Hello '\n' ");
        snprintf(input_file, BUFFER_LEN, "%c%s", fileNum, FILENAME_EXTENSION);
        
        printf("input_file  %s",input_file);
        
        int read_fileFd = open(input_file, O_RDONLY, 0440); 

        if (read_fileFd == -1)
        {
            write(fd, STD_ERROR_MSG, sizeof(STD_ERROR_MSG));
            break;
        }

        read(read_fileFd, read_buffer, BUFFER_LEN);
        strcat(read_buffer, "\0");
        write(fd, read_buffer, sizeof(read_buffer));
        close(read_fileFd);
    
        break;
    case DELETE_CMD_CHAR:
        snprintf(delete_file, BUFFER_LEN, "%c%s", fileNum, FILENAME_EXTENSION);
        if (remove(delete_file) == 0){ 
        printf("Deleted successfully"); 
        write(fd, "Deleted successfully", sizeof("Deleted successfully"));
        }

    default:
      if (isdigit(command))
      {
          write(fd, "Bless you", sizeof("Bless you"));
        
      }
    }
    }

    //  III.  Finished:
    printf("Thread %d quitting.\n", threadNum);
    return (NULL);
}

//---		Definition of functions:				---//

//  PURPOSE:  To run the server by 'accept()'-ing client requests from
//	'listenFd' and doing them.
void doServer(int listenFd)
{
    //  I.  Application validity check:

    //  II.  Server clients:
    pthread_t threadId;
    pthread_attr_t threadAttr;
    int threadCount = 0;

    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);

    while (1)
    {
        int *clientFdPtr = (int *)malloc(2 * sizeof(int));

        clientFdPtr[0] = accept(listenFd, NULL, NULL);
        clientFdPtr[1] = threadCount++;

        pthread_create(&threadId, &threadAttr, handleClient, clientFdPtr);
    }

    pthread_attr_destroy(&threadAttr);

    //  III.  Finished:

    // YOUR CODE HERE
}

//  PURPOSE:  To decide a port number, either from the command line arguments
//	'argc' and 'argv[]', or by asking the user.  Returns port number.
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
//	to the OS telling this server when a client process has connect()-ed
//	to 'port'.  Returns that file-descriptor, or 'ERROR_FD' on failure.
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

int main(int argc, char *argv[])
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