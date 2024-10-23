
# TwMailer

## Overview
TwMailer is a command-line tool designed for efficient email communication, following a client-server architecture. The server listens on a specified port for incoming requests and handles one client request at a time using an iterative model, meaning it processes requests sequentially without concurrent request handling. Clients send commands to the server to send, receive, read or delete emails. Incoming and outgoing messages, along with metadata, are stored in a "mail-spool-directory" for later retrieval.

## Technologies
The TwMailer project is implemented using C, with socket programming providing the foundation for client-server communication. The server listens on a specified TCP port, handling connections from clients one at a time. File system operations are used to store emails and metadata in the mail-spool-directory, which serves as persistent storage. The project relies on standard C libraries for file management, network communication, and string manipulation, ensuring a lightweight and efficient execution. The iterative design of the server avoids the complexity of multi-threading, focusing on simplicity and stability in handling sequential requests.

## Features
- **Send Messages**: Send messages to other users and save them in the recipient's directory.
- **List Messages**: View all messages in your own inbox (directory).
- **Read Messages**: Read specific messages from specific users in your own directory.
- **Delete Messages**: Delete specific messages from specific users in your own directory.

## User Input
Here are the possible user inputs for the TwMailer system:

```SEND```: Send a message to a specified recipient and save it in their directory.
   - **Format**:
     ```
     sender: <your_username>
     receiver: <recipient_username>
     subject: <subject_of_the_message>
     message: <your_message_content>
     ```
     
```LIST```: List all messages in your own inbox (directory).
   - **Format**: `<username>` (to list your own messages) or `All` (to list messages from all users)

```READ```: Read a specific message from a specific user in your own directory.
   - **Format**:
     ```
     user: <sender_username>
     index: <message_index>
     ```

```DEL```: Delete a specific message from a specific user in your own directory.
   - **Format**:
     ```
     user: <sender_username>
     index: <message_index>
     ```

```QUIT```: Exit the program.

## Installation
To set up TwMailer, follow these steps:

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/twmailer.git
   ```

2. Navigate to the project directory:
   ```bash
   cd twmailer
   ```

3. Build the project using:
   ```bash
   make all
   ```

## Running the Project

### Server Side
To start the TwMailer server, run the following command:
```bash
./twmailer_server.out <port> <mail_spool_directory>
```
- `<port>`: Specify the port number for the server.
- `<mail_spool_directory>`: Specify the directory where messages will be stored.

### Client Side
To start the TwMailer client, run the following command:
```bash
./twmailer_client.out <ip> <port>
```
- `<ip>`: Specify the server's IP address.
- `<port>`: Specify the port number on which the server is running.

## Contributing
Contributions to TwMailer are welcome! If you'd like to contribute, please fork the repository and submit a pull request with your changes.

## Note on Commit Messages
Please note that the commit messages in this project may not reflect professional standards. This is our first time implementing a program of this complexity, and we faced numerous challenges, particularly with socket programming. These difficulties often led to frustration and impatience, which is reflected in the commit messages. We appreciate your understanding as we continue to learn and improve our skills in software development 
