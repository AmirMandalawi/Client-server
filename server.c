/**
 * Skeleton file for server.c
 * 
 * You are free to modify this file to implement the server specifications
 * as detailed in Assignment 3 handout.
 * 
 * As a matter of good programming habit, you should break up your imple-
 * mentation into functions. All these functions should contained in this
 * file as you are only allowed to submit this file.
 */ 

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#define PORT_NUM 1024 //minimum port value to be entered
/**
 * The main function should be able to accept a command-line argument
 * argv[0]: program name
 * argv[1]: port number
 * 
 * Read the assignment handout for more details about the server program
 * design specifications.
 */ 
 
void create_socket(int port); //method to create the socket
void listener(int sockfd); //listening method for connections
void accept_requests(int sockfd); //accepting method for incoming requests from clients
void send_message(int clientfd); //sending message to the client server
void recieve_message(int clientfd); //recieving message from the client server 
void readInputFile(int clientfd, char file[]); //method for reading the inputs by the client
void writeToFile(int clientfd, char file[]); //method for wrting to the file by the client
void writing(int client, char message[]); //writing method to store the inputs

/**
 * method to throw error if there's one 
 */
void error(const char *msg){
	 printf("Error : %s\n ", msg);
	 exit(1); //exit the program
}
 
int main(int argc, char *argv[])
{
	int port = atoi(argv[1]); // store the entered port and convert it to an int
	
	if(argc != 2 || port < PORT_NUM){ //check the input is correct or the port number is less than 1024
		 return -1;
	}

	create_socket(port); //create the socket to form the connection
    return 0;
}

/**
 * method to create the socket
 */
void create_socket(int port){
	int sockfd, br; //storing the socket file descriptor and the bind 
	struct sockaddr_in serveraddr; //struct of the server address
 
  sockfd = socket(AF_INET, SOCK_STREAM, 0); //create the socket
	if(sockfd < 0){ //check if the socket has been created or not 
		close(sockfd);
		error("error creating socket");
	}
	printf("Socket created\n"); 
	
	serveraddr.sin_family = AF_INET; //add the IP address to the server address struct
	serveraddr.sin_addr.s_addr = INADDR_ANY; //an unspecific IP address for binding and creating the socket
	serveraddr.sin_port = htons(port); //converting to network byte order
	printf("Address created\n");
	
	br = bind(sockfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr)); //binding process
	if(br < 0){ //check if the binding was successful 
	    error("error binding\n");
	    exit(0);
	}
	printf("Binding successful\n");

  listener(sockfd); //calling the listener method
  accept_requests(sockfd); //calling the method to accept the incoming requests
}

/**
 * listening method for connections
 */
void listener(int sockfd){
	if(listen(sockfd, SOMAXCONN) < 0){ //check if the listening is successful
		 error("failed listening");
		 exit(0);
	}
	printf("listening successful\n");
}

/**
 * accepting th eincoming requests
 */
void accept_requests(int sockfd){
	while(1){
		struct sockaddr_in clientaddr; //client struct
		int clienlen = sizeof(clientaddr); //storing the size of the client struct
		int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, (socklen_t*)&clienlen); //accpeting the request from client
		
		if(clientfd < 0){ //checking if the request was successful or not 
			 error("accepting clients failed");
			 exit(0);
		}
		
		printf("Connection to client accepted\n");
		send_message(clientfd); //calling the method sending message to the client
		recieve_message(clientfd); //calling the method for recieving message from the client	
	}
 printf("Closing socket\n");
}

/**
 * method to send a simple message to the client 
 */
void send_message(int clientfd){
	char msg[] = "Hello\n"; //creating a char array 
  writing(clientfd, msg); //call the method for sending a message to the client
}

/**
 * method to recieve a message from the client to the server
 */
