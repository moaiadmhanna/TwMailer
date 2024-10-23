
# TwMailer

## Overview
TwMailer is a command-line tool designed for efficient email communication. It allows users to send messages, manage their inbox, and perform various operations through a simple interface. The application utilizes a client-server architecture for sending and receiving messages.

## Features
- **Send Messages**: Send messages to other users and save them in the recipient's directory.
- **List Messages**: View all messages in your own inbox (directory).
- **Read Messages**: Read specific messages from specific users in your own directory.
- **Delete Messages**: Delete specific messages from specific users in your own directory.
- **Client-Server Architecture**: Communicate through a server setup, allowing for multiple clients to send and receive messages.

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
