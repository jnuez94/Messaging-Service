Joshua Nuez
jn2548

"Messaging Service"

The SOFTWARE is a messaging service written in C/C++. It utilizes TCP server-client connection and multithreading to handle multiple clients.

The software was written in a Linux environment with GCC version 4.6.3 and OpenSSL version 1.0.1.

Type "make" on the console to compile the executables. The make command should create 2 executables "Server" and "Client".

Server Usage:
$ ./Server <port number>

Client Usage:
$ ./Client <ip address/domain name> <port number>

Available commands:

who				==> Displays name of other connected users, including the querying user

last <number>	==> Displays name of those users connected within the last <number> minutes

broadcast		==>	Broadcast <message> to all connected users, but not sender

send (<user> 
	<user>...
	<user>)
	<message>	==> Sends <message> to the list of users separated by space. Ignore users who are not logged in

send <user>
	<message>	==> Private <messages> to a single <user>

logout			==> Log out this user

