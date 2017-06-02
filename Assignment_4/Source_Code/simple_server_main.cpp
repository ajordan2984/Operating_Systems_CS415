//==============================================================
// Name        : simple_server_main.cpp
// Author      : Andrew Jordan
// Version     : 5.0
// Institution : CS415 Athens State University
// Instructor  : Dr.Adam Lewis
// Description : Server simulation using: Pthreads|Sockets|Ports
//===============================================================
/*
Outside sources used:
Linux Socket Programming In C++ by Rob Tougher: tldp.org/LDP/LG/issue74/tougher.html
Purpose of source: A working connection between processes that set the socket with 
the proper flags to allow data to be sent and received properly without loss of 
data as was the issue I ran into many times.
Changes to borrowed source: 
The string being sent to the client was converted to a cstring to avoid junk characters.                        
Buffer size was increased on the send and receive.
*
Server Design:
Start Client
	- Create server socket to preset port 1500.

|Executing User Input|
- Listen on socket for clients.
- Accept client and receive their ID.
- Store client ID into vector of known client IDS on server.
- Start thread for the work of processing/parse/execute/send of client commands.
- Create client socket between server/client based on client ID to send output back.
- *Design change:
Server contains the parser because if a command from a client request input redirection from
a file, the server must be able to parse the command from a file, execute it appropriately,
and send the results back to the client. The parsing of input commands from the client
have been reassigned to the server to eliminate unneeded duplication of both client and
server parsing the same commands,speed executing commands, and set assigned roles.
*
- Command:
	pwnme: tells the server to start executing commands.
	exit:  tells the server to resume echoing only data back to client.
	exit_: ends connection to the server.
|Sending output back|
- Start PTHREAD for each unique client 
- Create socket connection back to client with ID from client
- Redirect STD out to a pipe.
- Parse commands -> fork() -> execute ->*
- if command is cd/pwd -> change dir -> cout pwd ->*
- if just echoing, cout all strings from client ->*
-*-> flush cout -> write string from the pipe -> send string to client
- Send char $ back to client to signal output is finished.
- Redirect STD out back to server console.
- exit sending.
- Once command has been sent, Server listens to the unique socket for next command.
- Repeat server/client receive/parse/execute/send output until exit_ is sent.
- exit_ command sends signal to the server to delete the client id who sent it from the known 
	client list.
*/
#include <iostream>
#include "ServerSocket.h"
#include "SocketException.h"
#include "ClientSocket.h"
#include <string>
//
#include <unistd.h>
#include <ctype.h>
#include <queue>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
using namespace std;
struct client_args
{
	string id;
};
struct args
{
	int argc;
	char **argv;
	int id;
};
void *process_client(void*);				
void parser(char* argv, args *arguments);
char* S2C(string segment,int mysize);
std::string branch(args *aptr,bool ECHO);
std::string restore_output(int output_pipe[2],int savefd);
int output(args *aptr,int m);
int append(args *aptr,int m);
int input (args *aptr,int m);
bool dir(args *cmdptr);
void USER_PWD();
void sig_handler(int);
void quit_process();
void add_client(int);
void remove_client(int);
std::vector<int> clients;

int main ()
{
	int pid = 0;
	pthread_t SERVERpool[5]; //Number of threads used
	std::cout << "=> Server is starting on port 1500.\n";
  try
    {
      // Create the socket
      ServerSocket server ( 1500 );

      while ( true )
      {
    	  cout<<"=> Server is listening for new clients."<<endl;
    	  ServerSocket new_sock;
    	  server.accept ( new_sock );

    	  try
    	  {
	    	  if (signal(SIGINT,sig_handler)== SIG_ERR)
	    		  cout<<"Signal not caught"<<endl;

	    	  std::string data;
		  	  new_sock >> data;
		  	  cout<<"=> Client:"<<data<<" connecting."<<endl;
		  	  client_args *clientptr = new client_args;
		  	  clientptr->id = data;
		  	  sleep(10); // let the thread sleep so the client can bring output server online
		  	  pthread_create(&SERVERpool[pid],NULL,&process_client,clientptr); //Send thread to do work
		  	  pid++;

		  	  if(pid == 5)
		  		  break;

    	  }
    	  catch ( SocketException& ) {}
      }// end while server accepting clients
    }// end try connecting socket
  catch ( SocketException& e )
    {
      std::cout << "Exception was caught:" << e.description() << "\nExiting.\n";
    }

  cout<<"=> Server has reached its thread limit.Exiting."<<endl;
  for (int i=0;i<5;i++)
 	  		pthread_join(SERVERpool[pid],NULL);

  return 0;
}

