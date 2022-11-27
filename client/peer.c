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

int echod(int sd, char *contentname);
void reaper(int sig);
int TCP();
int echodrec(int sd, char *contentname, pdu spdu);
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
					fprintf(stderr, "Received Ack or Error  %c.\n", rpdu.type);
					if (rpdu.type != 'A')
					{
						// Received error
						printf("Not starting TCP.\n");
						printf("Peername already in use for this content\n");
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
						printf("Attempting to kill thread");
						closePID(contentname);
						printf("Successfully killed thread");
					}
				}
			case 'O':
				strcat(spdu.data, "\0");
				(void)write(s, &spdu, sizeof(spdu));
				bzero(rpdu.data, sizeof(rpdu.data));
				if (recvfrom(s, &rpdu, sizeof(rpdu), 0, (struct sockaddr *)&sin, &alen) < 0)
					fprintf(stderr, "recvfrom error\n");
				else
				{
					fprintf(stderr, "Received Ack or Error  %c.\n", rpdu.type);
					if (rpdu.type != 'O')
					{
						// Received error
						printf("Not starting TCP.\n");
						printf("Peername already in use for this content\n");
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
					// if not equal to null
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
								printf("Attempting to kill thread");
								closePID(contentname);
								printf("Successfully killed thread");
							}
						}
					}
				}
				break;

			// case 'O':
			// 	printf("Case O\n");
			// 	printf("Requesting content list.\n");
			// 	spdu.type='O';
			// 	bzero(spdu.data,sizeof(spdu.data));
			// 	strcat(spdu.data,"Send list of content\n");
			// 	bzero(listenBufS,sizeof(listenBufS));
			// 	(void) write(s, &spdu, strlen(spdu.data)+1);
			// 	if (recvfrom(s, listenBufS, sizeof(listenBufS), 0, (struct sockaddr *)&sin, &alen) < 0)
			// 			fprintf(stderr, "recvfrom error\n");
			// 	else{
			// 		for(i=0;i<100;i++){
			// 			rpdu.data[i]=listenBufS[i+1];
			// 		}
			// 		printf("List of registered content:\n%s", rpdu.data);
			// 	}
			// 	break;

			// case 'T':
			// 	printf("Case T\n");
			// 	printf("What content would you like to deregister\n");
			// 	scanf("%s", contentname);
			// 	spdu.type = 'T';
			// 	bzero(spdu.data,sizeof(spdu.data));
			// 	strcpy(spdu.data,peername);
			// 	strcat(spdu.data,"$");
			// 	strcat(spdu.data,contentname);
			// 	strcat(spdu.data,"$");
			// 	//sprintf(intString, "%d", portNumTCP[0]);
			// 	//strcat(spdu.data,intString);
			// 	//strcat(spdu.data,"$");
			// 	printf("%c %s should be sent.\n",spdu.type,spdu.data);
			// 	(void) write(s, &spdu, strlen(spdu.data)+1);
			// 	// get the ACK or error
			// 	bzero(listenBufR,sizeof(listenBufR));
			// 		if (recvfrom(s, listenBufR, sizeof(listenBufR), 0, (struct sockaddr *)&sin, &alen) < 0)
			// 			fprintf(stderr, "recvfrom error\n");
			// 		else {
			// 			fprintf(stderr, "Received Ack or Error %s.\n",listenBufR);
			// 			if(listenBufR[0]!='A'){
			// 				//Received error
			// 				printf("Not able to deregister\n");
			// 				break;
			// 			}
			// 			else{
			// 				printf("Attempting to close Socket for %s\n", contentname);
			// 				closePID(contentname);
			// 				printf("Finished closing socket\n");
			// 				//Received Acknowledgement

			// 			}
			// 		printf("Finishing ack processing\n");
			// 		}
			// 	break;
			// case 'Q':
			// 	if (strcmp(scannedtype, "Quit")){
			// 		printf("Invalid input, if you wish to Quit type Quit\n");
			// 		break;
			// 	}
			// 	printf("case Quit\n");
			// 	for(int k = 0; k<MAXTHREADNUM; k++){
			// 		if(strcmp(childr[k].content, "")){
			// 			spdu.type = 'T';
			// 			bzero(spdu.data,sizeof(spdu.data));
			// 			strcpy(spdu.data,peername);
			// 			strcat(spdu.data,"$");
			// 			strcat(spdu.data,childr[k].content);
			// 			strcat(spdu.data,"$");
			// 			printf("%c %s should be sent.\n",spdu.type,spdu.data);
			// 			(void) write(s, &spdu, strlen(spdu.data)+1);
			// 			// get the ACK or error
			// 			bzero(listenBufR,sizeof(listenBufR));
			// 				if (recvfrom(s, listenBufR, sizeof(listenBufR), 0, (struct sockaddr *)&sin, &alen) < 0)
			// 					fprintf(stderr, "recvfrom error\n");
			// 				else {
			// 					fprintf(stderr, "Received Ack or Error %s.\n",listenBufR);
			// 					if(listenBufR[0]!='A'){
			// 						//Received error
			// 						printf("Not able to deregister\n");
			// 					}
			// 					else{
			// 						printf("Attempting to close Socket for %s\n", childr[k].content);
			// 						closePID(childr[k].content);
			// 						printf("Finished closing socket\n");
			// 						//Received Acknowledgement

			// 					}
			// 				printf("Finishing ack processing\n");
			// 				}
			// 		}
			// 	}
			// 	break;
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
			exit(echod(new_sd, contentname));
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
	echodrec(sd, contentname, spdu);
	// check this shit
}

