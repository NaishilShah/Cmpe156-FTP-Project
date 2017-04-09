#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int sockfd, portnum, n, dataport;
int exitconn=1;
int instcount = 0;
char iobuff[1024];
char message[1024];
struct sockaddr_in server_addr, client_addr;	  //sockaddr_in - structure that helps you to reference to the socket's elements
struct hostent *host;	      //hostent - structure is used to keep information related to host
socklen_t client_len;
int dataport = 2565;
int a=1; //variable to make sure the first command is always PORT



int estab_control_channel(int argc, char *argv[])
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);	//This call returns a socket descriptor that you can use in later system calls or -1 on error.

    if (sockfd < 0) {		//Check for error
        printf("ERROR: Could not open socket \n");
        exit(1);
    }
    portnum = atoi(argv[2]); //changing the portnumber to integer
    
    host = gethostbyname(argv[1]);	/* The gethostbyname() function returns a structure of type hostent for
					the given host name.  Here name is either a hostname or an IPv4 address in standard dot notation*/
					
	//check for errors
    if (host == NULL) {
        printf("ERROR: Input host not found.\n");
        exit(0);
    }

	//initialize client
    bzero((char *) &server_addr, sizeof(server_addr));	//initialize to zero to avoid garbage values

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portnum);
    bcopy((char *)host->h_addr, (char *)&server_addr.sin_addr.s_addr, host->h_length);

    // Connecting to server-Request 
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR: Could not connect to server\n");
        exit(1);
    }
    printf("Connected to Server \n ");
    return sockfd; // returning the file descriptor obtained for data transfer
}


int estab_data_channel(int dataport)
{
	int optval=1;
	//struct hostent *host;
	struct sockaddr_in srvr_addr, clnt_addr;
	int newsockfd1=0;
	printf("ENTERED DATA CHANNEL FUNCTION\n");
	int newsockfd = 0;
	newsockfd = socket(AF_INET, SOCK_STREAM, 0);	//This call returns a socket descriptor that you can use in later system calls or -1 on error.
    if (newsockfd <= 0) 
    {		//Check for error
        printf("ERROR: Could not open socket \n");
        exit(1);
    }
    
    optval = 1;
    if(setsockopt(newsockfd,SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&optval,sizeof(optval)) < 0)
		perror("ERROR in setsock");
    
    printf("The newsockfd is: %d\n",newsockfd);
    portnum = dataport; //changing the portnumber to integer
    
   // host = gethostbyname(argv[1]);	/* The gethostbyname() function returns a structure of type hostent for
					//the given host name.  Here name is either a hostname or an IPv4 address in standard dot notation*/
	printf("Waiting for REPLY on DATA CHANNEL\n");				

	//initialize client
    bzero((char *) &srvr_addr, sizeof(srvr_addr));	//initialize to zero to avoid garbage values

    srvr_addr.sin_family = AF_INET;
    srvr_addr.sin_addr.s_addr = INADDR_ANY;
    srvr_addr.sin_port = htons(portnum);
    //bcopy((char *)host->h_addr, (char *)&server_addr.sin_addr.s_addr, host->h_length);

	
    //bind assign local protocol address to socket; returns 0 if successful
	if (bind(newsockfd, (struct sockaddr *) &srvr_addr, sizeof(srvr_addr)) < 0)
    {
        perror("ERROR: Could not bind server");
        exit(1);
    }
    //printf("Before Listening\n");
    //Awaiting client for further instructions via listen:
    listen(newsockfd, 5);
    //printf("LISTENING");
    socklen_t clnt_len = sizeof(clnt_addr);

    //Accept connection request from client 
    newsockfd1 = accept(newsockfd, (struct sockaddr *) &clnt_addr, &clnt_len);
    if (newsockfd1 < 0) 
    {
        printf("ERROR: Could not accept client connection");
        exit(1);
    }
    close(newsockfd);
    //printf("old closed.\n");
	printf("Data Channel connnected\n");
    return newsockfd1;
}



void wait_for_ls_data(int dataport)
{
	int newsockfd = estab_data_channel(dataport);

	//Empty buffer for recev ack
	bzero(iobuff,1024);
	
	// Reading server ACK for ls
	n = read( newsockfd, iobuff, 1024 );
	if ( n < 0 ) //check for errors
	{
		printf("ERROR: Could not read from socket");
		exit(1);
	}
	
	printf("===================DATA CHANNEL===================\n");
	printf("Received on port %d\n", dataport);
	printf("%s\n", iobuff);	
	close(newsockfd);
	printf("===================CLOSED DATA CHANNEL===================\n");
}


