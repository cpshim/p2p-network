/* time_client.c - main */
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <strings.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <math.h>
#include <time.h>

time_t t;

#define BUFSIZE 64
#define SERVER_TCP_PORT 3000 /* well-known port */
#define BUFLEN 256			 /* buffer length */
#define PACKET_LEN 99		 // last character is null
#define MAXCONTENT 5

#define MSG "Any Message \n"

int fSize(FILE *fp)
{
	fseek(fp, 0L, SEEK_END);
	int sz = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	return sz;
}
/*------------------------------------------------------------------------
 * main - UDP client for TIME service that prints the resulting time
 *------------------------------------------------------------------------
 */
typedef struct
{
	char type;
	char data[100];
} pdu;

typedef struct
{
	int port;
	int pid;
	char content[100];
} Content;

Content childr[MAXCONTENT];

int peerServer(int sd, char *contentname);
void reaper(int sig);
int TCP();
int peerClient(int sd, char *contentname, pdu spdu);
int TCPrec(int portTCP, char *contentname, pdu spdu, char *AddressS);
void closePID(char *contentname);

void sendFilename(int s);

int main(int argc, char **argv)
{

	while (1)
	{
		char *host = "localhost";
		int port = 3000;
		int threadNum = 0;
		struct hostent *phe;	/* pointer to host information entry	*/
		struct sockaddr_in sin; /* an Internet endpoint address		*/
		int s, n, type, alen;	/* socket descriptor and socket type	*/
		printf("What is your name?\n");
		char peername[100];
		scanf("%s", peername);
		while (strlen(peername) > 10)
		{
			printf("Please enter a shorter name (10 Characters)\n");
			scanf("%s", peername);
		}
		switch (argc)
		{
		case 1:
			break;
		case 2:
			host = argv[1];
		case 3:
			host = argv[1];
			port = atoi(argv[2]);
			break;
		default:
			fprintf(stderr, "usage: UDPtime [host [port]]\n");
			exit(1);
		}

		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);

		/* Map host name to IP address, allowing for dotted decimal */
		if (phe = gethostbyname(host))
		{
			memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
		}
		else if ((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
			fprintf(stderr, "Can't get host entry \n");

		/* Allocate a socket */
		s = socket(AF_INET, SOCK_DGRAM, 0);
		if (s < 0)
			fprintf(stderr, "Can't create socket \n");

		/* Connect the socket */
		if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
			fprintf(stderr, "Can't connect to %s %s \n", host, "Time");
		alen = sizeof(sin);
		int i;
		int j;
		/* Read files */
		printf("Issue a command.\n");
		char recieveData[101];
		char scannedtype[10];
		char contentname[100];
		char question[100];
		int portNumTCP;
		char intString[20];
		int debug = 0;
		pdu rpdu;
		pdu spdu;
		while (1)
		{
			// Initialize the send pdu data
			bzero(spdu.data, sizeof(spdu.data));
			strcpy(spdu.data, "\0");
			printf("PDU type:\n");

			scanf("%s", scannedtype);
			spdu.type = scannedtype[0];
			switch (spdu.type)
			{
			case 'R':
				srand((unsigned)time(&t));
				portNumTCP = (rand() % 40000) + 10000;
				printf("Which file to register?\n");
				scanf("%s", contentname);
				if (strlen(contentname) > 10)
				{
					printf("Content name too big\n");
					break;
				}
				// concatenate the name, content, and port with $
				strcat(spdu.data, peername);
				strcat(spdu.data, "$");
				strcat(spdu.data, contentname);
				strcat(spdu.data, "$");
				sprintf(intString, "%d", portNumTCP);
				strcat(spdu.data, intString);
				strcat(spdu.data, "\0");
				printf("%c %s %lu should be sent.\n", spdu.type, spdu.data, sizeof(spdu));
				(void)write(s, &spdu, sizeof(spdu));
				// get the ACK or error
				// char listenBufR[101];
				bzero(rpdu.data, sizeof(rpdu.data));
				if (recvfrom(s, &rpdu, sizeof(rpdu), 0, (struct sockaddr *)&sin, &alen) < 0)
					fprintf(stderr, "recvfrom error\n");
				else
				{
					fprintf(stderr, "Receive type from server:  %c.\n", rpdu.type);
					if (rpdu.type != 'A')
					{
						// Received error
						printf("Not starting TCP.\n");
						fprintf(stderr, "Receive data from server:  %s.\n", rpdu.data);
						// printf("Peername already in use for this content\n");
						// scanf("%s", peername);
						break;
					}
					else
					{
						// Received Acknowledgement
					}
				}
				fprintf(stderr, "Data from RDPU data: %s", rpdu.data);
				strcpy(childr[threadNum].content, contentname);
				fprintf(stderr, "Content in childr thread %d: %s\n", threadNum, childr[i].content);

				// portNumTCP[0] = htons(0);
				childr[threadNum].port = portNumTCP;
				childr[threadNum].pid = fork();

				switch (childr[threadNum].pid)
				{
				case 0: /* child */
					TCP(portNumTCP, contentname);
				default:
					printf("PID port and content of child: %d %d %s\n", childr[threadNum].pid, childr[threadNum].port, childr[threadNum].content); /* parent */
					threadNum++;
					continue;
				case -1:
					strcpy(childr[threadNum].content, "");
					childr[threadNum].port = 0;
					fprintf(stderr, "fork: error\n");
					break;
					break;
				}
			case 'T':
				printf("Which file to delete?\n");
				scanf("%s", contentname);
				if (strlen(contentname) > 10)
				{
					printf("Content name too big\n");
					break;
				}
				// concatenate the name, content, and port with $
				strcat(spdu.data, peername);
				strcat(spdu.data, "$");
				strcat(spdu.data, contentname);
				strcat(spdu.data, "\0");
				printf("%c %s %lu should be sent.\n", spdu.type, spdu.data, sizeof(spdu));
				(void)write(s, &spdu, sizeof(spdu));
				// get the ACK or error
				// char listenBufR[101];
				bzero(rpdu.data, sizeof(rpdu.data));
				if (recvfrom(s, &rpdu, sizeof(rpdu), 0, (struct sockaddr *)&sin, &alen) < 0)
					fprintf(stderr, "recvfrom error\n");
				else
				{
					fprintf(stderr, "Received Ack or Error in T %c.\n", rpdu.type);
					if (rpdu.type != 'A')
					{
						// Received error
						// printf("Not starting TCP.\n");
						// printf("Peername already in use for this content\n");
						printf("Not able to deregister\n");
						break;
					}
					else
					{
						printf("Attempting to kill thread\n");
						closePID(contentname);
						printf("Successfully killed thread\n");
					}
				}
				break;
			case 'O':
				strcat(spdu.data, "\0");
				(void)write(s, &spdu, sizeof(spdu));
				bzero(rpdu.data, sizeof(rpdu.data));
				if (recvfrom(s, &rpdu, sizeof(rpdu), 0, (struct sockaddr *)&sin, &alen) < 0)
					fprintf(stderr, "recvfrom error\n");
				else
				{
					fprintf(stderr, "Received Ack or Error in O  %c.\n", rpdu.type);
					if (rpdu.type != 'O')
					{
						// Received error
						// printf("Not starting TCP.\n");
						// printf("Peername already in use for this content\n");
						fprintf(stderr, "Error from Server:\n%s", rpdu.data);
						break;
					}
					else
					{
						fprintf(stderr, "List of registered content:\n%s", rpdu.data);
					}
				}
				break;

			case 'S':
				bzero(rpdu.data, sizeof(rpdu.data));
				printf("Tell me the content.\n");
				scanf("%s", contentname);
				strcat(spdu.data, peername);
				strcat(spdu.data, "$");
				strcat(spdu.data, contentname);
				strcat(spdu.data, "$");
				printf("%c %s is sent to index server.\n", spdu.type, spdu.data);
				// char listenBufS[101];
				// bzero(listenBufS, sizeof(listenBufS));
				(void)write(s, &spdu, strlen(spdu.data) + 1);
				if (recvfrom(s, &rpdu, sizeof(rpdu), 0, (struct sockaddr *)&sin, &alen) < 0)
					fprintf(stderr, "recvfrom error\n");
				// rpdu.type = listenBufS[0];
				// printf("%c was found.\n",listenBufS[0]);
				// listenBufS[101] = '\0';
				printf("Receive from server type: %c data %s buf.\n", rpdu.type, rpdu.data);
				switch (rpdu.type)
				{
				case 'S':
					printf("Case S.\n");
					bzero(spdu.data, sizeof(spdu.data));
					char readPeerName[10];
					char readAddr[80];
					char readPort[10];
					int track1S = 0;
					int track2S = 0;
					int peerlenS = 0;
					int adrlenS = 0;
					int portlenS = 0;
					// need dashes or some limiter to determine when end of name is
					i = 0;
					fprintf(stderr, "Before readPeerName while \n");
					while (rpdu.data[i] != '$')
					{
						readPeerName[i] = rpdu.data[i];
						i++;
					}
					readPeerName[i] = '\0';
					i++;
					t = 0;
					fprintf(stderr, "Before readAddr while \n");
					while (rpdu.data[i] != '$')
					{
						readAddr[t] = rpdu.data[i];
						i++;
						t++;
					}
					readAddr[t] = '\0';
					i++;
					t = 0;
					fprintf(stderr, "Before readPort while \n");
					while (rpdu.data[i] != '\0')
					{
						readPort[t] = rpdu.data[i];
						i++;
						t++;
					}
					readPort[t] = '\0';

					printf("Received %s %s %s to now try to download file.\n", readPeerName, readAddr, readPort);

					strcpy(spdu.data, readPeerName);
					strcat(spdu.data, "$");
					strcat(spdu.data, contentname);
					if (!(strcmp(readPeerName, peername)))
					{
						printf("You already have this file!\n");
						break;
					}
					int portSint = atoi(readPort);
					strcpy(childr[threadNum].content, contentname);
					childr[threadNum].port = portNumTCP;
					childr[threadNum].pid = fork();
					switch (childr[threadNum].pid)
					{
					case 0: /* child */
						TCPrec(portSint, contentname, spdu, readAddr);
						printf("Downloaded file done. I am now going to be a host.\n");
						sleep(1);
						// make myself a host
						bzero(spdu.data, sizeof(spdu.data));
						spdu.type = 'R';
						srand((unsigned)time(&t));
						int randum = (rand() % 40000) + 10000;
						fprintf(stderr, "Register content at port %d.\n", randum);
						strcat(spdu.data, peername);
						strcat(spdu.data, "$");
						strcat(spdu.data, contentname);
						strcat(spdu.data, "$");
						sprintf(intString, "%d", randum);
						strcat(spdu.data, intString);
						fprintf(stderr, "Sending registration PDU %c %s.\n", spdu.type, spdu.data);
						(void)write(s, &spdu, strlen(spdu.data) + 1);
						// get the ACK or error
						bzero(&rpdu, sizeof(rpdu));
						if (recvfrom(s, &rpdu, sizeof(rpdu), 0, (struct sockaddr *)&sin, &alen) < 0)
							fprintf(stderr, "recvfrom error\n");
						else
						{
							fprintf(stderr, "Received Ack or Error: %c.\n", rpdu.type);
							if (rpdu.type != 'A')
							{
								printf("Not making TCP.\n");
								break;
							}
						}
						fprintf(stderr, "Making a socket to register downloaded content\n");
						TCP(randum, contentname);
						break;
					default:
						printf("PID port and content of child: %d %d %s\n", childr[threadNum].pid, childr[threadNum].port, childr[threadNum].content); /* parent */
						printf("Continuing operation.\n");
						threadNum++;
						printf("There are now %d TCP ports open.\n", threadNum);
						continue;
					case -1:
						fprintf(stderr, "fork: error\n");
						break;
						break;
					}
					printf("Finalizing operation.\n");
					break;
				default:
					printf("Error in syntax command\n");
					break;
				}
				break;
			case 'Q':
				for (i = 0; i < MAXCONTENT; i++)
				{
					// reset spdu data
					bzero(spdu.data, sizeof(spdu.data));
					// if not equal to null
					fprintf(stderr, "Deleting from childr %d: %s\n", i, childr[i].content);
					fprintf(stderr, "Content in childr %d: %s\n", i, childr[i].content);
					if (strcmp(childr[i].content, ""))
					{
						spdu.type = 'T';
						strcat(spdu.data, peername);
						strcat(spdu.data, "$");
						strcat(spdu.data, childr[i].content);
						strcat(spdu.data, "\0");
						printf("%c %s %lu should be sent.\n", spdu.type, spdu.data, sizeof(spdu));
						(void)write(s, &spdu, sizeof(spdu));
						bzero(rpdu.data, sizeof(rpdu.data));
						if (recvfrom(s, &rpdu, sizeof(rpdu), 0, (struct sockaddr *)&sin, &alen) < 0)
							fprintf(stderr, "recvfrom error\n");
						else
						{
							fprintf(stderr, "Received Ack or Error  %c.\n", rpdu.type);
							if (rpdu.type != 'A')
							{
								// Received error
								printf("Not starting TCP.\n");
								printf("Peername already in use for this content\n");
								break;
							}
							else
							{
								printf("Attempting to kill thread\n");
								closePID(childr[i].content);
								printf("Successfully killed thread\n");
								sleep(1);
							}
						}
					}
				}
				break;

			default:
				printf("Should not get here All case.\n");
				break;
			}
		}
	}
}

int TCP(int portTCP, char *contentname)
{
	int sd, new_sd, client_len;
	struct sockaddr_in server, client;
	FILE *fptr;
	char *file;

	/* Create a stream socket	*/
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr, "Can't creat a socket\n");
		exit(1);
	}

	/* Bind an address to the socket	*/
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(portTCP);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		fprintf(stderr, "Can't bind name to socket\n");
		exit(1);
	}

	/* queue up to 5 connect requests  */
	listen(sd, 5);

	(void)signal(SIGCHLD, reaper);

	while (1)
	{
		client_len = sizeof(client);
		new_sd = accept(sd, (struct sockaddr *)&client, &client_len);
		if (new_sd < 0)
		{
			fprintf(stderr, "Can't accept client \n");
			exit(1);
		}
		switch (fork())
		{
		case 0: /* child */
			(void)close(sd);
			fprintf(stderr, "Connected TCP port.\n");
			exit(peerServer(new_sd, contentname));
		default: /* parent */
			fprintf(stderr, "Returning from TCP port creation.\n");
			(void)close(new_sd);
			break;
		case -1:
			fprintf(stderr, "fork: error\n");
		}
		// fprintf(stderr,"Test.\n");
	}
}

