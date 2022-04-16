# TCP Chat
## Compiling

#### To compile the server and the client on windows:
	> gcc .\server.c -o server.exe -lws2_32

#### To compile the server and the client on linux:
	> gcc .\server.c -o server.exe

## Running the application

#### First you have to start the server with the port that your application will run.
    > ./server.exe [port]
#### After that, you can create many clients to make a real chat. You will need past the machine ip of the server, the port which the server is running and your username for a better identification.
    > ./client.exe [ip] [port] [username]

