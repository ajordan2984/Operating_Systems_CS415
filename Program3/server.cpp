/*
 * main.cpp
 *
 *  Created on: March 1, 2017
 *      Author: Andrew Jordan
 *
 *
 * Purpose: Simulate Server
 */

#include <iostream>
// used with chdir() -> changes working directory
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
using namespace std;

struct args
{
	int argc;
	char **argv;
	char *id;
	char *connection;

};

void restore_output(int,int);
void set_connection(args*);
int output(args*,int);
int append(args*,int);
int input(args*,int);

args* parser(char*);  // Parses arguments and stores them inside an args struct
char* S2C(string,int);// Converts a string to a cstring, returns a char* to be stored in args
bool dir(args*);	  // Changes and displays directories
void USER_PWD();	  // Displays current working directory
void fork_off(args *);// Creates child process to do work and child is terminated with kill(pid,SIGTERM);
void sig_handler (int);// Catches ctrl-C and calls quit_process()
void quit_process();  // Terminates current process with kill(pid,SIGTERM)

int fd;

int main(void)
{

	puts ("Server - listening ");
	//Open fifo for write
	int code = mkfifo("/tmp/demo_fifo", 0666);
	// Error of fifo is already open
	if (code == -1)
	   perror ("mkfifo returned an error - file may already exist");
	puts("FIFO connection has opened");

	// Open fifo for reading
		fd = open("/tmp/demo_fifo", O_RDONLY);
	// Fifo not opended, exit
		if (fd ==-1)
			{
				perror("Cannot open FIFO for read");
				return EXIT_FAILURE;
			}

	while (true)
	{
			char *idbuff = new char[6];
			idbuff[5] = '\0';

		// Start work for finding args
			int counter = 0; // Number of arguments coming in
			char Len; // Length of arguments

			// Read information
			read (fd,idbuff,5);// Read and store id of client
			read (fd,&Len,1); // Read number of arguments coming in
			args *aptr = new args; // Create new pointer
			int maxargs = (int)Len;// Save number of arguments
			cout<<"max args:"<<maxargs<<endl;
			aptr ->argv = new char*[maxargs+1]; // Create new char* array to store arguments
			aptr ->argv[maxargs]= '\0'; // Set last place to null
			aptr ->argc = maxargs;
			aptr ->id = idbuff;
			set_connection(aptr);

		while (true)
		{
			if (counter == aptr->argc)
				break;

			read(fd,&Len,1); // Read size of command
			maxargs = (int)Len;// Save how many commands are coming in
			char *stringBuffer = new char[maxargs+1];
			memset(stringBuffer,0,maxargs);// Fill buffer with 0s

			read (fd,stringBuffer,maxargs); 	// Read command
			stringBuffer[maxargs] = '\0';		// Set last place as null
			aptr->argv[counter]= stringBuffer; 	// Save command in struct
			counter++; // bump counter

			cout<<"Server received: "<<stringBuffer<<" from client:"<<idbuff<<endl;
		}//end while read from pipe

		fork_off(aptr); // Fork off and start command
		cout<<"_____________________________"<<endl;
		cout<<"Server executed successfully."<<endl;
		cout<<"_____________________________"<<endl;
	}//end infinite while

}

void fork_off(args *aptr)
{
	cout<<"_____________________________"<<endl;
	cout<<"Server executing command.    "<<endl;
	cout<<"_____________________________"<<endl;

	/* Steps:
	 * A. Save stdout file descriptor
	 * B. Open connection to client
	 * C. Redirect stdout to client
	 * D. Once child finishes, redirect output to stdout
	 */
	int save_out = dup(STDOUT_FILENO);// Save file descriptor of stdout
	int newfd = open(aptr->connection, O_WRONLY);// Open fifo to send back output
	fflush(stdout);			// Clear out std output
	close(STDOUT_FILENO);	// Close std output
	dup2(newfd, STDOUT_FILENO);// Redirect output to fifo client

	int checkEXEC = 0;
	int m = 0;

	if (aptr->argv[0]== std::string("exit_"))
		{
			restore_output(newfd,save_out); // Restore output
			quit_process();
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
			restore_output(newfd,save_out); // Restore output
			cout<<">> Parent process "<<MYID<<" now continuing..:"<<endl;
			cout<<"Child process now dieing.."<<endl;
			kill(MYID,SIGTERM);
		}

	if(MYID == -1)
		{
			perror ("fork");
			exit(1);
		}

	if (MYID == 0)
		{
			cout<<">> Child process:"<<MYID<<" starting up.."<<endl;
			if (!dir(aptr))
			{
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
			}//if not cd or pwd command
		}//if MYID == 0

}// end fork off
void restore_output(int newfd, int save_out)
{
	char L = '\0';
	write(newfd,&L,1);// Disconnect client by sending null
	fflush(stdout);	// Clear out std output
	close(newfd);	// Close redirected output
	dup2(save_out,STDOUT_FILENO);// Restore output to console
}
void set_connection(args *aptr)
{
	string temppath = "/tmp/"+std::string(aptr->id); // Make string of path to client
	cout<< temppath<<" is my new path."<<endl;
	aptr->connection = S2C(temppath,temppath.size()); // convert string path to char*
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
	close (fd);
	cout<<"\nExiting.."<<endl;
	pid_t myid = getpid();
	kill (myid,SIGTERM);
	exit(0);
}
