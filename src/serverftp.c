#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>

int sockfd, newsockfd, portnum, n, i, c;
int exitconn=1;
socklen_t client_len;
char iobuff[1024];
char message[1024];
char ip[500];
int dataport=0;
FILE *file_pointer;
struct hostent *host;
struct sockaddr_in server_addr, client_addr;


int estab_control_channel(int argc, char *argv[])
{
    if(exitconn==2) 
	{
		printf("Waiting for client \n");
		goto l2;
	}
	
    portnum = atoi(argv[1]); //convert the portnumber recevied to an integer
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //returns a socket descriptor; returns -1 on error

    if (sockfd < 0) {
        printf("ERROR: Could not open socket");
        exit(1);
    }
    
    printf("Waiting for client \n");

    // Initialize socket
    
    bzero((char *) &server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(portnum);

	//bind assign local protocol address to socket; returns 0 if successful
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        printf("ERROR: Could not bind server");
        exit(1);
    }
    //Awaiting client for further instructions via listen:
    l2: listen(sockfd, 5);
    client_len = sizeof(client_addr);

    //Accept connection request from client 
    newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
    if (newsockfd < 0) {
        printf("ERROR: Could not accept client connection");
        exit(1);
    }
	
    return newsockfd; //return the connected socket file descriptor
}

int estab_data_channel(int dataport)
{
	struct hostent *host;
	struct sockaddr_in srvr_addr;
	
	printf("ENTERED DATA CHANNEL FUNCTION\n");
	
	int sfd = socket(AF_INET, SOCK_STREAM, 0);	//This call returns a socket descriptor that you can use in later system calls or -1 on error.

    if (sfd < 0) 
    {		//Check for error
        printf("ERROR: Could not open socket \n");
        exit(1);
    }
    int port = dataport; //changing the portnumber to integer
    
    host = gethostbyname(ip);	/* The gethostbyname() function returns a structure of type hostent for
					//the given host name.  Here name is either a hostname or an IPv4 address in standard dot notation*/
					
	// check for errors
    if (host == NULL) {
        printf("ERROR: Input host not found.\n");
        exit(0);
    }

	// initialize client
    bzero((char *) &srvr_addr, sizeof(srvr_addr));	//initialize to zero to avoid garbage values

    srvr_addr.sin_family = AF_INET;
    srvr_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &srvr_addr.sin_addr);
    bcopy((char *)host->h_addr, (char *)&srvr_addr.sin_addr.s_addr, host->h_length);

    // Connecting to server-Request 
    if (connect(sfd, (struct sockaddr*)&srvr_addr, sizeof(srvr_addr)) < 0) 
    {
        printf("ERROR: Could not connect to data channel of client\n");
        exit(1);
    }
    printf("Connected to Client on DATA CHANNEL \n ");
    return sfd; // returning the file descriptor obtained for data transfer
}

