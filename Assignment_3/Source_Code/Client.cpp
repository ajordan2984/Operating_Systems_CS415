//============================================================================
// Name        : source.cpp
// Author      : Andrew Jordan
// Version     : 5.0
// Institution : CS415 Athens State University
// Instructor  : Dr.Adam Lewis
// Description : Client simulation using: Pthreads|Semaphores|Pipes|Shared Memory
// command line build with proper flags: g++ source.cpp -o client.out -lpthread
// start program from command line: ./client.out
//============================================================================
/*
Client Design:
Start Client
	- Generate random 5 digit ID for the client.
	- Open named pipe to server with preset name "/tmp/demo_fifo" -> Write only permission.
	- Open/Create named pipe with ID for receiving back output from server.
	- Open/Create semaphore with ID for locking and unlocking shared memory between client/server.
Begin User Input
- Prompt input from user.
- Grab input from command line and save it.
- Send client ID to the server over named pipe.
	- Client ID is sent each time a client sends a command to the server
	  so that the server knows who to send the output back to.
- **Design change:
		-   Server contains the parser because if a command from a client request input redirection from
			a file, the server must be able to parse the command from a file, execute it appropriately,
			and send the results back to the client. The parsing of input commands from the client
			have been reassigned to the server to eliminate unneeded duplication of both client and
			server parsing the same commands,speed executing commands, and set assigned roles.
	**
- Open/Create shared memory between client/server. 
	- Shared memory is created based on client ID.
- Lock semaphore between client/server.
- Copy client input to shared memory between client/server.
- Unlock semaphore for the server to be able to read the shared memory.
- exit sending.
- Once command has been sent, client listens to its named pipe, based on ID, to receive output from server.
- Output is displayed on the clients terminal.
- Repeat client/server |send ID/write to shared memory/receive output| until exit_ is sent.
- exit_ command sends signal to the server to delete its id from the known client list
	and the server sends back a char $ that signals the client that the server is 
	disconnected and then the client terminates.
*/
#include <iostream>
#include <sys/wait.h>
#include <dirent.h>
#include <string>
#include <ctype.h>
#include <algorithm>
#include <string.h>
#include <signal.h>
#include <signal.h>

// Used in named pipe
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
// Used in IPC
#include<sys/ipc.h>
#include<sys/shm.h>
// Used in semaphore
#include<semaphore.h>
using namespace std;

void send(char *,int);
char* S2C(string,int);// Converts a string to a cstring, returns a char*
void sig_handler (int);// Catches ctrl-C and calls quit_process()
void quit_process();  // Terminates current process with kill(pid,SIGTERM)

sem_t *sem_id;
char *id;
int fd;
int socket;

int main(void)
{
	srand((int)time(0));

	// Create random ID for client from 10000 - 19999
	int rnd_id = 10000 + rand()%9999;
	string string_id = std::to_string(rnd_id);
	id = S2C(string_id,string_id.size()); // Store client id
	cout<<"|Client:"<<id<<" starting up.|"<<endl;


	// Create fifo for receiving output from server
	string temppath = "/tmp/"+std::string(id);
	const char *charpath = S2C(temppath,temppath.size()); // convert string path to char*

	// Create semaphore name
	string SEM_NAME = "/sem-"+std::string(id);
	const char *sempath = S2C(SEM_NAME,SEM_NAME.size()); // convert string path to char*

	// Create semaphore on client
	sem_id = sem_open(sempath,O_CREAT,0660,0);
	if (sem_id == SEM_FAILED)
	{
		perror("sem_open");
		exit(1);
	}

	//Open fifo for receving output
	int code = mkfifo(charpath, 0666);
	// Error of fifo is already open
	if (code == -1)
	 perror ("mkfifo returned an error for receving - file may already exist");

	// Open pipe for receiving
	socket = open(charpath,O_RDWR);
	if (socket == -1)
		{
			perror ("Cannot open fifo");
			exit(1);
		}
	// Create buffer to hold input from client
	char input[500];

	//Open fifo for write
	fd = open("/tmp/demo_fifo", O_WRONLY);
	if (fd == -1)
	{
		perror ("Cannot open fifo");
		return EXIT_FAILURE;
	}

	while (true)
	{
		if (signal(SIGINT,sig_handler)== SIG_ERR)
		{
			cout<<"Signal not caught"<<endl;
		}
		for (int i=0;i<500;i++)
				input[i]= '\0';

		cout<<">> |Please enter a command to the server:|"<<endl;
		fgets(input,500,stdin);

		// Send id of client to the server
		write(fd,id,strlen(id));
		// Send arguments via shared memory
		send (input,rnd_id);
		// Wait for reply from server
		char temp;
		while (true)
		{
			read(socket,&temp,1); // Read size of command
			if (temp == '\0')
				break;
			if (temp == '$')
				quit_process();
			cout<<temp;
		}
		cout<<">> |Command returned successful.|"<<endl;
	}// end infinite while
	return 0;
}
void send(char *input,int id_connection)
{
	// Used in creating shared memory
	int shared_size = 1000;
	int shmid;
	key_t key = id_connection;
	void *sharedptr;
	void *startptr;
	// Create shared memory
		shmid = shmget(key,shared_size,IPC_CREAT|0666);
		if (shmid <0)
		 {
			 perror("shmget");
			 exit(1);
		 }
	// Attach shared memory
		sharedptr = shmat(shmid,NULL,0);
		if (sharedptr  == (char *)-1)
		{
			perror("shmat");
			exit(1);
		}
	// Set a pointer to the start of shared memory
		startptr = sharedptr;

	cout<<"Sending:"<<input<<endl;
		//Send length of c-string
	*(int*)startptr = strlen(input);
		//Bump pointer
	startptr += sizeof(int);
		//Copy c-string into shared memory
	strcpy((char*)startptr,input);
		//Unblock server
	if (sem_post(sem_id) == -1)
		perror("sem_post: sem_id");

}//end send()
char* S2C(string segment,int mysize)
{
	//grab new memory for cstring
	char *temp = new char[mysize+1];
	// transfer characters from string to new char array
	for (int i = 0; i < mysize; i++)
		temp[i] = segment[i];
	temp[mysize] = '\0';
	return temp;
}
void sig_handler (int sig)
{
	if (sig == SIGINT)
		quit_process();
}
void quit_process()
{
	close (socket);
	close (fd);
	cout<<"\nExiting.."<<endl;
	pid_t myid = getpid();
	kill (myid,SIGTERM);
	exit(0);
}