void *process_client(void *dataptr)
//void process_client(string data_in)
{

	bool stopread = false;
	std::string emptyS ="";
	std::string part_data;
	std::string data ="";
	bool ECHO = true;
	char *buffer;
	client_args *cptr = (client_args*)dataptr;
	string data_in = cptr->id;
	std::string::size_type sz;
	int clientport = std::stoi(data_in,&sz);

	// Add client to server
	add_client(clientport);
	cout<<"=> Getting ready to process request from:"<<clientport<<endl;
	// Connect back to specific client
	try
	    {
	      ClientSocket client_socket ( "localhost", clientport );

	      while (true)
	    	 	{
		      	  try
		      	  {
		      		  while(true)
		      		  {
	    		    	client_socket >> part_data;

	    		    	for (unsigned int i=0;i<part_data.size();i++)
	    		    		{
	    		    			if (part_data[i]=='$')
	    		    				{
	    		    					stopread = true;
	    		    					break;
	    		    				}
	    		    			else
	    		    				data+= part_data[i];
	    		    		}
	    		    	if(stopread)
	    		    		break;
		      		  }// while receiving input from client
		      	  }// end try receiving input from client
		      	  catch ( SocketException& ) {}// catch bad input from client

	    		    cout<<"=> Server received:"<<data<<"| from client:"<<clientport<<endl;
	    	 		args *aptr = new args;
	    	 		aptr->id = clientport;
	    	 		buffer = S2C(data,data.size());
	    	 		parser(buffer,aptr);

	    	 		if (aptr->argv[0]== std::string("exit_"))
	    	 			{
	    	 				remove_client(clientport);
	    	 				pthread_exit(NULL);
	    	 			}
	    	 		// Check if exit executing commands
	    	 		if (aptr->argv[0]== std::string("exit"))
	    	 		 ECHO = true;

	    	 		// Check if only echoing command back
	    	 		if (aptr->argv[0]== std::string("pwnme"))
	    	 		 ECHO = false;

	    	 		try{
	    	 		client_socket << branch(aptr,ECHO);
	    	 		}
	    	 		catch ( SocketException& ) {}// catch bad output client

	    	 		cout<<"=> Data sent to:"<<aptr->id<<" successfully."<<endl;
	    	 		// Clean up
	    	 		data ="";
	    	 		std::string part_data;
	    	 		delete (aptr);
	    	 		delete(buffer);
	    	 	}// end while(true)
	    }// end try connect back to output server on client
	  catch ( SocketException& e )
	    {
	      std::cout << "Exception was caught:" << e.description() << "\n";
	    }
}// end process_client()
void parser(char* argv, args *arguments)
{
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
		while (
			(argv[i] != '\0')
			&&(argv[i] != ' ')
			&& (argv[i] != '\n')
			&& (argv[i] != '\r')
			&& (argv[i] != '\t')
			&& (argv[i] != '>')
			&& (argv[i] != '<')
			&& (argv[i] >= 0)
			&& (argv[i]< 128))

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

		if (argv[i]== '\0')
			break;
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
std::string branch(args *aptr,bool ECHO)
{
	cout<<"=> Server executing command from:"<<aptr->id<<endl;

	// Create pipe to store output from stdout
	int pipefd[2];
	   if (pipe(pipefd) == -1)
	        perror("pipe");

	int savefd = dup(STDOUT_FILENO);			// Save file descriptor of stdout
	fflush(stdout);					 			// Clear out std output
	close(STDOUT_FILENO);			  			// Close std output
	dup2(pipefd[1], STDOUT_FILENO);	    		// Redirect output to pipe
	bool CDcmd = true; 				  			// Save if command was cd or execvp
	// Variables for error checking
	int checkEXEC = 0;
	int m = 0;

	// if no PWNME, echo all commands back to client
	if (ECHO)
		{
			string tempString ="";
			int asize = aptr->argc;
			for (int i =0;i<asize;i++)
				tempString= tempString+aptr->argv[i]+ " ";
			//tempString+="$";
			cout<<tempString;
		}

	// Check if command changes directories
	if(!ECHO)
	CDcmd = dir(aptr);
	// If command changed directory then restore std output
	if(CDcmd || ECHO)
		return restore_output(pipefd,savefd);

	// If no cd command -> fork() then execvp()
	if (!CDcmd && !ECHO)
	{

		if (aptr->argv[0]== std::string("pwnme"))
		{
			cout<<"=> Server is now accepting commands from"<<aptr->id<<endl;
			return restore_output(pipefd,savefd);
		}
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
				return restore_output(pipefd,savefd);
				kill(MYID,SIGTERM);
			}

		if(MYID == -1)
			{
				perror ("fork");
				exit(1);
			}
		if (MYID == 0)
			{
				cout<<"=> Child on server executing."<<endl;
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
		}//end if not cd command or not just echo
}//end branch
std::string restore_output(int output_pipe[2], int savefd)
{

	std::string tempString = "";
	char tempc = '$';
	cout.flush();						// Flush all cout to pipe
	write(output_pipe[1], &tempc, 1);	// Write terminating character
	close(output_pipe[1]);				// Close writing end of pipe
	dup2(savefd,STDOUT_FILENO);			// Restore output to console
	fflush(stdout);						// Flush stdout buffer

	while (true)
	{
		read(output_pipe[0],&tempc,1);
		if (tempc == '$')
			break;
		tempString+=tempc;
	}

	tempString+="$";
	close(output_pipe[0]);
	return tempString;

}
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
	int newfd = open(aptr->argv[m + 1],O_CREAT|O_WRONLY|O_APPEND, 0644);
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
	int newID = open(aptr->argv[m+1], O_CREAT|O_RDONLY, 0644);
	if(read(newID, buffer, 1000) == -1)
		exit(-1);
// Parse commands from file and update args
	args *temp = new args;
	parser(buffer,temp);
	args *newarguments = new args;
	newarguments->argv = new char* [(temp->argc)+2];
	newarguments->argv[0] = aptr->argv[m -1];
		for (int i=1;i< (temp->argc)+1;i++)
			newarguments->argv[i] = temp->argv[i-1];

	newarguments->argv[(temp->argc)+1] = '\0';
	//execute new args
	return execvp(newarguments->argv[0], newarguments->argv);
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
				cout<<"=> Error:Did not change directories."<<endl;
			if (changedir_test != -1)
				{
					USER_PWD();
					return true;
				}
		}// end if cd

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
	cout<<"=> "<<newpath<<endl;
}

void sig_handler (int sig)
{
	if (sig == SIGINT)
		quit_process();
}

void quit_process()
{
	cout<<"\nExiting from server.."<<endl;
	pid_t myid = getpid();
	kill (myid,SIGTERM);
	exit(0);
}

void add_client(int id)
{
	bool add_client = true;
    int temp=0;

    for (unsigned int i = 0;i<clients.size();i++)
		{
			temp = clients[i];
			if (temp == id)
					add_client = false;
		}
	if (add_client)
		{
			cout<<"=> Adding new client:"<<id<<" |"<<endl;
			clients.push_back(id);
		}
}
void remove_client (int id)
{
	cout<<"=> Client:"<<id<<" Exiting.|"<<endl;
	int temp = 0;
	for (unsigned int i =0;i<clients.size();i++)
			{
				temp = clients[i];
				if (temp ==id)
				{
					clients.erase(clients.begin()+i);
					break;
				}// end if
			}// end for
	cout<<"=> "<<clients.size()<<" online.|"<<endl;
}
