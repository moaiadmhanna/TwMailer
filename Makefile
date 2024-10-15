all: server.out client.out

# Rule to compile the server
server.out: ./server/twmailer-server.c
	gcc -Wall -O -o twmailer-server.out ./server/twmailer-server.c

# Rule to compile the client
client.out: ./client/twmailer-client.c
	gcc -Wall -O -o twmailer-client.out ./client/twmailer-client.c

# Clean up the compiled files
clean:
	rm -f twmailer-server.out twmailer-client.out
