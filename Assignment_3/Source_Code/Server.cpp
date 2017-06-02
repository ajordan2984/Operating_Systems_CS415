//============================================================================
// Name        : source.cpp
// Author      : Andrew Jordan
// Version     : 5.0
// Institution : CS415 Athens State University
// Instructor  : Dr.Adam Lewis
// Description : Server simulation using: Pthreads|Semaphores|Pipes|Shared Memory
// command line build with proper flags: g++ source.cpp -o server.out -lpthread
// start program from command line: ./server.out
//============================================================================
/*
Server Design:
Start Client
	- Open named pipe to server with preset name "/tmp/demo_fifo" -> Read only permission.

|Executing User Input|
- Listen on named pipe for client ID.
- Read client ID once it is sent into the named pipe.
- Store client ID into vector of known client IDS on server.
- Start thread for the work of receive/parse/execute/send of client commands.
- Open shared memory between server/client based on client ID.
- **Design change:
		-   Server contains the parser because if a command from a client request input redirection from
			a file, the server must be able to parse the command from a file, execute it appropriately,
			and send the results back to the client. The parsing of input commands from the client
			have been reassigned to the server to eliminate unneeded duplication of both client and
			server parsing the same commands,speed executing commands, and set assigned roles.
	**
- Acess semaphore between server/client based on client ID.
- Lock semaphore.
- Read shared memory between server/client and save commands.
- Unlock semaphore for client writing to shared memory later on.
- Open named pipe back to client based on clients ID.
- Redirect STD out to named pipe back to client.
- Parse commands -> fork() -> execute -> output is automatically sent back to client.
- if command is cd/pwd -> change dir -> send present working directory back to client.
- Send char NULL back to client to signal output is finished.
- Redirect STD out back to server console.
- exit sending.
- Once command has been sent, Server listens to named pipe "/tmp/demo_fifo" for next client.
- Repeat server/client |receive clint ID/parse/execute/send output| until exit_ is sent.
- exit_ command sends signal to the server to delete the client id who sent it from the known 
	client list and the server sends back a char $ that signals the client that the server is 
	disconnected and then the client terminates.
- Server exits once all clients have left or maximum threads have been used.
*/
#include <iostream>
#include <sys/wait.h>
#include <dirent.h>
#include <string>
#include <ctype.h>
#include <algorithm>
#include <string.h>
#include <signal.h>
#include <queue>
#include <signal.h>

// Used in fifo
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
#include <pthread.h>
#include <stdlib.h>
using namespace std;

struct args
{
	int argc;
	char **argv;
	char *id;
	char *connection;
	sem_t *sem_id;
	void *shdmem;
};

struct client
{
	char *id;
	char *connection;
	sem_t *sem_id;
	void *shdmem;
};

void *branch(void*);			// Start commands for each new thread
void restore_output(int,int);	// Restore output to server
void set_connection(client*);	// Set std out to client
void set_semaphore(client*);	// Set semaphore for client to read shared memory
args* receive(client*);			// Read shared memory from client
int output(args*,int);			// Redirect output to specific file
int append(args*,int);			// Append output to specific file
int input(args*,int);			// Redirect std in from file

args* parser(char*);  // Parses arguments and stores them inside an args struct
char* S2C(string,int);// Converts a string to a cstring, returns a char* to be stored in args
bool dir(args*);	  // Changes and displays directories
void USER_PWD();	  // Displays current working directory
void sig_handler (int);// Catches ctrl-C and calls quit_process()
void quit_process();  // Terminates current process with kill(pid,SIGTERM)

void add_client(args*);		// Adds client to list of users on server
void remove_client(args*);	// Removes client from list of users active

int from_client;
std::vector<int> clients;

