/* time_server.c - main */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>
#include <strings.h>
#include <math.h>
#include <arpa/inet.h>
#define MAX_NUM_OF_PEER 3
#define MAX_NUM_OF_CONTENT 5

/*------------------------------------------------------------------------
 * main - Iterative UDP server for TIME service
 *------------------------------------------------------------------------
 */
void printStructs();

typedef struct
{
	char peerName[10];
	char port[10];
	char address[80];
	int lastUsed;
} Peer;
typedef struct
{
	Peer peerList[MAX_NUM_OF_PEER];
	char contentName[10];
	int numOfPeer;
} Content;

Content contentList[MAX_NUM_OF_CONTENT];
struct pdu
{
	char type;
	char data[100];
};
int main(int argc, char *argv[])
{
	struct sockaddr_in fsin; /* the from address of a client	*/

	char rbuf[101], sbuf[101]; /* "input" buffer; any size > 0	*/
	char *pts;
	char readPeerName[10], readContentName[10], readPort[10], readAddress[70];
	int sock; /* server socket		*/
	// time_t now;																		 /* current time			*/
	int alen;																		 /* from-address length		*/
	int numContentOne = 0, numContentTwo = 0, numContentThree = 0, numOfContent = 0; // how many items are in database
	int endOfPeerName = 0, endOfContentName = 0, endOfPort = 0;
	int deleteContentIndex = 0, deletePeerIndex = 0;
	struct sockaddr_in sin; /* an Internet endpoint address         */
	int s, type;			/* socket descriptor and socket type    */
	int port = 3000;
	int i, j, t, bytesRead, duplicateContent, duplicatePeer, duplicatePeerIndex, duplicateContentIndex;
	struct pdu spdu, rpdu;
	// struct content a, b, c, d, e, f, g;
	// struct stat fileStats;
	// struct content database[7];
	//  char database[10][100][2];  //database only able to hold 10 unique users and 100 unique content name and 1 address; might need to make dynamic array
	int fileSize;
	FILE *filePointer;

	switch (argc)
	{
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
		fprintf(stderr, "can't bind to %d port\n", port);
	listen(s, 5);
	alen = sizeof(fsin);

	fprintf(stderr, "Before Init");
	//---------------initialize database-----------------------
	for (i = 0; i < MAX_NUM_OF_CONTENT; i++)
	{
		strcpy(contentList[i].contentName, "");
		contentList[i].numOfPeer = 0;
		fprintf(stderr, "Init Content");
		for (j = 0; j < MAX_NUM_OF_PEER; j++)
		{
			fprintf(stderr, "Init Peer");
			strcpy(contentList[i].peerList[j].peerName, "");
			strcpy(contentList[i].peerList[j].address, "");
			strcpy(contentList[i].peerList[j].port, "");
			contentList[i].peerList[j].lastUsed = 0;
		}
	}
	printStructs();
	while (1)
	{   
		
		fprintf(stderr, "bzero rbuf\n");
		bzero(rbuf, sizeof(rbuf));
		fprintf(stderr, "bzero rpdu.data\n");
		bzero(rpdu.data, sizeof(rpdu.data));
		//fprintf(stderr, "bzero rpdu.type\n");
		//bzero(rpdu.type, sizeof(rpdu.type));
		fprintf(stderr, "bzero spdu.data\n");
		bzero(spdu.data, sizeof(spdu.data));
		//fprintf(stderr, "bzero spdu.type\n");
		//bzero(spdu.type, sizeof(spdu.type));

		fprintf(stderr, "before receive\n");
		fprintf(stderr, "size of rpdu: %d\n", sizeof(rpdu));
		if (recvfrom(s, &rpdu, sizeof(rpdu), 0, (struct sockaddr *)&fsin, &alen) < 0)
			fprintf(stderr, "recvfrom error\n");

		//bzero(rbuf, sizeof(rbuf));
		// if (recvfrom(s, rbuf, sizeof(rbuf), 0,
		// 			 (struct sockaddr *)&fsin, &alen) < 0)
		// 	fprintf(stderr, "recvfrom error\n");
		// int k;
		// char assignPDU[101];
		// if (rbuf[0] == 'R' || rbuf[0] == 'S' || rbuf[0] == 'T' || rbuf[0] == 'O' || rbuf[0] == 'E')
		// {
		// 	for (k = 0; k < 100; k++)
		// 		assignPDU[k] = rbuf[k + 1];
		// 	assignPDU[101] = '\0';
		// 	strcpy(rpdu.data, assignPDU);
		// 	rpdu.type = rbuf[0];
		// 	printf("Recieved PDU\ntype: %c\ntype:%s\n", rpdu.type, rpdu.data);
		// }
		// else
		// {
		// 	printf("Error PDU type %c received\n", rbuf[0]);
		// }
		// rpdu.type = rbuf[0];
		// char assignPDU[101];
		// for (i = 0; i < 100; i++)
		// 	assignPDU[i] = rbuf[i + 1];
		// assignPDU[101] = '\0';
		// strcpy(rpdu.data, assignPDU);
		fprintf(stderr, "Recieve type %c\n", rpdu.type);
		fprintf(stderr, "Recieve data %s\n", rpdu.data);

		switch (rpdu.type)
		{
		case 'R':
			fprintf(stderr, "Case R \n");
			endOfPeerName = 0;
			endOfContentName = 0;
			endOfPort = 0;
			// need dashes or some limiter to determine when end of name is
			i = 0;
			fprintf(stderr, "Before readPeerName while \n");
			while(rpdu.data[i] != '$'){
				readPeerName[i] = rpdu.data[i];
				i++;
			}
			readPeerName[i] = '\0';
			i++;
			t = 0;
			fprintf(stderr, "Before readContentName while \n");
			while(rpdu.data[i] != '$'){
				readContentName[t] = rpdu.data[i];
				i++;
				t++;
			}
			readContentName[t] = '\0';
			i++;
			t = 0;
			fprintf(stderr, "Before readPort while \n");
			while(rpdu.data[i] != '\0'){
				readPort[t] = rpdu.data[i];
				i++;
				t++;
			}
			fprintf(stderr, "After readPort while \n");
			readPort[t] = '\0';
			
			// for (i = 0; i < 100; i++)
			// {
			// 	fprintf(stderr, "In for loop %d\n", i);
			// 	if (rpdu.data[i] == '$' && (endOfPeerName != 0))
			// 	{
			// 		endOfContentName = i;
					
			// 	}
			// 	else if ((rpdu.data[i] == '$'))
			// 	{
			// 		endOfPeerName = i;
			// 	}
			// 	else if (rpdu.data[i] == '\0')
			// 	{
			// 		endOfPort = i;
			// 	}
			// }
			
			fprintf(stderr, "readPort: %s\n",readPort);
			fprintf(stderr, "readContentName: %s\n",readContentName);
			fprintf(stderr, "readPeerName: %s\n",readPeerName);
			char *readAddr = inet_ntoa(fsin.sin_addr);
			
			// put for loop into a function to call
			// if (checkInDatabase(readPeerName, readContentName, database))

			duplicatePeer = 0;
			duplicateContent = 0;
			//printStructs();
			

			for (i = 0; i < MAX_NUM_OF_CONTENT; i++)
			{	
				fprintf(stderr, "Check duplicate");
				if (strcmp(readContentName, contentList[i].contentName) == 0)
				{
					fprintf(stderr, "Is duplicate");
					duplicateContentIndex = i;
					duplicateContent = 1;
					
					for (j = 0; j < MAX_NUM_OF_PEER; j++)
					{
						if (strcmp(readPeerName, contentList[i].peerList[j].peerName) == 0)
						{
							duplicatePeer = 1;
						}
					}
				}
				// if (strcmp(readPeerName, peerList[i].peerName) != 0)
				// { // strcmp output 0 if true
				// 	duplicatePeer = 1;
				// 	duplicatePeerIndex = i;
				// 	for (j = 0; j < 5; j++)
				// 	{
				// 		if (strcmp(readContentName, peerList[i].database[j].contentName) != 0)
				// 		{
				// 			duplicateContent = 1;
				// 		}
				// 	}
				// }
			}
			
			if (duplicatePeer)
			{
				spdu.type = 'E';
				strcpy(spdu.data, "Error: This peer already registered this content");
				fprintf(stderr, "%s\n", spdu.data);
				sendto(s, &spdu, sizeof(spdu), 0,
					   (struct sockaddr *)&fsin, sizeof(fsin));
			}
			else
			{
				if (duplicateContent)
				{

					if (contentList[i].numOfPeer < MAX_NUM_OF_PEER)
					{
						strcpy(contentList[duplicateContentIndex].peerList[contentList[duplicateContentIndex].numOfPeer].peerName, readPeerName);
						strcpy(contentList[duplicateContentIndex].peerList[contentList[duplicateContentIndex].numOfPeer].port, readPort);
						strcpy(contentList[duplicateContentIndex].peerList[contentList[duplicateContentIndex].numOfPeer].address, readAddr);
						contentList[duplicateContentIndex].numOfPeer++;
						// fprintf(stderr, "ContentName:%s, PeerName: %s, Port: %s, Number of Peers: %s\n",
						// 		contentList[duplicateContentIndex].contentName,
						// 		contentList[duplicateContentIndex].peerList[0].peerName,
						// 		contentList[duplicateContentIndex].peerList[0].address,
						// 		contentList[duplicateContentIndex].numOfPeer);
					}
					else
					{
						spdu.type = 'E';
						strcpy(spdu.data, "Error: No more space left in database");
						// fprintf(stderr, "%s\n", spdu.data);
						// sendto(s, &spdu, sizeof(spdu), 0,
						// 	   (struct sockaddr *)&fsin, sizeof(fsin));
					}
				}
				else
				{
					if (numOfContent < MAX_NUM_OF_CONTENT)
					{
						fprintf(stderr, "Enter into Database\n");
						fprintf(stderr, "readPeerName: %s\n",readPeerName);
						fprintf(stderr, "readContentName: %s\n",readContentName);
						fprintf(stderr, "readPort: %s\n",readPort);
						fprintf(stderr, "readAddr: %s\n",readAddr);

						strcpy(contentList[numOfContent].contentName, readContentName);
						fprintf(stderr, "contentName: %s %s\n",contentList[numOfContent].contentName,readContentName);
						
						strcpy(contentList[numOfContent].peerList[0].peerName, readPeerName);
						fprintf(stderr, "peerName: %s %s\n",contentList[numOfContent].peerList[0].peerName,readPeerName);
						
						strcpy(contentList[numOfContent].peerList[0].port, readPort);
						fprintf(stderr, "port: %s %s\n",contentList[numOfContent].peerList[0].port,readPort);

						strcpy(contentList[numOfContent].peerList[0].address, readAddr);
						fprintf(stderr, "Addr: %s\n",contentList[numOfContent].peerList[0].address);

						contentList[numOfContent].numOfPeer = contentList[numOfContent].numOfPeer + 1;

						fprintf(stderr, "numOfPeer: %d\n",contentList[numOfContent].numOfPeer);

						fprintf(stderr, "ContentName:%s, PeerName: %s, Port: %s, Number of Peers: %d\n",
								contentList[numOfContent].contentName,
								contentList[numOfContent].peerList[0].peerName,
								contentList[numOfContent].peerList[0].port,
								contentList[numOfContent].numOfPeer);
						numOfContent+=1;
						fprintf(stderr, "numOfContent:%d", numOfContent);
						spdu.type = 'A';
						strcpy(spdu.data, "Success\0");
						sendto(s, &spdu, sizeof(spdu), 0,
							   (struct sockaddr *)&fsin, sizeof(fsin));
					}
					else
					{
						spdu.type = 'E';
						strcpy(spdu.data, "Error: No more space left for new peers");
						fprintf(stderr, "%s\n", spdu.data);
						sendto(s, &spdu, sizeof(spdu), 0,
							   (struct sockaddr *)&fsin, sizeof(fsin));
					}
				}
			}
			printStructs();
			break;
			// case 'T':
			// 	strncpy(readPeerName, rpdu.data, 10);
			// 	strncpy(readContentName, rpdu.data + 10, 10);
			// 	for (i = 0; i < 5; i++)
			// 	{
			// 		if (strcmp(readContentName, contentList[i].contentName) != 0)
			// 		{
			// 			deleteContentIndex = i;
			// 			for (j = 0; j < 3; j++)
			// 			{
			// 				if (strcmp(readPeerName, contentList[i].peerList[j].peerName) != 0)
			// 				{
			// 					deletePeerIndex = j;
			// 					break;
			// 				}
			// 			}
			// 		}
			// 	}

			// 	for (i = deletePeerIndex; i < 3; i++)
			// 	{
			// 		contentList[deleteContentIndex].peerList[i] = contentList[deleteContentIndex].peerList[i + 1];
			// 	}
			// 	contentList[deleteContentIndex].numOfPeer -= 1;
			// 	break;

			// case 'C':
			// 	filePointer = fopen(rpdu.data, "r");

			// 	if (filePointer == NULL)
			// 	{
			// 		spdu.type = 'E';
			// 		strcpy(spdu.data, "Error opening file");
			// 		fprintf(stderr,"%s\n", spdu.data);
			// 		sendto(s, &spdu, sizeof(spdu), 0,
			// 			   (struct sockaddr *)&fsin, sizeof(fsin));
			// 	}

			// 	if (filePointer != NULL)
			// 	{
			// 		lstat(rpdu.data, &fileStats);
			// 		fileSize = (int)fileStats.st_size;

			// 		for (i = 0; i < fileSize; i += 99)
			// 		{
			// 			fseek(filePointer, i, SEEK_SET);
			// 			bytesRead = fread(rbuf, sizeof(rbuf) - 1, 1, filePointer);

			// 			strcpy(spdu.data, rbuf);

			// 			if (feof(filePointer))
			// 			{
			// 				spdu.type = 'F';
			// 			}
			// 			else
			// 			{
			// 				spdu.type = 'D';
			// 			}

			// 			sendto(s, &spdu, sizeof(spdu), 0,
			// 				   (struct sockaddr *)&fsin, sizeof(fsin));

			// 			bzero(spdu.data, 100);
			// 			bzero(rbuf, 100);
			// 		}
			// 	}
			// 	fclose(filePointer);
			// 	break;
		case 'S':
			i = 0;
			bzero(spdu.data, sizeof(spdu.data));
			strcpy(spdu.data, "\0");
			spdu.type = 'O';
			bzero(readPeerName, sizeof(readPeerName));
			bzero(readContentName, sizeof(readContentName));
			int minLastUse = MAX_NUM_OF_PEER;
			int indexLastUsePeer = 0;
			int indexLastUseContent = 0;
			fprintf(stderr, "Before readPeerName while \n");
			while(rpdu.data[i] != '$'){
				readPeerName[i] = rpdu.data[i];
				i++;
			}
			readPeerName[i] = '\0';
			fprintf(stderr, "readPeerName: %s\n",readPeerName);
			i++;
			t = 0;
			fprintf(stderr, "Before readContentName while \n");
			while(rpdu.data[i] != '\0'){
				readContentName[t] = rpdu.data[i];
				i++;
				t++;
			}
			readContentName[t] = '\0';
			fprintf(stderr, "readContentName: %s\n",readContentName);
			for (i = 0; i < MAX_NUM_OF_CONTENT; i++)
			{	
				fprintf(stderr, "Check duplicate");
				if (strcmp(readContentName, contentList[i].contentName) == 0)
				{
					fprintf(stderr, "Is duplicate");
					for (j = 0; j < MAX_NUM_OF_PEER; j++)
					{
						if(contentList[i].peerList[j].lastUsed == 1)
						{
							indexLastUsePeer = j;
							indexLastUseContent = i;
							break;
						}
					}
				}
			}
			strcat(spdu.data, contentList[indexLastUseContent].peerList[indexLastUsePeer].address);
			strcat(spdu.data, "$");
			strcat(spdu.data, contentList[indexLastUseContent].peerList[indexLastUsePeer].port);
			(void)sendto(s, &spdu, sizeof(spdu), 0, (struct sockaddr *)&fsin, sizeof(fsin));
			break;
		case 'O':
			fprintf(stderr,"Case O\n");
			bzero(spdu.data, sizeof(spdu.data));
			strcpy(spdu.data, "\0");
			spdu.type = 'O';
			for (i = 0; i < MAX_NUM_OF_CONTENT; i++)
			{	 
				fprintf(stderr, "Check duplicate");
				if (strcmp(contentList[i].contentName, "") != 0)
				{
					strcat(spdu.data, contentList[i].contentName);
					strcat(spdu.data, "\n");
				}
			}
			fprintf(stderr,"Sending: %c %s", spdu.type, spdu.data);
			sendto(s, &spdu, sizeof(spdu), 0, (struct sockaddr *)&fsin, sizeof(fsin));
			break;
		case 'E':
			fprintf(stderr, "Error\n");
			break;
		default:
			fprintf(stderr, "Default\n");
			break;
		}
	}
}
// print structures debug
void printStructs()
{
	printf("\n");
	printf("Struct Array is\n\n");
	for (int i = 0; i < MAX_NUM_OF_CONTENT; i++)
	{
		printf("Content: %s NumOfPeers: %d\n", contentList[i].contentName, contentList[i].numOfPeer);
		printf("Peers: \n");
		for (int j = 0; j < MAX_NUM_OF_PEER; j++)
		{
			printf("%s %s %s", contentList[i].peerList[j].peerName, contentList[i].peerList[j].port, contentList[i].peerList[j].address);
			if ((strcmp(contentList[i].peerList[j].peerName, "")))
			{
				printf(" Last Used %d\n", contentList[i].peerList[j].lastUsed);
			}
			else
			{
				printf("\n");
			}
		}
		printf("\n");
	}
}
// int checkInDatabase(char readPeerName[], char readContentName[], struct content *database[])
// {
// 	int i;
// 	char peerName[10], contentName[10];
// 	for (i = 0; i < 7; i++)
// 	{
// 		strcpy(peerName, database[i].peerName);
// 		if (strcmp(readPeerName, peerName) != 0)
// 		{ // strcmp output 0 if true
// 			if (strcmp(readContentName, *database[i].contentName) != 0)
// 			{
// 				return 1;
// 			}
// 		}
// 	}
// 	return 0;
// }