void closePID(char *contentname)
{
	printf("Checking %s.\n", contentname);
	int verdict = 0;
	int i;
	for (i = 0; i < MAXCONTENT; i++)
	{
		if (!(strcmp(childr[i].content, contentname)))
		{
			verdict = 1;
			printf("Closing socket %s with pid: %d\n", contentname, childr[i].pid);
			kill(childr[i].pid, SIGTERM);
			(void)signal(SIGCHLD, SIG_IGN);
			printf("Closed PID:%d\n", childr[i].pid);
			strcpy(childr[i].content, "");
			childr[i].port = 0;
			childr[i].pid = 0;
			break;
		}
	}
	printf("Finished closing PID\n");
}

int TCPrec(int portTCP, char *contentname, pdu spdu, char *AddressS)
{
	fprintf(stderr, "Attempting to download %s.\n", contentname);
	fprintf(stderr, "Trying to connect to %s.\n", AddressS);

	int n, i, bytes_to_read;
	int sd, port;
	port = portTCP;
	struct hostent *hp;
	struct sockaddr_in server;
	char *host, *bp, rbuf[BUFLEN], sbuf[PACKET_LEN], sbuf2[PACKET_LEN];
	host = AddressS;
	FILE *fp;

	/* Create a stream socket	*/
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr, "Can't creat a socket\n");
		exit(1);
	}

	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if (hp = gethostbyname(host))
		bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
	else if (inet_aton(host, (struct in_addr *)&server.sin_addr))
	{
		fprintf(stderr, "Can't get server's address\n");
		exit(1);
	}

	/* Connecting to the server */
	if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		fprintf(stderr, "Can't connect \n");
		exit(1);
	}
	peerClient(sd, contentname, spdu);
	// check this shit
}