int transferdata(int newsockfd)
{
	int count = 0;
	char *waste;
	char *token;
	int n5,n6=0;

    //Read the client's request using the obtained file descriptor
    bzero(iobuff,1024);
    n = read(newsockfd,iobuff,1024);
    
    //Check if the exit command is entered
    if (strncmp(iobuff,"QUIT",4)==0)
    {
		printf("The previous client QUIT.\n");
		   n = write(newsockfd,iobuff,1024); //pass the command back to client
       
	//close the connection with present client and return 2 indicating to wait for a new connection       
		if (n>=0) 
		{
			close(newsockfd);
			return 2;
		}
		else  //check for error
		{
			printf("ERROR: Can not write QUIT to socket");
			exit(1);
		}
		//printf("after closing");

	}

	//printf("%s\n",iobuff);

	if(strncmp(iobuff,"PORT", 4) == 0)
	{
		//printf("checking - %s\n",iobuff);
		waste = strtok(iobuff, " ");
		token = strtok(NULL, ",");
		while( token != NULL )
		{
			if(count==0)
		    {
		        strcpy(ip,token);
		        strcat(ip,".");
		    }
		    if (count ==1 || count ==2)
		    {
		        strcat(ip,token);
		        strcat(ip,".");
		    }
		    
		    if(count==3)
				strcat(ip,token);
		    
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
		printf("The portnumber for DATA CHANNEL is: %d\n",dataport);
		//printf("checking before IP\n");
		printf("The Client is present on IP: %s\n", ip);
		//printf("checking after IP\n");
		bzero(iobuff, 1024);
		strcpy(iobuff, "200 Command OK.");
		n = write(newsockfd,iobuff,1024);
		if (n < 0) //check for error
		{
			printf("ERROR: Could not send ACK for PORT line");
			exit(1);
		}
		
		
	}
	
	if(strncmp(iobuff,"LIST", 4) == 0)
	{
		strcpy(message,iobuff);
		strcpy(iobuff,"200 Command OK.");
		n = write(newsockfd,iobuff,1024);
		if (n < 0) //check for error
		{
			printf("ERROR: Could not send ACK for PORT line");
			exit(1);
		}
		//estab data channel
		sleep(1);
		int datasockfd = estab_data_channel(dataport);
		
		
		//change LIST to ls
		char *waste;
		char *tok;
		strcpy( iobuff, message );
		
		waste = strtok(iobuff, " ");
		tok = strtok(NULL,"\n");
		
		strcpy(message, "ls ");
		strcat(message,tok);
		
		//process the command
		file_pointer = popen(message, "r"); 
		if (file_pointer == NULL)
        printf("ERROR: could not process command");
	
		bzero(iobuff,500); // clear the buffer for writing new results
		
		//Copying character by character the whole file to the buffer
		//Had to do this since fgets stopped reading after it encountered a new line
		for (i = 0; i < 500; ++i)
		{
			c = getc(file_pointer);
			if (c == EOF)
			{	
				pclose(file_pointer);
				break;
			}
			else
			iobuff[i] = c;
		}
		
		// Send response back to client
		n = write(datasockfd,iobuff,500);

		if (n < 0) {
			printf("ERROR: could not write to socket");
			exit(1); //checking for errors
		}
		close(datasockfd);
		printf("Disconnected from DATA channel\n");
	}
	
	
	if(strncmp(iobuff,"RETR",4)==0)
	{
		strcpy(message,iobuff);
		strcpy(iobuff,"200 Command OK.");
		n = write(newsockfd,iobuff,1024);
		if (n < 0) //check for error
		{
			printf("ERROR: Could not send ACK for PORT line");
			exit(1);
		}
		//estab data channel
		sleep(1);
		int datasockfd = estab_data_channel(dataport);
		
		//Extract filename
		char *waste;
		char *tok;
		strcpy( iobuff, message );
		
		waste = strtok(iobuff, " ");
		tok = strtok(NULL,"\n"); //token contains filename
		
		if (access(tok, F_OK) != -1)
		{
			strcpy(iobuff, "OK");
			write(datasockfd, iobuff, strlen(iobuff));
			//SendFileOverSocket(socket, file_name);
			
					
			struct stat	obj;
			int	file_desc, file_size;

			stat(tok, &obj);
			file_desc = open(tok, O_RDONLY);
			file_size = obj.st_size;
			send(datasockfd, &file_size, sizeof(int), 0);
			sendfile(datasockfd, file_desc, NULL, file_size);
			
		}
		else 
		{
			// Requested file does not exist, notify the client
			strcpy(iobuff, "NO");
			write(datasockfd, iobuff, strlen(iobuff));
		}
		
		
	}


	if(strncmp(iobuff,"STOR",4)==0)
	{
		strcpy(message,iobuff);
		strcpy(iobuff,"200 Command OK.");
		n = write(newsockfd,iobuff,1024);
		if (n < 0) //check for error
		{
			printf("ERROR: Could not send ACK for PORT line");
			exit(1);
		}
		//estab data channel
		sleep(1);
		int datasockfd = estab_data_channel(dataport);
		
			//Empty buffer for recev data
		bzero(iobuff,1024);
		recv(datasockfd, iobuff, 2, 0);
		int file_desc, file_size;
		char *data;
		if (strcmp(iobuff, "OK") == 0) 
		{

			recv(datasockfd, &file_size, sizeof(int), 0);
			data = malloc(file_size);
			file_desc = open("put_output.txt", O_CREAT | O_EXCL | O_WRONLY, 0666);
			recv(datasockfd, data, file_size, 0);
			write(file_desc, data, file_size);
			close(file_desc);
			printf("FILE required received via port %d\n", dataport);
		}
		else
		{
			printf("No FILE found by client.\n");
		}

		//close(datasockfd);
	
	}

}


void main(int argc, char *argv[])
{
    	
	if (argc != 2)  //Checking if the user input is in the valid format
	{
      printf("usage %s port\n", argv[0]);
      exit(0);
   	}
   	
	l1:		newsockfd = estab_control_channel(argc, argv); //call to establish a connection
    		printf("Connected to Client \n");
	
    	// while an exit indication is not received, continue to process client commands	
	while(exitconn)
	{
		exitconn=transferdata(newsockfd);
		if (exitconn==2)
			goto l1; //if an exit indication is received, go to establishing a new connection
	}
}