int main(void)
{
	pthread_t SERVERpool[20]; //Number of threads used
	int pid = 0;

	puts (">> |Server is opening connection.|");
		//Open fifo for write
	int code = mkfifo("/tmp/demo_fifo", 0666);
		// Error of fifo is already open
	if (code == -1)
		  perror ("mkfifo returned an error - file may already exist");
	puts(">> |Connection to /tmp/demo_fifo is now open.|");

	// Open a temp file for the server to receive connections
	from_client = open("/tmp/demo_fifo", O_RDONLY);

	// Fifo not opended, exit
			if (from_client ==-1)
				{
					perror("Cannot open FIFO for read");
					return EXIT_FAILURE;
				}

		while (true)
			{
				client *client1 = new client;
				char *idbuff = new char[6];
				idbuff[5] = '\0';

				// Read information
				read (from_client,idbuff,5);		// Read and store id of client
				client1->id = idbuff; 				// Save client id in struct
				set_connection(client1); 			// Set connection back to client
				set_semaphore(client1);				// Set semaphore between processes
				args *aptr = receive(client1); 		// Read and store commands from client

				// Copy arguments over
				aptr->connection = client1->connection;
				aptr->id = client1->id;
				aptr->sem_id = client1->sem_id;
				aptr->shdmem = client1->shdmem;
				add_client(aptr);					// Adds client to list of active users

				// Begin work
				pthread_create(&SERVERpool[pid],NULL,&branch,aptr); //Send thread to do work
				pthread_join(SERVERpool[pid],NULL);
				pid++;												// Mark thread as used
				cout<<">> |Server executed successfully.|"<<endl;
				cout<<">>"<<endl;

				// Quit if all clients are off OR no more threads
				if (clients.size()==0 || pid == 20)
						quit_process();
			}//end infinite while
	}//end main