/*	reaper		*/
void reaper(int sig)
{
	int status;
	while (wait3(&status, WNOHANG, (struct rusage *)0) >= 0)
		;
}

int peerServer(int sd, char *contentname)
{
	char *bp, sbuf[256], rbuf[100], fbuf[101];
	int i, n, bytes_to_read, fileSize, numOfPackets, bytesRead, packetNum;
	FILE *filePointer;

	// read(sd, sbuf, 256);

	printf("We are trying to open %s\n", contentname);

	// filePointer = fopen(sbuf, "r");
	// sbuf[strcspn(sbuf, "\r\n")] = 0;
	filePointer = fopen(contentname, "r");

	if (filePointer == NULL)
	{
		fprintf(stderr, "Error opening file\n");
		write(sd, "Error opening file", 19);
	}

	if (filePointer != NULL)
	{
		// write(sd, error, sizeof(error));
		// close(sd);

		// printf("WE opened file\n");

		fseek(filePointer, 0L, SEEK_END); // finding the size of file

		fileSize = ftell(filePointer);
		numOfPackets = fileSize / PACKET_LEN;
		fseek(filePointer, 0L, SEEK_SET);

		if (fileSize % PACKET_LEN != 0)
		{
			numOfPackets++;
		}

		printf("%d\n", fileSize);
		printf("%d\n", numOfPackets);

		// rewind(filePointer);

		for (i = 0; i < fileSize; i += PACKET_LEN)
		{
			// printf("I am in for loop\n");
			// printf("%d\n", i);
			bzero(rbuf, 100);
			bzero(fbuf, 101);
			fseek(filePointer, i, SEEK_SET);						// seek 99 bytes every iteration to send data in packets
			bytesRead = fread(rbuf, sizeof(char), PACKET_LEN, filePointer); // returns same number as elements if successful
			// printf("I am reading\n");
			// printf("%s\n", rbuf);
			fbuf[0] = 'C';
			fbuf[1] = '\0';
			strcat(fbuf, rbuf);
			
			fprintf(stderr, "This is being sent: %s\n", fbuf);
			write(sd, fbuf, 101);
		}

		fclose(filePointer);
	}

	close(sd);

	return (0);
}

int peerClient(int sd, char *contentname, pdu spdu)
{
	int n, i, bytes_to_read;
	int port;
	struct hostent *hp;
	struct sockaddr_in server;
	char *host, *bp, rbuf[BUFLEN], sbuf[101], hello[6], fbuf[100], cbuf[100];
	FILE *filePointer;
	fprintf(stderr, "Attempting to download file.\n");
	filePointer = fopen(contentname, "w"); // create new file and open for
	while (n = read(sd, sbuf, 101))
	{
		fprintf(stderr, "This is what was received: %s\n", sbuf);
		
		for (i = 1; i < 101; i++)
		{
			cbuf[i-1] = sbuf[i];
		}
		fprintf(stderr, "This is what is being put in file: %s\n", cbuf);
		fputs(cbuf, filePointer);
		//}
		bzero(sbuf, 101);
		bzero(cbuf, 100);
		// fclose(filePointer);
	}
	
	fprintf(stderr, "file %s downloaded: ", contentname);
	fclose(filePointer);

	close(sd);
	return (0);
}