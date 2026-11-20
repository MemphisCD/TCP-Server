# TCP chat server & command line client terminal (C++)

A server multiple users can join on their terminal

## Description

A server program binds a socket to an IP address, and accepts multiple clients to send data.
Each client has a unique username, and views messages from other client similar 
to a message board. The server and client program runs on C++.

## Key Features
- I/O multiplexing using the poll() system call, this monitors file descriptors signaling data transmission
- Clients send messages to entire server via Multithreading using mutex to synchronize resource access.


## Dependencies ????
- Tested on Linux (WSL), compatible with any Linux Distribution that is POSIX-compliant
- Any compiler that can support c++17 (such as g++ 13.3.0 or gcc 7+)

## Installation and compilation  

1. Clone the repository:

'''bash
git clone <???>
cd <>
'''
2. Compile both the server and client programs:
'''bash
	g++ -std=c++17 tcp_server.cpp -o s
	g++ -std=c++17 tcp_client.cpp -o c
'''
## Execution:

1. Start the server:

'''bash
	./s
'''
2. Start the client (in a new terminal):

'''bash
	./c
'''

## Example output

### Server Terminal

'''text
name@host$ ./s

Socket created
 File Descriptor ID: 3
Socket Bind successful
Socket Listening...
NEW CLIENT ACCEPTED! ID: 4
NEW CLIENT ACCEPTED! ID: 5

user_1: hello!

____________

user_2: Greetings friend!

____________

user_1: How are you?

____________

user_2: I must leave now... Goodbye!

____________
Client Disconnected. ID: 5

user_1: dang

____________
'''

### Client (user_1) terminal

'''text
name@host$ ./c

Welcome to the Server!
 Your username is: user_1
Enter 'Ctrl-D' to quit
____________
hello!

user_2: Greetings friend!

____________
How are you?

user_2: I must leave now... Goodbye!

____________
dang

name@host$ 
'''

## Future improvements
- private messaging between two or more clients
- administration capabilities where a client has special privileges
- clients can choose their own username


## Acknowledgments

https://medium.com/@smumtaz.bscs24seecs/setting-up-a-client-and-server-using-c-sockets-c3ddbc1742f8