void wait_for_get_data( int dataport)
{
	int newsockfd = estab_data_channel(dataport);

	//Empty buffer for recev data
	bzero(iobuff,1024);
	recv(newsockfd, iobuff, 2, 0);
	int file_desc, file_size;
	char *data;
	if (strcmp(iobuff, "OK") == 0) 
	{

		recv(newsockfd, &file_size, sizeof(int), 0);
		data = malloc(file_size);
		file_desc = open("get_output.txt", O_CREAT | O_EXCL | O_WRONLY, 0666);
		recv(newsockfd, data, file_size, 0);
		write(file_desc, data, file_size);
		close(file_desc);
		printf("===================DATA CHANNEL===================\n");
		printf("FILE required received via port %d\n", dataport);
	}
	else
	{
		printf("===================DATA CHANNEL===================\n"); 
		printf("No such file found at the server.\n");
	}

	close(newsockfd);
	printf("===================CLOSED DATA CHANNEL===================\n");


}


void send_put_file(int dataport, char *message)
{
	int datasockfd = estab_data_channel(dataport);
	if (access(message, F_OK) != -1)
		{
			strcpy(iobuff, "OK");
			write(datasockfd, iobuff, strlen(iobuff));
			//SendFileOverSocket(socket, file_name);
			
			struct stat	obj;
			int	file_desc, file_size;

			stat(message, &obj);
			file_desc = open(message, O_RDONLY);
			file_size = obj.st_size;
			send(datasockfd, &file_size, sizeof(int), 0);
			sendfile(datasockfd, file_desc, NULL, file_size);
	
			printf("===================DATA CHANNEL===================\n");
			printf("FILE sent to Server via port %d\n", dataport);
	
		}
		else 
		{
			// Requested file does not exist, notify the client
			strcpy(iobuff, "NO");
			write(datasockfd, iobuff, strlen(iobuff));
			printf("===================DATA CHANNEL===================\n");
			printf("NO file found. Error sent to server on %d\n", dataport);
	
		}
		
	close(datasockfd);
	printf("===================CLOSED DATA CHANNEL===================\n");
		
}


