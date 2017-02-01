/*
 * main.cpp
 *
 *  Created on: Jan 26, 2017
 *      Author: andrew
quick commands:
 g++ main.cpp -> compiles main when in directory
 .a/.out main.cpp -> only works when in directory of project
 ps -> shows working processes
 find help on topic : example - man fork() 
 execvp -> okay to use as its in the same family of exec commands
 chdir(*char) -> changes directory, save current directory and new directory
 ctrl + Z -> stop process
 kill %1 -> kills associated process by number
 */

/*
 * main.cpp
 *
 *  Created on: Jan 26, 2017
 *      Author: andrew
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
using namespace std;


struct args
{
	int argc;
	char **argv;
};

args* parser(char*);
char* s2c(string,int);
bool dir(args*);
void USER_PWD();
void IOchange(char *);


int main(void)
{
	char input[1500];
	bool runexec = false;
	int checkEXEC;

	while (true)
	{
		for (int i=0;i<1500;i++)
				input[i]= '\0';

		fgets(input,1500,stdin);
		args *aptr;
		aptr = parser(input);

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
							cout<<">>Child process:"<<REpid<<" starting up:"<<endl;
							int m = 0;
							for (int i = 0; i < aptr->argc; ++i)
							  {
							    if (aptr->argv[i] == std::string(">") ||
								aptr->argv[i] == std::string("<") ||
								aptr->argv[i] == std::string("<<") ||
								aptr->argv[i] == std::string(">>"))
							      {	m = i;	}
						
							  }
							if (aptr->argv[m] == string(">"))
							  {
							    int newfd = open(aptr->argv[m + 1],
							     O_CREAT|O_WRONLY|O_TRUNC, 00200);
							    close(STDOUT_FILENO);
							    dup2(newfd, 1);
							    aptr->argv[m] = NULL;
							    checkEXEC = execvp(aptr->argv[0], aptr->argv);
							  }
							else if (aptr->argv[m] == string(">>"))
							  {
							    int newfd = open(aptr->argv[m + 1],
								 O_CREAT|O_WRONLY|O_APPEND, 00200);
							    close(STDOUT_FILENO);
							    dup2(newfd, 1);
							    aptr->argv[m] = NULL;
							    checkEXEC = execvp(aptr->argv[0], aptr->argv);
							  }
							else if (aptr->argv[m] == string("<"))
							  {
							    int i = m;
							    /*
							      do
							      {
							      aptr->argv[i] = aptr->argv[i + 1];
							      ++i;
							      }while(aptr->argv[n] != NULL);
							    */
							    for (i = 0; i < aptr->argc; ++i)
							      cout << aptr->argv[i] << endl;
							    
							    char buffer[1000];
							    char* cp;
							    
							    int newID = open("readIn.txt", O_CREAT|O_RDONLY, 0000);
							    if(read(newID, buffer, 1000) == -1)
							      exit(-1);
							    for(int i = 0; i < 1000; ++i)
							      {
								if(buffer[i] == '\n')
								  {
								    buffer[i] = '\0';
								    break;
								  }
							      }
							    aptr->argv[m] = buffer;
							    aptr->argv[m + 1] = NULL;
							    
							    execvp(aptr->argv[0], aptr->argv);
							  }	
							else
							  {	checkEXEC = execvp (aptr->argv[0], aptr->argv);	}
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
	}// end infinite while
	return 0;
}

args* parser(char* argv)
{
	args *arguments = new args;
	int counter;
	int i=0;
	int IOcounter =0;
	string temp ="";

	char dg[] = {'>','>','\0'};
	char dl[] = {'<','<','\0'};
	char grtr[] ={ '>','\0'};
	char less[] = {'<','\0'};
	bool storelastcmd = false;

	// store commands in Q
	queue<char*> cmds;
	// grab individual commands with pointer
	char *cmdptr;


	// split cstring with strtok using delimeters
	cmdptr = strtok(argv," \n\t\r\a");
	cout<<">>";
	while (cmdptr != NULL)
	{
		cout<<cmdptr<<" ";
		int section = static_cast< int >(strlen(cmdptr));
		while (i < (section+1))
		{

			if (cmdptr[i]=='<')
			{
				if (temp != "")
				cmds.push(s2c(temp,IOcounter));

				if(cmdptr[i]=='<' && cmdptr[i+1]=='<')
				{
					cmds.push(dl);
					i+=2;
				}
				else
					{
						cmds.push(less);
						i++;
					}
			temp="";
			IOcounter=1;
			storelastcmd = true;
			}// end if <

			if (cmdptr[i]=='>')
			{
				if (temp != "")
				cmds.push(s2c(temp,IOcounter));
				if(cmdptr[i]=='>' && cmdptr[i+1]=='>')
				{
					cmds.push(dg);
					i+=2;
				}
				else
					{
						cmds.push(grtr);
						i++;
					}
			temp="";
			IOcounter=1;
			storelastcmd = true;
			}// end if >

			temp += cmdptr[i];
			IOcounter++;
			i++;
		}// end while looking for <,<<,>>,> symbols

		if (storelastcmd)
		 {
			 cmds.push(s2c(temp,IOcounter));
			 storelastcmd = false;
		 }
		 else
			 cmds.push(cmdptr);

		 cmdptr = strtok(NULL," \n\t\r\a");
	}
	cout<<endl;
	// store arguments plus 1 for a null argument
	counter = cmds.size();
	arguments->argc = counter;
	arguments->argv = new char*[counter+1];

	//move commands from q to char array
	for (int i=0;i<counter;i++)
		{
			arguments->argv[i] = cmds.front();
			cmds.pop();
		}

	cmds.empty();
	//Set last element equal to null to tell linux where to quit
	arguments->argv[counter] = '\0';
	//return pointer containing arguments
	return arguments;
}
char* s2c(string data,int length)
{
	char *p = new char[length+1];
	for (int i =0;i<length;i++)
	p[i]=data[i];

	p[length]='\0';
	return p;
}
bool dir(args *cmdptr)
{
	bool temp = false;
	// cycle through commands
	for (int i =0;i<cmdptr->argc;i++)
	{
		if (cmdptr->argv[i]==std::string("cd"))
		{
			int changedir_test = chdir(cmdptr->argv[i++]);
			if (changedir_test == -1)
				cout<<">>Did not change directories"<<endl;
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
		if (cmdptr->argv[i]==std::string(">"))
		{
			/*
			IOchange(cmdptr->argv[i+1]);
			temp = true;
			break;
			*/
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
