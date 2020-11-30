# instant-messaging-console
A server and client(s) program for people who wants to do instant messaging

Note: This program was done using as template the code in Unix Network Programming - The sockets API Volume 1
Chapter 6
Without these templates and explanations(client and server), it would have been almost impossible for me 
to do this project. So, many thanks to:
W. Richard Stevens
Bill Fenner
Andrew M. Rudoff

Also, I used the following link:
http://man7.org/linux/man-pages/man3/strtok.3.html 	// for use in string tokenizer to parse clients draws


program was carefully tested successfully (more than ten clients) in the following operating systems:

ubuntu 32-bits
ubuntu 64-bits
Mac OS (at the school-lab)

/------------------------------------------------------------------------------------------------------------------------------------/

The program runs as follows:

run 'make' comand at terminal

to start server type:
	 ./imserver <port-number>

to start clients type:
	 ./imclient <server-address> <server-port_number> <client-name>

After this is done, if there is only one client a message informing the client he is the only one connected will appear.
I there are more than one client the following will happen:
 - A message welcoming the new client appear in his chat page
 - The name he entered will appear as well
 - A list of current connected clients will be presented in his page(excluding him. I know, I know, this was not the instruction, 
   but I take the risk because is more fun).
 - The other clients will be informed that a new client entered the chat and an updated list will be sending to them. They 
   themselves will not see their names printed because they know they are connected (again, I know this was not the instruction
   but it was fun for me to do that, sorry).
- The number of updates clients is always at the top of the last connected clients. As informing he/she how many people in total.
  (In this case he/she is also included)
- The 'all' reserved word and the same case name have been successfully implemented. The rejected client will be informed
  about why he/she was rejected, and the server will give hints about how to do it correctly.


/------------------------------------------------------------------------------------------------------------------------------------/

The format to type a message will be as follows

for everybody:
   
msg @ all  // Note the newline
<message>  // write message here, then press enter

for one person:

msg @ user1 // substitute corresponding name here
< message> // write message here, then press enter

for a group:

msg @ user1, user2, ... , user-n
<message>	// write message here, then press enter

If a client entered the message in a wrong way. A message from server will be sent indicating instructions

/------------------------------------------------------------------------------------------------------------------------------------/

A client disconnected...
When this happens (a client disconnect pressing ctrl-c), the remaining clients will be informed a client just left the chat.
Then, an updated list will be sent to each client of how many people are still left in the chat. (the informed client 
himself will not be in the list, same reason as above.

/------------------------------------------------------------------------------------------------------------------------------------/

to clean the project
	type 'make clean' at terminal (now updated to not receive an error message when executed multiple times




