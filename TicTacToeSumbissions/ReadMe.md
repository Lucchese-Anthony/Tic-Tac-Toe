#submissions are found at /CSclass/

GROUP 4: Henry Lembo, Anthony Lucchese, Prisco Pocius
ReadMe.md
This simple read me file tells the reader how to compile and run our game, as well as some noted.

Version 3
1. compile:
   server: gcc -lpthread -o server server-thread-2021.c server-assignment-3.c
   client: gcc -o client client-assignment-3.c client-thread-2021.c
2. run:
   server: ./server 17400
   client: ./client freebsd1.cs.scranton.edu 17400 c*.h
3. on the client:
   - now the client must put in their username, press enter, and then their
   password, then the client is connected
   - Note: if the username is correct but the password is incorrect, the
   user is disconnected and must re-reun the program (designed like that)
4. everything is the same otherwise stated for Version 2, and further 
   Version 1


Version 2
1. Compile
Client: gcc -o client client-assignment-2.c client-thread-2021.c
Server: gcc -lpthread -o server server-assignment-2.c server-thread-2021.c
2. Run
Client: ./client freebsd1.cs.scranton.edu 17400 c*.h
Server: ./server 17400
3. Notes
Same as in version 1.

Version 1
1. Compile
Client: gcc -o client Client-Assignment-1.c client-thread-2021.c
Server: gcc -lpthread -o server Server-Assignment-1.c server-thread-2021.c
2. Run
Client: ./client freebsd1.cs.scranton.edu 17400 c*.h
Server: ./server 17400
3. Notes
3.1 We generally did both the compiles and runs on the same line by putting && between them.
      They looked like this
      Client: gcc -o client Client-Assignment-1.c client-thread-2021.c && ./client freebsd1.cs.scranton.edu 17400 c*.h
      Server: gcc -lpthread -o server Server-Assignment-1.c server-thread-2021.c && ./server 17400
3.2 The server must be ran on freebsd1. Otherwise the clients must be on 2 and 3.
3.3 The server must be ran first. The clients can start in any order after the server is up.
3.4 The first client to connect is player 1 and goes first.
3.5 In order to make a move, the player must enter the coordinates as "x y". There should only be a space, not a comma.
