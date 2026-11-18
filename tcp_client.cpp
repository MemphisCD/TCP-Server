#include <iostream>
#include <sys/socket.h>
#include <poll.h>
#include <string.h>
#include <cstring>
#include <vector>
#include <arpa/inet.h>
#include <unistd.h>


int main(){
	
	int PORT = 8080;
	int BUFF_SIZE = 1024;
	//int max_conn = 3;
	
	int client_fd;
	char cli_buffer[BUFF_SIZE];
	std::memset(cli_buffer, 0, sizeof(cli_buffer));
	
	if((client_fd=socket(AF_INET, SOCK_STREAM, 0)) < 0){
		std::cout << "SOCKET CREATION FAILED!\n";
		return 1;
	}
	
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT);
	
	
	if(connect(client_fd,(struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		std::cout << "CLIENT CONNECTION FAILURE!\n";
		return 1;
	}
	
	
	ssize_t greetings = recv(client_fd, cli_buffer, sizeof(cli_buffer)-1,0);
	if(greetings <= 0){return 1;}
	std::cout << "Welcome to the Server!\n Your user name is: user_" << cli_buffer <<"\n"
	<< "____________\n";	
	
	std::string user_name = cli_buffer;  // the user name of the connected client

	
	
	
	std::vector<struct pollfd> fds(2);
	fds[0].fd = client_fd;
	fds[0].events = POLLIN;
	fds[1].fd = STDIN_FILENO;
	fds[1].events = POLLIN;
	//struct* pollfd fd_add = &fd;
	
	for(;;){
		int event_val = poll(fds.data(), 2, -1);
		std::memset(cli_buffer, 0, sizeof(cli_buffer));
		
		if (POLLIN & fds[0].revents){
			ssize_t data_recieved = recv(client_fd, cli_buffer, sizeof(cli_buffer)-1,0);
			if(data_recieved > 0){
				std::cout << "\n" << cli_buffer <<"\n";
				std::cout << "____________\n";
			}
			else{break;}
		}if(POLLIN & fds[1].revents){
			std::string str;
			if(!std::getline(std::cin, str)){
				break;
			}
			std::string msg = "user_"+user_name + ": " + str + "\n";
			if(send(client_fd, msg.c_str(), msg.length(), 0) < 0){
				std::cout << "SEND FAILED! \n";
				break; 
			}
		}
	}
	
	
	close(client_fd);
	return 0;
}