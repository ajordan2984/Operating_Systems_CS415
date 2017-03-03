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
};

args* parser(char*);  // Parses arguments and stores them inside an args struct
char* S2C(string,int);// Converts a string to a cstring, returns a char* to be stored in args
void sig_handler (int);// Catches ctrl-C and calls quit_process()
void quit_process();  // Terminates current process with kill(pid,SIGTERM)

char *id;
int fd;
int pipeget;

int main(void)
{
	args *aptr;
	srand((int)time(0));

	// Create random ID for client from 10000 - 19999
	int rnd_id = 10000 + rand()%9999;
	string string_id = std::to_string(rnd_id);
	id = S2C(string_id,string_id.size()); // Store client id
	cout<<"Client:"<<id<<" starting up."<<endl;

	// Create fifo for receiving output from server
	string temppath = "/tmp/"+std::string(id);
	cout<< temppath<<" is my new path for receiving output."<<endl;
	const char *charpath = S2C(temppath,temppath.size()); // convert string path to char*

	//Open fifo for receving output
	int code = mkfifo(charpath, 0666);
	// Error of fifo is already open
	if (code == -1)
	 perror ("mkfifo returned an error for receving - file may already exist");


	// Open pipe for receiving
	pipeget = open(charpath,O_RDWR);
	if (pipeget == -1)
		{
			perror ("Cannot open fifo");
			return EXIT_FAILURE;
		}
	// Create buffer to hold input from client
	char input[1500];
	//fflush(stdin); // flush buffer of any left over characters
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
		for (int i=0;i<1500;i++)
				input[i]= '\0';

		fgets(input,1500,stdin);
		aptr = parser(input);

		int max = aptr->argc; // max number of arguments sent to server
		char L = (char)max; // Convert number of arguments to char

		write(fd,id,strlen(id)); // Send number to server of client
		write(fd,&L,1); // Send number to server so it knows how many are coming
		for (int i=0;i<max;i++)
			{
				L = (char)strlen(aptr->argv[i]);
				write(fd,&L,1);
				write(fd,aptr->argv[i],strlen(aptr->argv[i])); // Send string characters
			}

		char temp;
		while (true)
		{
			read(pipeget,&temp,1); // Read size of command
			if (temp == '\0')
				break;
			cout<<temp;
		}
		cout<<"____________________________"<<endl;
		cout<<"Command returned successful."<<endl;
		cout<<"____________________________"<<endl;
	}// end infinite while

	return EXIT_SUCCESS;
	return 0;
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
