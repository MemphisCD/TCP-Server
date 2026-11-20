#include <iostream>
#include <sys/socket.h>
#include <poll.h>
#include <cstring>
#include <vector>
#include <arpa/inet.h>
#include <unistd.h>

int main(){
	
	int PORT = 8080; // TCP PORT 
	int BUFF_SIZE = 1024; // size of message buffer
	
	
	int client_fd; // clients file descriptor
	char cli_buffer[BUFF_SIZE]; // buffer for data sent/received by client
	std::memset(cli_buffer, 0, sizeof(cli_buffer)); // zero-initialize buffer to clearout garbage data left over
	
	if((client_fd=socket(AF_INET, SOCK_STREAM, 0)) < 0){ // get IPv4 socket file descriptor for Client
		std::cout << "SOCKET CREATION FAILED!\n";
		return 1;
	}
	
	struct sockaddr_in serv_addr; // IPv4 address structure
	std::memset(&serv_addr, 0, sizeof(serv_addr)); // zero-initialize address structure
	serv_addr.sin_family = AF_INET; // set address type to IPv4
	serv_addr.sin_port = htons(PORT); //convert TCP port number to network byte order
	
	
	//Convert human readable IPv4 string (127.0.0.1) to binary byte order
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0){
		std::cout << "ADDRESS NOT SUPPORTED!\n";
		return 1;
	}
	
	if(connect(client_fd,(struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){ // connect socket to address given client_fd
		std::cout << "CLIENT CONNECTION FAILURE!\n";
		return 1;
	}
	
	
	ssize_t greetings = recv(client_fd, cli_buffer, sizeof(cli_buffer)-1,0); // receive greetings message
	if(greetings <= 0){return 1;} //if no greetings terminate program
	std::cout << "Welcome to the Server!\n Your user name is: user_" << cli_buffer <<"\n";
	std::cout << "Enter 'Ctrl-D' to quit \n" << "____________\n";	
	
	std::string user_name = cli_buffer;  // the user name of the connected client
	
	//create short polling list that monitors client_fd and keyboard (fds[0] and fds[1] respectively)
	std::vector<struct pollfd> fds(2); 
	fds[0].fd = client_fd;			
	fds[0].events = POLLIN;
	fds[1].fd = STDIN_FILENO;
	fds[1].events = POLLIN;
	
	for(;;){
		// run loop indefinitely message until disconnect or error
		int event_val = poll(fds.data(), 2, -1); // let poll() sleep until an event is flagged
		std::memset(cli_buffer, 0, sizeof(cli_buffer));
		
		// if a message is received, print buffer to console
		if (POLLIN & fds[0].revents){ 
			ssize_t data_recieved = recv(client_fd, cli_buffer, sizeof(cli_buffer)-1,0);
			if(data_recieved > 0){
				std::cout << "\n" << cli_buffer <<"\n";
				std::cout << "____________\n";
			}
			else{break;} // terminate if server disconnects or error occurs
			
		// if a keyboard activity detected, read message to be sent
		}if(POLLIN & fds[1].revents){
			std::string str;
			// if stdin hits End of File (EOF) when user presses Ctrl-D, break loop
			if(!std::getline(std::cin, str)){  
				break;
			}
			// format message with user ID to be sent and displayed to all recipients
			std::string msg = "user_"+user_name + ": " + str + "\n"; 
			if(send(client_fd, msg.c_str(), msg.length(), 0) < 0){ // if send failure, terminate
				std::cout << "SEND FAILED! \n";   
				break; 
			}
		}
	}
	
	close(client_fd); // close file descriptor to prevent resource leak
	return 0;
}