int transferdata(int sockfd)
{
    printf("> "); //a probe for the user to enter commands
    
    bzero(iobuff,1024); //empty the buffer for the user command
    fgets(iobuff,1024,stdin);
    
    //printf("%s",iobuff);

	/*if(a==1)
	{	
		/*if((strncmp(iobuff, "PORT", 4) == 0) || (strncmp(iobuff, "port", 4) == 0))
		{
			char *waste;
			char *token;
			int n5,n6,count=0;
			strcpy(message,iobuff);
			
			//calc data port number for itself
			waste = strtok(iobuff, " ");
			token = strtok(NULL, ",");
			while( token != NULL )
			{
				if(count==4)
					n5=atoi(token);
					//printf("%d\n",n5);
				
				if(count==5)
				{
					n6=atoi(token);
					//printf("%d\n",n6);
					dataport = (n5*256)+n6;
				}
				token = strtok(NULL, ",");
				count++;
			}
			
			strcpy(message,"PORT 127,0,0,1,10,5"); //this will create port number 2565
			
			
			// send PORT command to server
			n = write(sockfd, message, strlen(message));
		   
			if (n < 0) //check for errors
			{
				printf("ERROR: Could not write to socket");
				exit(1);
			}
			

			//empty the buffer before reading the server output
			bzero(iobuff,1024);
			
			// Reading server ACK for PORT
			n = read(sockfd, iobuff, 1024);
			if (n < 0) //check for errors
			{
				printf("ERROR: Could not read from socket");
				exit(1);
			}
			a++;
			printf("%s\n",iobuff);  //printing the PORT ACK received from server	    
	}*/
    
    if(strncmp(iobuff, "quit", 4) == 0)
    {
		bzero(iobuff,1024);
		strcpy(iobuff,"QUIT");
		n = write(sockfd, iobuff, strlen(iobuff));
		
		if (n < 0) //check for errors
		{
			printf("ERROR: Could not write to socket");
			exit(1);
		}
		bzero(iobuff,1024);
		n = read(sockfd, iobuff, 1024);
		if (strncmp(iobuff,"QUIT",4)==0)
		{	
			close(sockfd);
			printf("Connection Terminated from server\n");
			return 0;
		}
	}

	
	if( strncmp(iobuff, "ls", 2) == 0 )
		{
			char *waste;
			char *tok;
			//printf("%s",iobuff);
			//printf("checking");
			
			if(strcmp(iobuff,"ls\n")==0)
			{
				printf("The correct syntax is ls <argument>");
				return 1;
			}
			
			//changing ls to LIST
			strcpy( message, iobuff );
			
			waste = strtok(message, " ");
			tok = strtok(NULL,"\n");
			
			strcpy(iobuff, "LIST ");
			strcat(iobuff,tok);
			
			//sending the LIST command
			n = write(sockfd, iobuff, strlen(iobuff));
		   
			if (n < 0) //check for errors
			{
				printf("ERROR: Could not write to socket");
				exit(1);
			}
			printf("sent ls command\n");
				//Empty buffer for recev ack
			bzero(iobuff,1024);
			
			// Reading server ACK for ls
			n = read(sockfd, iobuff, 1024);
			if (n < 0) //check for errors
			{
				printf("ERROR: Could not read from socket");
				exit(1);
			}
			
			printf("%s\n",iobuff);  //printing the PORT ACK received from server

			wait_for_ls_data(dataport);
		}
			
		if( strncmp(iobuff, "get", 3) == 0)
		{
			strcpy( message, iobuff );
			if(strcmp(iobuff,"get\n")==0)
			{
				printf("The correct syntax is: get <filename>\n");
				return 1;
			}

			char *waste;
			char *tok;

				//changing get to RETR
			waste = strtok(message, " ");
			tok = strtok(NULL,"\n");
			
			strcpy(iobuff, "RETR ");
			strcat(iobuff,tok);
			
			//sending the RETR command
			n = write(sockfd, iobuff, strlen(iobuff));
		   
			if (n < 0) //check for errors
			{
				printf("ERROR: Could not write to socket");
				exit(1);
			}
			printf("sent get command\n");
				//Empty buffer for recev ack
			bzero(iobuff,1024);
			
			// Reading server ACK for get
			n = read(sockfd, iobuff, 1024);
			if (n < 0) //check for errors
			{
				printf("ERROR: Could not read from socket");
				exit(1);
			}
			
			printf("%s\n",iobuff);  //printing the PORT ACK received from server

			wait_for_get_data(dataport);
			
		}
	
		
		if( strncmp(iobuff, "put", 3) == 0)
		{
			strcpy( message, iobuff );
			if(strcmp(iobuff,"put\n")==0)
			{
				printf("The correct syntax is: put <filename>\n");
				return 1;
			}

			char *waste;
			char *tok;

				//changing get to RETR
			waste = strtok(message, " ");
			tok = strtok(NULL,"\n"); //contains filename
			
			strcpy(iobuff, "STOR ");
			strcat(iobuff,tok); // create STOR filename
			strcpy(message,tok);//copy the filename
			
			//sending the STOR command
			n = write(sockfd, iobuff, strlen(iobuff));
		   
			if (n < 0) //check for errors
			{
				printf("ERROR: Could not write to socket");
				exit(1);
			}
			printf("sent put command\n");
				//Empty buffer for recev ack
			bzero(iobuff,1024);
			
			// Reading server ACK for put
			n = read(sockfd, iobuff, 1024);
			if (n < 0) //check for errors
			{
				printf("ERROR: Could not read from socket");
				exit(1);
			}
			
			printf("%s\n",iobuff);  //printing the PORT ACK received from server

			send_put_file(dataport,message);
		}
	
	printf("End of %d excec command.\n", instcount++);
    return 1;
}



void main(int argc, char *argv[])
{
	int exitconn = 1;
	if (argc != 3) 
	{
      printf("usage %s <server IP> <server portnumber> \n", argv[0]); //Checking if the user input is in the valid format
      exit(0); 
   	}
   	
    sockfd = estab_control_channel(argc, argv); //call to establish a connection
    
    //sending port command
    
    strcpy(message,"PORT 127,0,0,1,10,5"); //this will create port number 2565

	n = write(sockfd, message, strlen(message));
   
	if (n < 0) //check for errors
	{
		printf("ERROR: Could not write to socket");
		exit(1);
	}

	//empty the buffer before reading the server output
	bzero(iobuff,1024);
	
	// Reading server ACK for PORT
	n = read(sockfd, iobuff, 1024);
	if (n < 0) //check for errors
	{
		printf("ERROR: Could not read from socket");
		exit(1);
	}
	printf("PORT command sent. %s\n",iobuff);  //printing the PORT ACK received from server

    
  	// while an exit indication is not received, continue to process client commands	
    while(exitconn)
    	exitconn = transferdata(sockfd);
}

