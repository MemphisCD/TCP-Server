#include <iostream>
#include <sys/socket.h>
#include <poll.h>
#include <cstring>
#include <vector>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>

std::mutex mtx; // organizes send messages by threads
constexpr int BUFF_SIZE = 1024; // maximum size of text sent/received (1024 Bytes)

void send_all_client(int client_fd, char (*buffer)[BUFF_SIZE], size_t size){ 
	std::lock_guard<std::mutex> lock(mtx);  // locks the mutex to prevent race conditions during messaging
	if(send(client_fd, *buffer, size, 0) < 0){   // send message buffer to designated client
		std::cout << "Failed to send message to Client ID: " << client_fd << "\n"; 
	} 
}

int main(){

	int PORT = 8080; // TCP Listening Port
	int max_conn = 3; // maximum number of clients that can wait to connect

	int server_fd = socket(AF_INET, SOCK_STREAM, 0); // get a IPv4 file descriptor for TCP socket
	if(server_fd < 0){ 
		std::cout << "NO SOCKET! \n"; return 1;  // terminate if failed socket creation
	}
	std::cout << "Socket created \n File Descriptor ID: " << server_fd << "\n"; 

	int opt=1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // allows reuse of a port after server restarts
	
	struct sockaddr_in address; // ipv4 socket address structure 
	address.sin_family = AF_INET; // sets address type to IPv4
	address.sin_addr.s_addr = INADDR_ANY; // bind any network interface (ethernet, local host, or wi-fi)
	address.sin_port = htons(PORT); // converts port number to network byte order
	
	if(bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0){ // bind socket to specified IP address TCP Port
		std::cout << "BIND FAILED\n";
		return 1;
	}
	std::cout << "Socket Bind successful\n";
	
	if(listen(server_fd, max_conn) < 0){ // allow the socket to listen for clients 
		std::cout << "LISTEN FAILED\n";
		return 1;
	}
	std::cout << "Socket Listening...\n";
	
	std::vector<struct pollfd> fds(1); // organizes the server (which is fds[0]) and all clients file descriptors
	fds[0].fd = server_fd; 
	fds[0].events = POLLIN; // watches server socket connection requests
	
	int user_num = 1; // increments for each connection to assign distinct user ID
	std::vector<std::thread> send_client; // stores worker threads
	
	for(;;){ 
		//runs indefinitely to maintain a continuous message service
		int event_val = poll(fds.data(), fds.size(), 500);	// blocks for 500 milliseconds to wait for socket activity
		
		if(event_val < 0){ // interrupts for loop if poll() raises an error
			break;
		}
		if (event_val == 0){  //no data to read, restart loop
			continue;
		}
		
		// Iterates backward to prevent shifting bugs if erasing disconnected clients
		for(int i = fds.size()-1; i>=0; --i){ 
			if(fds[i].revents & POLLIN){
					char serv_buffer[BUFF_SIZE]; // buffer to store incoming data
					std::memset(serv_buffer, 0, sizeof(serv_buffer)); // ensure buffer is zero-initialized to prevent garbage data leftover
					
					if(fds[i].fd == server_fd){	// activity on server_fd indicates new client connection
						int client_fd = accept(server_fd, nullptr, nullptr); // accept a new client connection
						if(client_fd >=0){
							fds.push_back({client_fd, POLLIN, 0}); // adds new client to fds vector and monitored by poll()
							std::cout << "NEW CLIENT ACCEPTED! ID: " << client_fd << "\n";
							// send assigned user number to new client so it can identify itself in chat
							std::string greetings = std::to_string(user_num); 
							send(client_fd, greetings.c_str(), greetings.size(), 0); 
							++user_num;
									
						}
					}
					else{ // activity on a client_fd either means client disconnected or an incoming message
						ssize_t data_recieved = recv(fds[i].fd, serv_buffer, sizeof(serv_buffer)-1, 0); 
						if(data_recieved <= 0){
							// if no valid data has been sent, the client will be disconnected, and its respective vector element is also closed
							std::cout << "Client Disconnected. ID: " << fds[i].fd << "\n"; 
							close(fds[i].fd); // close client file descriptor 
							fds.erase(fds.begin()+i); // remove client to update poll list 
							continue;
						}
						else{	
							std::cout << "\n" << serv_buffer <<"\n"; // the server buffer prints data to console
							std::cout << "____________\n";
							
							for(size_t j = 1; j < fds.size(); j++){ // send data to all other clients
								if (j == static_cast<size_t>(i)){  // skips client that originally sent the data
									continue;
								}else{
									send_client.emplace_back(send_all_client, fds[j].fd, &serv_buffer, data_recieved); // create thread to send message
								}
							}
							for(auto& thr: send_client){ // waits for all messaging threads to finish before clearing the buffer.
								thr.join();
							}
							send_client.clear(); // destroy finished threads and clear resources
						}
					}
			}
		}
	}
	// closes each file descriptor before termination to prevent resource leaks
	for(const auto& x : fds){ 
		close(x.fd);          
	} 
	return 0;
}
