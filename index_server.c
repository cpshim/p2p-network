/* time_server.c - main */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*------------------------------------------------------------------------
 * main - Iterative UDP server for TIME service
 *------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
	struct  sockaddr_in fsin;	/* the from address of a client	*/
    struct content {
        char peerName[10];
        char contentName[10];
        char address[80];
    }
	struct pdu {
		char type;
		char data[100];
	};
	char	rbuf[100], sbuf[100];		/* "input" buffer; any size > 0	*/
	char    *pts;
    char readPeerName[10], readContentName[10], readAddress[80];
	int	sock;			/* server socket		*/
	time_t	now;			/* current time			*/
	int	alen;			/* from-address length		*/
	int *numContent = 0, *numUsers = 0;	//how many items are in database
	struct  sockaddr_in sin; /* an Internet endpoint address         */
    int     s, type;        /* socket descriptor and socket type    */
	int 	port=3000;
	int i, j, bytesRead;
	struct pdu spdu, rpdu;
	struct stat fileStats;
    char database[10][100][2]; //database only able to hold 10 unique users and 100 unique content name and 1 address; might need to make dynamic array
	int fileSize;
	FILE *filePointer;
                                                                                

	switch(argc){
		case 1:
			break;
		case 2:
			port = atoi(argv[1]);
			break;
		default:
			fprintf(stderr, "Usage: %s [port]\n", argv[0]);
			exit(1);
	}

        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(port);
                                                                                                 
    /* Allocate a socket */
        s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s < 0)
		fprintf(stderr, "can't create socket\n");
                                                                                
    /* Bind the socket */
        if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		fprintf(stderr, "can't bind to %d port\n",port);
        listen(s, 5);	
	alen = sizeof(fsin);

	while (1) {
		
		if (recvfrom(s, &rpdu, sizeof(rpdu), 0, 
				(struct sockaddr *)&fsin, &alen) < 0)
			fprintf(stderr, "recvfrom error\n");

		switch(rpdu.type){
            case 'R':
                strncpy(readPeerName, rpdu.data, 10);
                strncpy(readContentName, rpdu.data+10, 10);
				strncpy(readAddress, rpdu.data+20, 80);

				//put for loop into a function to call
				if (checkInDatabase(readPeerName, readContentName)) {
					spdu.type = 'E';
					strcpy(spdu.data, "Error: Content already registered");
					printf("%s\n",spdu.data);
					sendto(s, &spdu, sizeof(spdu), 0, 
						(struct sockaddr *)&fsin, sizeof(fsin));
				}
				else {
					if ((*numContent < 100) && (*numUsers < 10)) {
						database[*numUsers][0][0] = readPeerName;
						database[*numUsers][*numContent][0] = readContentName;
						database[*numUsers][*numContent][1] = readAddress;
					}
					//database[i] = //add content into database

					spdu.type = 'A';
					strcpy(spdu.data, "Success");
					sendto(s, &spdu, sizeof(spdu), 0, 
						(struct sockaddr *)&fsin, sizeof(fsin));

					//create tcp socket
				}
                // for (i = 0; i < 10; i++) {
				// 	if (strcmp(readPeerName, database[i][0][0].peerName) != 0) { //strcmp output 0 if true
				// 		for (j = 0; j < 100; j++) {
				// 			if (strcmp(readContentName, database[i][j][0].contentName) != 0) {
				// 				spdu.type = 'E';
				// 				strcpy(spdu.data, "Error: Content already registered");
				// 				printf("%s\n",spdu.data);
				// 				sendto(s, &spdu, sizeof(spdu), 0, 
				// 					(struct sockaddr *)&fsin, sizeof(fsin));
				// 				break;
				// 			}
				// 		}
				// 	}
                // }

			case 'C':
				filePointer = fopen(rpdu.data, "r");

				if (filePointer == NULL){
					spdu.type = 'E';
					strcpy(spdu.data, "Error opening file");
					printf("%s\n",spdu.data);
					sendto(s, &spdu, sizeof(spdu), 0, 
						(struct sockaddr *)&fsin, sizeof(fsin));
				}

				if (filePointer != NULL) {
					lstat(rpdu.data, &fileStats);
					fileSize = (int) fileStats.st_size;

					for(i = 0; i < fileSize; i += 99){
						fseek(filePointer, i, SEEK_SET);
						bytesRead = fread(rbuf,sizeof(rbuf)-1,1,filePointer);

						strcpy(spdu.data, rbuf);

						if (feof(filePointer)) {
							spdu.type = 'F';
						}
						else {
							spdu.type = 'D';
						}

						sendto(s, &spdu, sizeof(spdu), 0, 
							(struct sockaddr *)&fsin, sizeof(fsin));

						bzero(spdu.data, 100);
						bzero(rbuf, 100);
					}		
				}				
				fclose(filePointer); 
				break;

			case 'E':
				printf("Error\n");
				break;
		}
	}
}

int checkInDatabase(char readPeerName[], char readContentName[]) {
	for (i = 0; i < 10; i++) {
		if (strcmp(readPeerName, database[i][0][0].peerName) != 0) { //strcmp output 0 if true
			for (j = 0; j < 100; j++) {
				if (strcmp(readContentName, database[i][j][0].contentName) != 0) {
					return 1;
				}
			}
		}
	}
	return 0;
}