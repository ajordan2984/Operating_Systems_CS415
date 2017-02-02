/*
 * main.cpp
 *
 *  Created on: Jan 26, 2017
 *      Author: Andrew Jordan
 *       		Lucas Pruitt
 *       		Adam Moses
 *
 * Purpose: Simulate bash shell
 * Design:
 * If input from shell calling program
 * - copy arguments,fork,execute commands
 * If no input from shell calling program:
 * - Grab line of input
 * - Store in cstring
 * - Parse cstring
 * 		- convert each individual command to string
 * 		- use length of string to allocate new memory for char[]
 * 		- put in Q of commands converted
 * 		- use size of Q to allocate new memory for struct char[]
 * 		- transfer all commands to the struct char[] from Q
 * 		- add any > || >> || < || <<
 * 	- Check if directory change command
 * 	- If not changing directories
 *		- fork
 *		- have child check out I/O redirect and do so
 *		- have child process execute command
 *		- kill child process off
 *		- return to parent process
 *	- If exit or ctrl-C pressed, exit process
 *	- Repeat all steps above in infinite while in main.
 */

#include <iostream>
#include <unistd.h>
// used with chdir() -> changes working directory
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <dirent.h>
#include <string>
#include <ctype.h>
#include <algorithm>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <queue>
#include <signal.h>
using namespace std;


struct args
{
	int argc;
	char **argv;
};

args* parser(char*);  // Parses arguments and stores them inside an args struct
void order(char*);	  // Sets up order of functions called for reuse if needed
char* S2C(string,int);// Converts a string to a cstring, returns a char* to be stored in args
bool dir(args*);	  // Changes and displays directories
void USER_PWD();	  // Displays current working directory
void fork_off(args *);// Creates child process to do work and child is terminated with kill(pid,SIGTERM);
void sig_handler (int);// Catches ctrl-C and calls quit_process()
void quit_process();  // Terminates current process with kill(pid,SIGTERM)


int main(int argc,char *argv[])
{
	if (argc>1)
	{
		args *aptr = new args;
		aptr->argc = argc;
		aptr->argv = new char*[argc+1];
		for (int i=1;i<argc;i++)
			aptr->argv[i-1]=argv[i];
		aptr->argv[aptr->argc] = '\0';
		fork_off(aptr);
	}

	char input[1500];
	while (true)
	{
		if (signal(SIGINT,sig_handler)== SIG_ERR)
		{
			cout<<"Signal not caught"<<endl;
		}
		for (int i=0;i<1500;i++)
				input[i]= '\0';


		fgets(input,1500,stdin);
		order(input);
	}// end infinite while
	return 0;
}
void order(char *input)
{
	args *aptr;
	aptr = parser(input);
	fork_off(aptr);
}
void fork_off(args *aptr)
{
	bool runexec = false;
	int checkEXEC;
	if (aptr->argv[0]== std::string("exit_"))
		quit_process();

		runexec = dir(aptr);
			if (!runexec)
			{
				pid_t REpid = fork(); // returned pid
						switch (REpid)
						{
							case -1:
								perror ("fork");
								exit(1);
								break;
							case 0:
							{
								cout<<">>Child process:"<<REpid<<" starting up.."<<endl;

								int m = 0;
								for (int i = 0; i < aptr->argc; i++)
								  {
								    if (aptr->argv[i] == std::string(">") ||
									aptr->argv[i] == std::string("<") ||
									aptr->argv[i] == std::string("<<") ||
									aptr->argv[i] == std::string(">>"))
								      {	m = i;	}

								  }
								if (aptr->argv[m] == std::string(">"))
								  {

										int newfd = open(aptr->argv[m + 1],O_CREAT
											|O_WRONLY|O_TRUNC, 0644);
								    	close(STDOUT_FILENO);
								    	dup2(newfd, 1);
								    	aptr->argv[m] = NULL;
								    	checkEXEC = execvp(aptr->argv[0], aptr->argv);
								  }
								if (aptr->argv[m] == std::string(">>"))
								  {
								    	int newfd = open(aptr->argv[m + 1],
								    			O_CREAT|O_WRONLY|O_APPEND, 0644);
								    	close(STDOUT_FILENO);
								    	dup2(newfd, 1);
								    	aptr->argv[m] = NULL;
								    	checkEXEC = execvp(aptr->argv[0], aptr->argv);
								  }
								if (aptr->argv[m] == std::string("<"))
								  {
										char buffer[1000];
								    	auto newID = open(aptr->argv[m+1], O_CREAT
								    		|O_RDONLY, 0644);

								    	if(read(newID, buffer, 1000) == -1)
								    		exit(-1);

								    for(int i = 0; i < 1000; i++)
								    	if(buffer[i] == '\n' ||buffer[i] == '\t'||buffer[i] == '\r'|| buffer[i] == '\a')
								    		buffer[i] = ' ';

								    // Set last place to NULL
								    buffer[999] = '\0';
								    aptr->argv[m] = buffer;
								    aptr->argv[m + 1] = NULL;
								    execvp(aptr->argv[0], aptr->argv);
								  }

								else
								 checkEXEC = execvp (aptr->argv[0], aptr->argv);

								if (checkEXEC == -1)
									perror("exec");
								kill (REpid,SIGTERM);
								break;
							}
							default:
								if (wait(0)==-1)
									perror("wait");
								cout<<">> Parent process "<<REpid<<" now continuing..:"<<endl;
								break;
						}//end switch
			}// if runexec
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
	int changedir_test;
	bool temp = false;
	// cycle through commands
	for (int i =0;i<cmdptr->argc;i++)
	{
		if (cmdptr->argv[i]==std::string("cd"))
		{
			if (cmdptr->argv[i+1]==std::string(".."))
				 changedir_test = chdir("..");
			if (cmdptr->argv[i+1]==std::string("../.."))
				 changedir_test = chdir("../..");

			if (changedir_test == -1)
				cout<<">>Error:Did not change directories."<<endl;
			else
				{
					USER_PWD();
					temp = true;
					break;
				}
		}// if cd

		if (cmdptr->argv[i]==std::string("pwd"))
			{
				USER_PWD();
				temp = true;
				break;
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
	cout<<"\nExiting.."<<endl;
	pid_t myid = getpid();
	kill (myid,SIGTERM);
	exit(0);
}
