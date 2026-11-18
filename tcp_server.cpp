#include <iostream>
#include <sys/socket.h>
#include <poll.h>
#include <string.h>
#include <cstring>
#include <vector>
#include <arpa/inet.h>
#include <unistd.h>

// threading
#include <thread>
#include <mutex>

std::mutex mtx;
constexpr int BUFF_SIZE = 1024;
//Socket, bind, listen, accept, recv, close

void send_all_client(int client_fd, char (*buffer)[BUFF_SIZE], size_t size){
	std::lock_guard<std::mutex> lock(mtx);
	if(send(client_fd, *buffer, size, 0) < 0){  // WAIT! try using &serv_buffer first!!!
		std::cout << "Failed to send message to Client ID: " << client_fd << "\n"; 
	} // also if a send to a client fails show that
}

int main(){

	int PORT = 8080;
	int max_conn = 3;

	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_fd < 0){ std::cout << "NO SOCKET! \n"; return 1;}

	std::cout << "Socket created \n File Descriptor ID: " << server_fd << "\n";

	int opt=1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	
	
	if(bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0){
		std::cout << "BIND FAILED\n";
		return 1;
	}
	
	std::cout << "Socket Bind successful\n";
	
	if(listen(server_fd, max_conn) < 0){
		std::cout << "LISTEN FAILED\n";
		return 1;
	}

	std::cout << "Socket Listening...\n";
	
	std::vector<std::thread> send_client;
	std::vector<struct pollfd> fds(1);
	fds[0].fd = server_fd;
	fds[0].events = POLLIN;
	
	int user_num = 1;
	for(;;){
		int event_val = poll(fds.data(), fds.size(), 500);	
		
		if(event_val < 0){
			break;
		}
		if (event_val == 0){ 
			continue;
		}
		
		for(int i = fds.size()-1; i>=0; --i){
			if(fds[i].revents & POLLIN){
					char serv_buffer[BUFF_SIZE];
					std::memset(serv_buffer, 0, sizeof(serv_buffer));
					
					if(fds[i].fd == server_fd){				
						int client_fd = accept(server_fd, nullptr, nullptr);
						if(client_fd >=0){
							fds.push_back({client_fd, POLLIN, 0});
							std::cout << "NEW CLIENT ACCEPTED! ID: " << client_fd << "\n";
							std::string greetings = std::to_string(user_num);
							send(client_fd, greetings.c_str(), greetings.size(), 0);
							++user_num;
									
						}
					}
					else{
						ssize_t data_recieved = recv(fds[i].fd, serv_buffer, sizeof(serv_buffer)-1, 0);
						if(data_recieved <= 0){
							std::cout << "Client Disconnected. ID: " << fds[i].fd << "\n";
							close(fds[i].fd);
							fds.erase(fds.begin()+i);	
							continue;
						}
						else{	
							std::cout << "\n" << serv_buffer <<"\n";
							std::cout << "____________\n";
							
							for(size_t j = 1; j < fds.size(); j++){ 
								if (j == static_cast<size_t>(i)){
									continue;
								}else{
									send_client.emplace_back(send_all_client, fds[j].fd, &serv_buffer, data_recieved);
								}
							}
							for(auto& thr: send_client){
								thr.join();
							}
							send_client.clear();
						}
					}
			}
		}
	}
	
	for(const auto& x : fds){
		close(x.fd);
	}
	return 0;
}

//goals
/*
- clients can see there user name including each others for each message
-the server doesnt have to interact with that chat BUT all the clients should see their names and messages 
(SEE TF2 as an example)
-clients should be able to log out
*/