%.o: %.c ../lib
	gcc -C -o $@ $< -I

ftpserver: serverftp.c
	gcc -Wall serverftp.c -o ftpserver
	mv ftpserver ../bin
	
ftpclient: serverftp.c
	gcc -Wall clientftp.c -o ftpclient
	mv ftpclient ../bin
	
clean:
	rm -f ../bin/ftpclient ../bin/ftpserver ../bin/*.o
	
