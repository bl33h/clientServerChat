# clientServerChat
This is a chat application that facilitates communication between multiple clients through a centralized server. The application is designed using socket programming and handles multiple client connections concurrently using threads.

<p align="center">
  <br>
  <img src="https://i.pinimg.com/originals/0e/4a/c3/0e4ac37acbff81cd087aa19692a07a9d.gif" alt="Chat Application Architecture" width="500">
  <br>
</p>
<p align="center">
  <a href="#Files">Files</a> •
  <a href="#Features">Features</a> •
  <a href="#How-to-Use">How To Use</a> •
  <a href="#Contributions">Contributions</a>
</p>

## Files
- `client.c`: Implements the client-side of the chat application.
- `server.c`: Implements the server-side of the chat application, managing multiple client connections.
- `chat.pb-c.c`, `chat.pb-c.h`: Provide protobuf-c functionality for structured message communication between clients and server.

## Features
- **Multi-client Support**: The server can handle multiple client connections simultaneously.
- **Threaded Communication**: Utilizes threads to manage client connections and maintain server responsiveness.
- **Protobuf Communication**: Uses protobuf for structured and efficient data serialization.
- **Dynamic User Interaction**: Supports real-time messaging, user status updates, and connection management.
- **Robust Error Handling**: Implements error checking and handling to ensure reliable operation.

## How To Use
To clone and run this application, you'll need [WSL (Windows Subsystem for Linux)](https://learn.microsoft.com/en-us/windows/wsl/install) and the following tools installed on it: [Git](https://git-scm.com), [C compiler](https://gcc.gnu.org), and [POSIX threads](https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html). From your command line:

```bash
# Clone this repository
$ git clone https://github.com/bl33h/clientServerChat

# Open the project
$ cd src

# Compile the server
$ gcc -o server server.c chat.pb-c.c -lprotobuf-c -pthread
$ ./server 8080 [THE PORT MIGHT BE DIFFERENT]

# Compile the client
$ gcc -o client client.c chat.pb-c.c -lprotobuf-c -pthread
$ ./client username 127.0.0.1 [THE IP MIGHT BE DIFFERENT] 8080 [SAME PORT AS THE SERVER]

# Start the server (specify the port number)
$ ./server 12345

# In a new terminal, start a client
$ ./client 127.0.0.1 12345