void recieve_message(int clientfd){
		while(1){
	    char message[100] = {'\0'}; //create a char array

	    ssize_t r = recv(clientfd, message, 100, 0); //recieve the message and store in the char array
        if (r <= 0){ //check if the message was recieved
            error("Error recieving message\n");
            close(clientfd);
            exit(0);
        }
      printf("Message recieved\n");
	    char commandLine[3] = { tolower(message[0]), tolower(message[1]), tolower(message[2])}; //making the command input case insensitive

	    if(strncmp(commandLine,"bye", 3) == 0){ //check if the command is bye
	       printf("Port closed\n");
	     	 break; //break out of the for loop
	    	 close(clientfd); //close the client server
	    } 
	    else if (strncmp(commandLine,"get", 3) == 0){ //check if the command is get
			    char file[100] = {'\0'}; //creating a char array

		     	for(int i = 4 ; i < 100 ; i++){ //looping after the 4th character
				      if(message[i] == '\n'){
				      	break; 
			      	}
				      file[i-4] = message[i]; //storing the message char by char starting from index 0 
		    	}
		    	printf("%s \n ", file);
			    readInputFile(clientfd, file); //calling the method to read the file that was entered 
		  }
	  	else if(strncmp(commandLine,"put", 3) == 0){ //check if the command is put 
		      char file[100] = {'\0'}; //creating a char array

	      	for(int i = 4 ; i < 100 ; i++){ //looping after the 4th character
		      		if(message[i] == '\n'){
					       break;
				      }
			      	file[i-4] = message[i]; //storing the message char by char starting from index 0 
			    }

			    writeToFile(clientfd, file); //calling the method to write to the file that was entered
		  }
	  }
	close(clientfd); //closing the client connection
}

/**
 * reding the inputs of the file that was entered 
 */
void readInputFile(int clientfd, char file[]){
	int count = 0; //varibale to track the index number of the file
	char file_name[1000] = {'\0'};
	char c;
	if(file == NULL){ //if the file doesn't exist
		writing(clientfd, "SERVER 500 GET ERROR\n");
	}

	FILE *f = fopen(file, "r"); //open the file for reading 
	if(f == NULL){ //if the file doesn't exist
		writing(clientfd, "SERVER 404 Not Found\n"); //write the message to the client
		return;
	}

	while((c = fgetc(f)) != EOF){ //store the inputs in the char array until the end of the file 
		file_name[count] = c; //store the chars into the array
		count++; //increment the index number
	}
	file_name[count++] = '\n'; //add 3 new line chars 
	file_name[count++] = '\n';
	file_name[count++] = '\n';
	writing(clientfd, "SERVER 200 OK\n"); //write the message to the client
  writing(clientfd, file_name); //print the values inside the file on clients server
  fclose(f); //close the file 
}

/**
 * write to the file that was entered
 */
void writeToFile(int clientfd, char file[]){
	FILE *f = fopen(file, "w"); //open the file for writing

	if(f == NULL){
     writing(clientfd, "SERVER 501 Put Error\n");
	}

	bool newLine = false; 
	while(1){
		char input[200]={0}; //creating the char array

		ssize_t r = recv(clientfd, input, 200, 0); //recieve the input from the client
        if (r < 0){ //check if the message was recieved
            error("Error recieving message\n");
            close(clientfd);
            exit(0);
        }

        if(strcmp(input, "\n") == 0){ //check if the input has a new line
			     if(newLine){
			       fclose(f); //close the file
			       break; //break out of the while loop
		      }
		      newLine = true; //set the boolean to true 
		   }
		   else {
		     	newLine = false; //set the boolean to false
	   	}
	  	fprintf(f, "%s", input);
  }
  writing(clientfd, "Server 201 created\n"); //write the message to the client
}

/**
 * method for writing messages
 */
void writing(int client, char message[])
{
  int s = write(client, message, strlen(message)); //write to the client
  if (s < 0){ //check if the writing was successful
    error("error on writing the message\n");
  }
  printf("writing was successful success\n");
}
