//=======================================================
// Name        : simple_client_main.cpp
// Author      : Andrew Jordan
// Version     : 5.0
// Institution : CS415 Athens State University
// Instructor  : Dr.Adam Lewis
// Description : Client simulation using: Sockets|Ports
//======================================================
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
Client Design:
Start Client
	- Generate random 4 digit ID for the client.
	- Create socket to predefined port 1500 that the server is listening on. 
	- Send ID to server so that the server can send output back to the client.
Begin User Input
- Prompt input from user.
- Grab input from command line and save it.
- Create server on the client side to listen for output from the executing server.
- Client ID is used by the executing server to connect back to the client on a unique port.
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
- Once command has been sent, client listens to its unique socket for reply from server.
- Output is displayed on the clients terminal.
- Repeat client/server send/receive until exit_ is sent.
- exit_ command sends signal to the executing server to delete its id from the known client list
*/
#include "ClientSocket.h"
#include "SocketException.h"
#include "ServerSocket.h"
#include <iostream>
#include <string>
#include <algorithm>
std::string rnd_id();

int main ()
{

	std::string myID = rnd_id();
	std::string::size_type sz;
	int port = std::stoi(myID,&sz);

	ServerSocket new_sock;
	std::string data;
	std::string output;
	std::string endnotice ="$";


	// Create client part to send port number to server for send/replies
	 cout<<"=> Creating connection to server."<<endl;
   try
    {
      ClientSocket client_socket ( "localhost", 1500 );
      try
      {
    	  client_socket << myID;
      }
      catch ( SocketException& ) {}
     // std::cout << "We received this response from the server:\n\"" << reply << "\"\n";;
    }
  catch ( SocketException& e )
    {
      std::cout << "Exception was caught:" << e.description() << "\n";
    }
  ///////////////////
  // Create server to listen for output back
  	 try
  	    {
  	      // Create the socket
  	      ServerSocket server ( port);
  	      server.accept ( new_sock );
  	    }
  	  catch ( SocketException& e )
  	    {
  	      std::cout << "Exception was caught:" << e.description() << "\nExiting.\n";
  	    }
     cout<<"=> Connection from server to client is on port:"<<myID<<endl;
  // Begin sending/receiving messages
  	bool stopread = false;
  	std::string t_data ="";
  	while (true)
  	{
  		cout<<"=> Enter command to the server."<<endl;
  	  	std::getline(std::cin,data);
  	  	data+="$";
  		try
 	    {
 	      new_sock << data;
 	      new_sock >> output;
 	      int osize = output.size();
 	      cout<<"=> Client Received:"<<endl;
 	      for (int i=0;i<osize;i++)
 	    	  {
 	    	  	 if (output[i]== '$')
 	    		  break;
				  cout<<output[i];
 	    	  }

 	      cout<<endl;

 	    }// end try
 	  catch ( SocketException& ) {}

 	 data ="";
 	 output="";
  	}

  return 0;
}

std::string rnd_id()
{
	srand((int)time(0));
	char start ='0';
	std::string temp = "0000";

	temp[0]='3';
	for (int i=1;i<4;i++)
		temp[i]= start + rand()%9;
	return temp;
}