/*	reaper		*/
void reaper(int sig)
{
	int status;
	while (wait3(&status, WNOHANG, (struct rusage *)0) >= 0)
		;
}

int echod(int sd, char *contentname)
{
	FILE *fptr;
	char intString[20];
	char *bp, buf[BUFLEN], sbuf[PACKET_LEN], sbuf2[PACKET_LEN + 1], sbuf3[PACKET_LEN + 1];
	int n, bytes_to_read;
	bzero(sbuf3, PACKET_LEN + 1);
	for (int p = 0; p < 20; p++)
	{
		sbuf3[p] = '\0';
	}
	n = read(sd, sbuf3, PACKET_LEN + 1);
	fprintf(stderr, "Received %s PDU\n", sbuf3);

	fprintf(stderr, "Sending %s.\n", contentname);

	fptr = fopen(contentname, "r");
	if (fptr != NULL)
	{
		fprintf(stderr, "Opened %s.\n", contentname);
		int file_Size = fSize(fptr);
		int packetDist = file_Size / PACKET_LEN;
		if (file_Size % PACKET_LEN != 0)
		{
			packetDist++;
		}
		fprintf(stderr, "Sending %d packets.\n", packetDist);

		sprintf(intString, "%d", packetDist);
		write(sd, intString, sizeof(intString));
		sleep(2); // send number of packets
		for (int i = 0; i < packetDist; i++)
		{

			sbuf2[0] = 'C';
			sbuf2[1] = '\0';
			char flag[file_Size];
			fread(sbuf, sizeof(char), PACKET_LEN, fptr);
			strcat(sbuf2, sbuf);
			fprintf(stderr, "Packet %d Data being sent %s\n", i, sbuf2);
			write(sd, sbuf2, PACKET_LEN + 1);
			bzero(sbuf, PACKET_LEN);
			bzero(sbuf2, PACKET_LEN + 1);
			sleep(2);
			fseek(fptr, 0L, 100);
			fprintf(stderr, "Packet %d sent.\n", i);
		}
		fclose(fptr);
	}
	else
	{
		fprintf(stderr, "\n File %s not found", buf);
		write(sd, "?File not found\n", sizeof("?File not found\n"));
	}

	close(sd);

	return (0);
}

int echodrec(int sd, char *contentname, pdu spdu)
{
	fprintf(stderr, "Attempting to download file.\n");
	FILE *fptr;
	char intString[20];
	char *bp, buf[BUFLEN], sbuf[PACKET_LEN + 1], sbuf2[PACKET_LEN + 1], sbuf3[PACKET_LEN + 1];
	bzero(sbuf, sizeof(sbuf));
	strcat(sbuf, contentname);
	int n, bytes_to_read;
	char ecbuf[100];
	FILE *fp;
	fp = fopen(sbuf, "w");
	strcpy(sbuf3, "D");
	strcat(sbuf3, contentname);
	fprintf(stderr, "Sending PDU: %s\n", sbuf3);
	write(sd, sbuf3, PACKET_LEN + 1);
	while (n = read(sd, sbuf, 1))
	{
		if (sbuf[0] == '?')
		{
			fprintf(stderr, "This data is corrupted or not available\n");
			read(sd, sbuf2, PACKET_LEN);
			fprintf(stderr, "Error message from server: %s\n", sbuf2);
			break;
		}
		int packetnum = atoi(sbuf);
		char file[(packetnum * PACKET_LEN) + 1];
		bzero(file, sizeof(file));
		file[0] = '\0';
		bzero(sbuf, sizeof(sbuf));
		for (int i = 0; i < packetnum; i++)
		{
			while (n = read(sd, sbuf, PACKET_LEN + 1))
			{
				// strcat(sbuf,"\0");
				for (int k = 0; k < PACKET_LEN; k++)
				{
					sbuf2[k] = sbuf[k + 1];
				}
				sbuf2[PACKET_LEN] = '\0';
				if (sbuf[0] == 'C')
				{
					strcat(file, sbuf2);
					// fprintf(stderr, "Received Data %s\n", sbuf2);
					// fprintf(stderr, "File content:%s", file);
				}
				else if (sbuf[0] == '?')
				{
					fprintf(stderr, "This data is corrupted or not available\n");
					fprintf(stderr, "Error message from server: %s\n", sbuf2);
				}
			}
		}
		fprintf(stderr, "file %s downloaded: ", contentname);
		fprintf(stderr, "File content: %s", file);
		fprintf(fp, "%s", file);
	}
	fclose(fp);

	close(sd);
	return (0);
}