#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_BUFFER 1024

void chatloop(char *name, int socketFd);
void buildMessage(char *result, char *name, char *msg);
void setupAndConnect(struct sockaddr_in *serverAddr, struct hostent *host, int socketFd, long port);
void setNonBlock(int fd);
void interruptHandler(int sig);

static int socketFd;

int store_bit=0;

int main(int argc, char *argv[])
{
    char *name;
    struct sockaddr_in serverAddr;
    struct hostent *host;
    long port;

    if(argc != 4)
    {
        fprintf(stderr, "./client [username] [host] [port]\n");
        exit(1);
    }
    name = argv[1];
    if((host = gethostbyname(argv[2])) == NULL)
    {
        fprintf(stderr, "Couldn't get host name\n");
        exit(1);
    }
    port = strtol(argv[3], NULL, 0);
    if((socketFd = socket(AF_INET, SOCK_STREAM, 0))== -1)
    {
        fprintf(stderr, "Couldn't create socket\n");
        exit(1);
    }

    setupAndConnect(&serverAddr, host, socketFd, port);
    setNonBlock(socketFd);
    setNonBlock(0);

    //Set a handler for the interrupt signal
    signal(SIGINT, interruptHandler);

    chatloop(name, socketFd);
}

//Main loop to take in chat input and display output
void chatloop(char *name, int socketFd)
{
    fd_set clientFds;
    char chatMsg[MAX_BUFFER];
    char chatBuffer[MAX_BUFFER], msgBuffer[MAX_BUFFER];
int username_bit=0;

    while(1)
    {
        //Reset the fd set each time since select() modifies it
        FD_ZERO(&clientFds);
        FD_SET(socketFd, &clientFds);
        FD_SET(0, &clientFds);
if(username_bit==0){
strcpy(chatMsg,name);
chatMsg[strlen(name)] = '\0';
if(write(socketFd, chatMsg, MAX_BUFFER - 1) == -1) perror("write failed: ");
memset(&chatBuffer, 0, sizeof(chatBuffer));
username_bit=1;
}
        if(select(FD_SETSIZE, &clientFds, NULL, NULL, NULL) != -1) //wait for an available fd
        {
            for(int fd = 0; fd < FD_SETSIZE; fd++)
            {
                if(FD_ISSET(fd, &clientFds))
                {


                    if(fd == socketFd) //receive data from server
                    {
                        int numBytesRead = read(socketFd, msgBuffer, MAX_BUFFER - 1);
                        msgBuffer[numBytesRead] = '\0';
if(strcmp(msgBuffer,"Do you want to recieve?\n")==0){
store_bit=1;
}


if(strstr(msgBuffer,"ff")!=NULL){
char *file=NULL, *clean=NULL;

FILE *fp=NULL;

int numbytes;
char buf[MAX_BUFFER]={'\0'}, wbuffer[MAX_BUFFER]={'\0'};

file=strstr(msgBuffer,"/=");
file=file+2;

strcpy(wbuffer, file);

printf("%s\n",file);


//Open file
	if ( (fp = fopen("copy.txt", "wb")) == NULL){
		perror("fopen");
		exit(1);
	}
	
		numbytes = fwrite(wbuffer, sizeof(char), sizeof(wbuffer), fp);
fclose(fp);
}

                        printf("%s", msgBuffer);
                        memset(&msgBuffer, 0, sizeof(msgBuffer));
                    }
                    else if(fd == 0) //read from keyboard (stdin) and send to server
                    {

                        fgets(chatBuffer, MAX_BUFFER - 1, stdin);
                        if(strcmp(chatBuffer, "/exit\n") == 0){
                            interruptHandler(-1); //Reuse the interruptHandler function to disconnect the client
			}

			else if(strcmp(chatBuffer,"ls\n")==0){
				if(write(socketFd, chatBuffer, MAX_BUFFER - 1) == -1) perror("write failed: ");
				memset(&chatBuffer, 0, sizeof(chatBuffer));
			}

else if(store_bit==1){
if(write(socketFd, chatBuffer, MAX_BUFFER - 1) == -1) perror("write failed: ");
				memset(&chatBuffer, 0, sizeof(chatBuffer));
			}



			else if(strstr(chatBuffer,"-f")!=NULL){
				char *file=NULL, *clean=NULL;
				FILE *fp;

				int numbytes;	
				char buf[MAX_BUFFER]={'\0'};

				file=strstr(chatBuffer,"-f");
				file=file+3;
clean=strchr(chatBuffer,'\n');
*clean='\0';

printf("%s\n",file);

fp = fopen(file, "r");

if(fp==NULL){
printf("error\n");
}


		numbytes = fread(buf, sizeof(char), sizeof(buf), fp);
		printf("fread %d bytes\n ", numbytes);

fclose(fp);
strcat(chatBuffer,"\n/=");
strcat(chatBuffer,buf);
				if(write(socketFd, chatBuffer, MAX_BUFFER - 1) == -1) perror("write failed: ");
	                        memset(&chatBuffer, 0, sizeof(chatBuffer));

				
}


                        else
                        {
                            buildMessage(chatMsg, name, chatBuffer);
                            if(write(socketFd, chatMsg, MAX_BUFFER - 1) == -1) perror("write failed: ");
                            //printf("%s", chatMsg);
                            memset(&chatBuffer, 0, sizeof(chatBuffer));
                        }
                    }
                }
            }
        }
    }
}

//Concatenates the name with the message and puts it into result
void buildMessage(char *result, char *name, char *msg)
{
    memset(result, 0, MAX_BUFFER);
    strcpy(result, name);
    strcat(result, ": ");
    strcat(result, msg);
}

//Sets up the socket and connects
void setupAndConnect(struct sockaddr_in *serverAddr, struct hostent *host, int socketFd, long port)
{
    memset(serverAddr, 0, sizeof(serverAddr));
    serverAddr->sin_family = AF_INET;
    serverAddr->sin_addr = *((struct in_addr *)host->h_addr_list[0]);
    serverAddr->sin_port = htons(port);
    if(connect(socketFd, (struct sockaddr *) serverAddr, sizeof(struct sockaddr)) < 0)
    {
        perror("Couldn't connect to server");
        exit(1);
    }
}

//Sets the fd to nonblocking
void setNonBlock(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    if(flags < 0)
        perror("fcntl failed");

    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
}

//Notify the server when the client exits by sending "/exit"
void interruptHandler(int sig_unused)
{
    if(write(socketFd, "/exit\n", MAX_BUFFER - 1) == -1)
        perror("write failed: ");

    close(socketFd);
    exit(1);
}
