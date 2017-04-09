/*
===========================================================================================================================
Name: Naishil Shah
cruzid: nanshah@ucsc.edu 
===========================================================================================================================


There are 3 directories(folders) in the submission. Following is the explanation of the file organization:

src - This contains the source files of server and client, named serverftp.c and clientftp.c respectively. Also contains
	  makefile.

bin - The directory contains the object and excecutables of the codes. There is a sample.txt file which can be use for demo
	  purposes for checking the functionality of GET and PUT command.

doc - Contains the README.txt file and Documentation.pdf file.

---------------------------------------------------------------------------------------------------------------------------

***Steps to run: 

Assuming you are in the submitted folder.

1. cd src/

2. make ftpserver

3. make ftpclient

4. cd ../bin/
Run the server with the following command:

5. ./myserver <portnum> 

Run a different terminal for the client with the following command:

6. ./myclient <server-ip> <server-listen-port>


***Description: 

1. The functionality of commands port, ls, get, put and quit works perfectly.
2. Two different channels - control and data, were used to provide funcationality.
3. All the error cases of missing files, invalid commands, syntax errors etc has been taken care of.
4. There is only one port command sent at the beginning of the execution. It could be also modified to be sent for each
   command according to the original protocol.

***ISSUES and PROBLEMS NOT TACKLED***

1. The abort command could not be implemented due to time constraints
2. There might be unnecesarry print statements which I felt were to be given to the user. They can be removed and I hope 
   not to loose many points for that.

Thank you for grading the assignment.