void *branch(void *data_in)
{
	args *aptr = (args*)data_in;
	cout<<">> Server executing command from:"<<aptr->id<<endl;
	/* Steps:
		 * A. Save stdout file descriptor
		 * B. Open connection to client
		 * C. Redirect stdout to client
		 * D. Once cd or child finishes, redirect output to stdout
	*/
	bool CDcmd = true; // Save if command was cd or execvp
	bool exitcond = false;
	int save_out = dup(STDOUT_FILENO);// Save file descriptor of stdout
	int newfd = open(aptr->connection, O_WRONLY);// Open fifo to send back output
	fflush(stdout);			// Clear out std output
	close(STDOUT_FILENO);	// Close std output
	dup2(newfd, STDOUT_FILENO);// Redirect output to fifo client
	// Variables for error checking
	int checkEXEC = 0;
	int m = 0;

	// Check if exit command
	if (aptr->argv[0]== std::string("exit_"))
		{
			cout<<'$'<<endl; // Send exit signal to client
			restore_output(newfd,save_out); // Restore output
			remove_client(aptr);
			exitcond = true;
		}

	// Check if command changes directories
	CDcmd = dir(aptr);
	// If command changed directory then restore std output
	if(CDcmd)
		{
			restore_output(newfd,save_out);
			pthread_exit(NULL);
		}
	// If no cd command -> fork() then execvp()
	if (!CDcmd && !exitcond)
	{
		/* Fork here and create child process
			 * MYID > 0 : In parrent
			 * MYID == 0 :In child
			 * MYID == -1:Error
		*/
		pid_t MYID = fork();
		if (MYID > 0)
			{
				if (wait(0)==-1)
				perror("wait");
				restore_output(newfd,save_out); // Restore output
				cout<<">> |Parent process "<<MYID<<" continuing|"<<endl;
				cout<<">> |Child process exiting|"<<endl;
				kill(MYID,SIGTERM);
			}

		if(MYID == -1)
			{
				perror ("fork");
				exit(1);
			}
		if (MYID == 0)
			{
				cout<<">> |Child on server executing|"<<endl;
				for (int i = 0; i < aptr->argc; i++)
					{
						if (aptr->argv[i] == std::string(">") ||
						aptr->argv[i] == std::string("<") ||
						aptr->argv[i] == std::string("<<") ||
						aptr->argv[i] == std::string(">>"))
						{	m = i;	}
					}//end for
					if (aptr->argv[m] == std::string(">"))
						checkEXEC = output(aptr,m);

					if (aptr->argv[m] == std::string(">>"))
						checkEXEC = append(aptr,m);

					if (aptr->argv[m] == std::string("<"))
						checkEXEC = input(aptr,m);

					if (m == 0)
						checkEXEC = execvp(aptr->argv[0], aptr->argv);

					if (checkEXEC == -1)
						perror("exec");
				}//if MYID == 0
		}//else not cd command or exit command
	pthread_exit(NULL);
}//end branch
void restore_output(int newfd, int save_out)
{
	char L = '\0';
	write(newfd,&L,1);// Disconnect client by sending null
	fflush(stdout);	// Clear out std output
	close(newfd);	// Close redirected output
	dup2(save_out,STDOUT_FILENO);// Restore output to console
}
void set_connection(client *aptr)
{
	string temppath = "/tmp/"+std::string(aptr->id); // Make string of path to client
	aptr->connection = S2C(temppath,temppath.size()); // convert string path to char*
}
void set_semaphore(client *aptr)
{
	// Create semaphore name
		string temppath = "/sem-"+std::string(aptr->id);
		const char *sempath = S2C(temppath,temppath.size()); // convert string path to char*
		// Create semaphore on client
		aptr->sem_id = sem_open(sempath,O_CREAT,0660,0);
		if (aptr->sem_id == SEM_FAILED)
		{
			perror("sem_open");
			exit(1);
		}
}
args* receive(client *client1)
{
	int id_connection = atoi(client1->id);
	int shared_size = 1000;
	int counter = 0;
	string temp="";

	// Used in opening connection
	int shmid;
	key_t key = id_connection;
	void *sharedptr;
	void *startptr;

	// Create shared memory
	shmid = shmget(key,shared_size,IPC_CREAT|O_RDONLY);
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
		client1->shdmem = sharedptr;
	// Lock semaphore
		if (sem_wait(client1->sem_id) == -1)
				perror("sem_wait: sem_id");
			// Grab length of argument
		counter = *(int*)startptr;
			// Bump pointer
		startptr += sizeof(int);
			// Set char to start of argument
		char *a = (char*)startptr;
			// Build argument string
		for (int i=0;i<counter;i++)
			temp+=*(a+i);

			// Convert string to char*
		char *newarg = S2C(temp,temp.size());
		cout<<">> Server received: "<<newarg<<" from client:"<<id_connection<<endl;
		return parser(newarg);
}// end receive()
int output(args *aptr,int m)
{
	int newfd = open(aptr->argv[m + 1],O_CREAT|O_WRONLY|O_TRUNC, 0644);
	close(STDOUT_FILENO);
	dup2(newfd, 1);
	aptr->argv[m] = NULL;
	return execvp(aptr->argv[0], aptr->argv);
}
int append(args *aptr,int m)
{
	int newfd = open(aptr->argv[m + 1],
	O_CREAT|O_WRONLY|O_APPEND, 0644);
	close(STDOUT_FILENO);
	dup2(newfd, 1);
	aptr->argv[m] = NULL;
	return execvp(aptr->argv[0], aptr->argv);
}
int input (args *aptr,int m)
{
	char buffer[1000];
		for(int i = 0; i < 1000; i++)
			buffer[i]= '\0';
	auto newID = open(aptr->argv[m+1], O_CREAT|O_RDONLY, 0644);
	if(read(newID, buffer, 1000) == -1)
		exit(-1);
// Parse commands from file and update args
	args *temp = parser(buffer);
	args *newarguments = new args;
	newarguments->argv = new char* [(temp->argc)+2];
	newarguments->argv[0] = aptr->argv[m -1];
		for (int i=1;i< (temp->argc)+1;i++)
			newarguments->argv[i] = temp->argv[i-1];

	newarguments->argv[(temp->argc)+1] = '\0';
	//execute new args
	return execvp(newarguments->argv[0], newarguments->argv);
}
args* parser(char* argv)
{
	args *arguments = new args;
	string segment = "";
	int segment_size = 0;
	int i = 0;
	char *GGRTR = new char[3]{'>', '>', '\0'};
	char *LLESS = new char[3]{'<', '<', '\0'};
	char *GRTR = new char[2]{'>', '\0'};
	char *LESS = new char[2]{'<', '\0'};
	bool store_command = false;
	queue<char*> Qcommands;

	while (argv[i] != '\0')
	{
		while ((argv[i] != ' ')
			&& (argv[i] != '\n')
			&& (argv[i] != '\r')
			&& (argv[i] != '\t')
			&& (argv[i] != '>')
			&& (argv[i] != '<'))
		{
			segment += argv[i];
			segment_size++;
			i++;
			store_command = true;
		}
		if (store_command)
		{
			Qcommands.push(S2C(segment, segment_size));
			segment = "";
			segment_size = 0;
			store_command = false;
		}

		if (argv[i] == '>')
		{
			int double_check = i;
			if (argv[double_check + 1] == '>')
			{
				Qcommands.push(GGRTR);
				i++;
			}
			if (argv[double_check + 1] != '>')
				Qcommands.push(GRTR);
		}//end if >

		if (argv[i] == '<')
		{
			int double_check = i;
			if (argv[double_check + 1] == '<')
			{
				Qcommands.push(LLESS);
				i++;
			}
			if (argv[double_check + 1] != '<')
				Qcommands.push(LESS);
		}//end if <
		i++;
	}//end while

	//Allocate memory for arguments inside of struct
	int counter = Qcommands.size();
	arguments->argc = counter;
	arguments->argv = new char*[counter+1];

	// Transfer commands into struct
	for (int i = 0; i < counter; i++,Qcommands.pop())
		arguments->argv[i] = Qcommands.front();

	arguments->argv[counter] = '\0';
	//return pointer containing arguments
	return arguments;
}
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
bool dir(args *cmdptr)
{
	int changedir_test = 0;
	bool temp = false;
	// cycle through commands
	for (int i =0;i<cmdptr->argc;i++)
	{
		if (cmdptr->argv[i]==std::string("cd"))
		{
			if (cmdptr->argv[i+1]=='\0')
				changedir_test = 1;
			else if (cmdptr->argv[i+1]==std::string(".."))
				 changedir_test = chdir("..");
			else if (cmdptr->argv[i+1]==std::string("../.."))
				 changedir_test = chdir("../..");
			else if (cmdptr->argv[i+1]!='\0')
				changedir_test = chdir(cmdptr->argv[i+1]);

			if (changedir_test == -1)
				cout<<">>Error:Did not change directories."<<endl;
			if (changedir_test != -1)
				{
					USER_PWD();
					return true;
				}
		}// if cd

		if (cmdptr->argv[i]==std::string("pwd"))
			{
				USER_PWD();
				return true;
			}
	}// end for
return temp;
}// end non_exe
void USER_PWD()
{
	char buffer[200];
	char *newpath = getcwd(buffer,200);
	string currpath = newpath;
	cout<<">>"<<currpath<<endl;
}
void sig_handler (int sig)
{
	if (sig == SIGINT)
		quit_process();
}
void quit_process()
{
	close (from_client);
	cout<<"\nExiting.."<<endl;
	pid_t myid = getpid();
	kill (myid,SIGTERM);
	exit(0);
}
void add_client(args *aptr)
{
	bool add_client = true;
	int id = atoi(aptr->id);
	for (unsigned int i = 0;i<clients.size();i++)
		if (clients[i]==id)
			add_client = false;

	if (add_client)
		clients.push_back(id);
}
void remove_client (args *aptr)
{
	int id = atoi(aptr->id);
	if(shmdt(aptr->shdmem) < 0)
	cout<<"|Error: detaching shared memory|"<<endl;

	cout<<">> |Client:"<<id<<" Exiting.|"<<endl;
	for (unsigned int i =0;i<clients.size();i++)
			if (clients[i]==id)
			{
				clients.erase(clients.begin()+i);
				break;
			}
	cout<<">> | "<<clients.size()<<" online.|"<<endl;
}
