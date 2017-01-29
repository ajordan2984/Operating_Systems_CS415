#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

using namespace std;

struct args
{
	int argc;
	char** argv;
};
// ^^ arguments for the exec() command

args* parser ( char* cp )
{
   	args* pass = new args;
	// ^^ we're passing this one back

	int iNum, argNum = 1;

   	char *bgn = cp;
	// ^^ so we can reset cp

    	for ( ; *cp != '\0' ; ++cp )
    	{
       		if ( *cp == ' ' )
        	{
			// argNum was original set to 1 since it only increments
			// at the spaces between arguments (it would have missed
			// one otherwise)
        		++argNum;
        		*cp = '\0';
			// ^^ replace spaces in the original string with nulls
        	}
    	}

    	char **pp, **bp;
	// ^^ pp will point to the beginning of the char** (eventually)
	// but we need to iterate it across the arguments
	// so we have bp to reset it

    	pp = new char*[argNum + 1];
	// ^^ like before, one extra space for a null char
    	iNum = 0;
    	cp = bgn; // reset cp
    	bp = pp; //set bp

	for ( int i = 0 ; i < argNum ; ++i, ++pp)
    	{
        	for ( iNum = 0; *cp != '\0' ; ++cp, ++iNum )
		{
			// this increments the iNum counter
			// which counts how many characters
			// each of the new arguments has
		}

        	*pp = new char[iNum + 1];
		// ^^ use iNum to declare a new char string for each argument
		// the +1 is for the null character we put in at the end
		// of each argument in the previous section
        	cp = bgn;

       		for ( int j = 0 ; j < (iNum + 1) ; ++j, ++cp )
        	{
			(*pp)[j] = *cp;
			// while this is memory-reference-tastic...
			// it's the easiest way to make each of the
			// characters in the new char string equal
			// to the elements of the original string
		}

        	bgn = cp;
    	}
    	
	*pp = new char;
   	*pp = NULL;

    	pp = bp; // reset the pp pointer
    	pass->argc = argNum;
    	pass->argv = pp;

    	return pass;
}

int main ()
{
  	int in, out;
  	pid_t parent_pid, child_pid;
  	int status;

    	string str;
    	getline(cin, str); // dump the entire input string into a string class
    	int n = str.length();

    	char cstr[n + 1];
	// ^^ this will make splitting the arguments easier
	// also, it's one place longer so we can add a null char at the end

    	for (int i = 0; i < n; ++i)
	{
        	cstr[i] = str[i];
	}

    	cstr[n] = '\0';
    	args* pass;
    	pass = parser ( cstr );
	// ^^ passes in a cstring and returns an arg struct

    	if (pass->argv[2] == ">")
      	{
		// code for dealing with file redirection
      	}

	child_pid = fork();
  	if ( child_pid < 0 )
    	{
		cout << "fork failed" << endl;
      		exit(1);
    	}
  
  	if ( child_pid == 0)
    	{
      		execvp (pass->argv[0], pass->argv);
		// ^^ this seems to work for ls and programs in
		// the directory from which the parser program is called
		cout << "didn't work" << endl;
		// ^^ if this prints, the command wasn't executed
    	}
  	else
    	{
      		wait(&status);
    	}

  return 0;
}
