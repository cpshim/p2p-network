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

void printStructs();
void init_lastUsed(int contentNum);
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
/*------------------------------------------------------------------------
 * main - Iterative Index Server
 *------------------------------------------------------------------------
 */
int main(int argc, char *argv[])
{
	struct sockaddr_in fsin; /* the from address of a client	*/

	char rbuf[101], sbuf[101]; /* "input" buffer; any size > 0	*/
	char *pts;
	char readPeerName[10], readContentName[10], readPort[10], readAddress[70];
	int sock;																		 /* server socket		*/
	int alen;																		 /* from-address length		*/
	int numContentOne = 0, numContentTwo = 0, numContentThree = 0, numOfContent = 0; // how many items are in database
	int endOfPeerName = 0, endOfContentName = 0, endOfPort = 0;
	int deleteContentIndex = 0, deletePeerIndex = 0;
	struct sockaddr_in sin; /* an Internet endpoint address         */
	int s, type;			/* socket descriptor and socket type    */
	int port = 3000;
	int i, j, t, bytesRead, duplicateContent, duplicatePeer, duplicatePeerIndex, duplicateContentIndex;
	struct pdu spdu, rpdu;
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
		fprintf(stderr, "bzero spdu.data\n");
		bzero(spdu.data, sizeof(spdu.data));

		if (recvfrom(s, &rpdu, sizeof(rpdu), 0, (struct sockaddr *)&fsin, &alen) < 0)
			fprintf(stderr, "recvfrom error\n");

		fprintf(stderr, "Recieve type %c\n", rpdu.type);
		fprintf(stderr, "Recieve data %s\n", rpdu.data);

		switch (rpdu.type)
		{
		case 'R':
			//Case R, Register File
			bzero(spdu.data, sizeof(spdu.data));
			fprintf(stderr, "Case R \n");
			endOfPeerName = 0;
			endOfContentName = 0;
			endOfPort = 0;
			i = 0;
			fprintf(stderr, "Before readPeerName while \n");
			// Read peername, contentname and port number
			while (rpdu.data[i] != '$')
			{
				readPeerName[i] = rpdu.data[i];
				i++;
			}
			readPeerName[i] = '\0';
			i++;
			t = 0;
			fprintf(stderr, "Before readContentName while \n");
			while (rpdu.data[i] != '$')
			{
				readContentName[t] = rpdu.data[i];
				i++;
				t++;
			}
			readContentName[t] = '\0';
			i++;
			t = 0;
			fprintf(stderr, "Before readPort while \n");
			while (rpdu.data[i] != '\0')
			{
				readPort[t] = rpdu.data[i];
				i++;
				t++;
			}
			fprintf(stderr, "After readPort while \n");
			readPort[t] = '\0';

			fprintf(stderr, "readPort: %s\n", readPort);
			fprintf(stderr, "readContentName: %s\n", readContentName);
			fprintf(stderr, "readPeerName: %s\n", readPeerName);
			char *readAddr = inet_ntoa(fsin.sin_addr);

			duplicatePeer = 0;
			duplicateContent = 0;
			// Check for duplicated peers or content name
			// If peer is duplicated, that means that the content name is in the database
			for (i = 0; i < MAX_NUM_OF_CONTENT; i++)
			{
				if (strcmp(readContentName, contentList[i].contentName) == 0)
				{
					fprintf(stderr, "Is duplicate\n");
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
			}
			// Condition for duplicated Registration
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
				// Condition if the content is already in the server
				if (duplicateContent)
				{
					if (contentList[duplicateContentIndex].numOfPeer < MAX_NUM_OF_PEER)
					{
						strcpy(contentList[duplicateContentIndex].peerList[contentList[duplicateContentIndex].numOfPeer].peerName, readPeerName);
						strcpy(contentList[duplicateContentIndex].peerList[contentList[duplicateContentIndex].numOfPeer].port, readPort);
						strcpy(contentList[duplicateContentIndex].peerList[contentList[duplicateContentIndex].numOfPeer].address, readAddr);
						init_lastUsed(duplicateContentIndex);
						contentList[duplicateContentIndex].peerList[contentList[duplicateContentIndex].numOfPeer].lastUsed = 1;
						contentList[duplicateContentIndex].numOfPeer++;

						spdu.type = 'A';
						strcpy(spdu.data, "Register Success\0");
						sendto(s, &spdu, sizeof(spdu), 0,
							   (struct sockaddr *)&fsin, sizeof(fsin));
					}
					else
					{
						spdu.type = 'E';
						strcpy(spdu.data, "Error: No more space left in database");
						fprintf(stderr, "%s\n", spdu.data);
						sendto(s, &spdu, sizeof(spdu), 0,
							   (struct sockaddr *)&fsin, sizeof(fsin));
						break;
					}
				}
				// Condition if the content is new
				else
				{
					if (numOfContent < MAX_NUM_OF_CONTENT)
					{
						// Add content to the next spot of the data base
						fprintf(stderr, "Enter into Database\n");
						fprintf(stderr, "readPeerName: %s\n", readPeerName);
						fprintf(stderr, "readContentName: %s\n", readContentName);
						fprintf(stderr, "readPort: %s\n", readPort);
						fprintf(stderr, "readAddr: %s\n", readAddr);

						strcpy(contentList[numOfContent].contentName, readContentName);
						fprintf(stderr, "contentName: %s %s\n", contentList[numOfContent].contentName, readContentName);

						strcpy(contentList[numOfContent].peerList[0].peerName, readPeerName);
						fprintf(stderr, "peerName: %s %s\n", contentList[numOfContent].peerList[0].peerName, readPeerName);

						strcpy(contentList[numOfContent].peerList[0].port, readPort);
						fprintf(stderr, "port: %s %s\n", contentList[numOfContent].peerList[0].port, readPort);

						strcpy(contentList[numOfContent].peerList[0].address, readAddr);
						fprintf(stderr, "Addr: %s\n", contentList[numOfContent].peerList[0].address);

						contentList[numOfContent].numOfPeer = contentList[numOfContent].numOfPeer + 1;

						fprintf(stderr, "numOfPeer: %d\n", contentList[numOfContent].numOfPeer);
						init_lastUsed(numOfContent);
						contentList[numOfContent].peerList[0].lastUsed = 1;
						fprintf(stderr, "ContentName:%s, PeerName: %s, Port: %s, Number of Peers: %d\n",
								contentList[numOfContent].contentName,
								contentList[numOfContent].peerList[0].peerName,
								contentList[numOfContent].peerList[0].port,
								contentList[numOfContent].numOfPeer);
						numOfContent += 1;
						fprintf(stderr, "numOfContent:%d", numOfContent);
						spdu.type = 'A';
						strcpy(spdu.data, "Register Success\0");
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
		case 'T':
			// Case T, Remove peer. 
			i = 0;
			int searchDelete = 0;
			bzero(spdu.data, sizeof(spdu.data));
			strcpy(spdu.data, "\0");
			bzero(readPeerName, sizeof(readPeerName));
			bzero(readContentName, sizeof(readContentName));
			// Read peername and contentname
			fprintf(stderr, "Before readPeerName while \n");
			while (rpdu.data[i] != '$')
			{
				readPeerName[i] = rpdu.data[i];
				i++;
			}
			readPeerName[i] = '\0';
			fprintf(stderr, "readPeerName: %s\n", readPeerName);
			i++;
			t = 0;
			fprintf(stderr, "Before readContentName while \n");
			while (rpdu.data[i] != '\0')
			{
				readContentName[t] = rpdu.data[i];
				i++;
				t++;
			}
			// Find indexes of peer and content to be deleted
			for (i = 0; i < MAX_NUM_OF_CONTENT; i++)
			{
				if (strcmp(readContentName, contentList[i].contentName) == 0)
				{
					fprintf(stderr, "Is duplicate\n");
					for (j = 0; j < MAX_NUM_OF_PEER; j++)
					{
						if (strcmp(readPeerName, contentList[i].peerList[j].peerName) == 0)
						{
							searchDelete = 1;
							deleteContentIndex = i;
							deletePeerIndex = j;
							break;
						}
					}
				}
			}
			fprintf(stderr, "deleteContentIndex: %d, deletePeerIndex: %d, searchDelete: %d\n", deleteContentIndex, deletePeerIndex, searchDelete);
			// If indexes are found: move the next peer into the previous peer until last peer
			if (searchDelete)
			{
				int *temp;
				for (i = deletePeerIndex; i < MAX_NUM_OF_PEER - 1; i++)
				{
					if ((strcmp(contentList[deleteContentIndex].peerList[i + 1].peerName, "")) || (i == deletePeerIndex))
					{
						fprintf(stderr, "Addr: %s %s\n", contentList[deleteContentIndex].peerList[i].address, contentList[deleteContentIndex].peerList[i + 1].address);
						strcpy(contentList[deleteContentIndex].peerList[i].address, contentList[deleteContentIndex].peerList[i + 1].address);
						fprintf(stderr, "%s %s\n", contentList[deleteContentIndex].peerList[i].address, contentList[deleteContentIndex].peerList[i + 1].address);

						fprintf(stderr, "Peer %s %s\n", contentList[deleteContentIndex].peerList[i].peerName, contentList[deleteContentIndex].peerList[i + 1].peerName);
						strcpy(contentList[deleteContentIndex].peerList[i].peerName, contentList[deleteContentIndex].peerList[i + 1].peerName);
						fprintf(stderr, "%s %s\n", contentList[deleteContentIndex].peerList[i].peerName, contentList[deleteContentIndex].peerList[i + 1].peerName);

						fprintf(stderr, "Port %s %s\n", contentList[deleteContentIndex].peerList[i].port, contentList[deleteContentIndex].peerList[i + 1].port);
						strcpy(contentList[deleteContentIndex].peerList[i].port, contentList[deleteContentIndex].peerList[i + 1].port);
						fprintf(stderr, "%s %s\n", contentList[deleteContentIndex].peerList[i].port, contentList[deleteContentIndex].peerList[i + 1].port);

						contentList[deleteContentIndex].peerList[i].lastUsed = contentList[deleteContentIndex].peerList[i + 1].lastUsed;
					}
					else
					{
						break;
					}
				}

				// de-init the last peer.
				if (strcmp(contentList[deleteContentIndex].peerList[i].peerName, ""))
				{
					strcpy(contentList[deleteContentIndex].peerList[i].address, "");
					strcpy(contentList[deleteContentIndex].peerList[i].peerName, "");
					strcpy(contentList[deleteContentIndex].peerList[i].port, "");
					contentList[deleteContentIndex].peerList[i].lastUsed = 0;
				}
				contentList[deleteContentIndex].numOfPeer -= 1;
				numOfContent -= 1;
				if (strcmp(contentList[deleteContentIndex].peerList[0].peerName, "") == 0)
				{
					strcpy(contentList[deleteContentIndex].contentName, "");
				}
				// Send Ack
				spdu.type = 'A';
				strcpy(spdu.data, "Delete Success\0");
				sendto(s, &spdu, sizeof(spdu), 0,
					   (struct sockaddr *)&fsin, sizeof(fsin));
			}
			else
			{
				spdu.type = 'E';
				strcpy(spdu.data, "Error: No content exists in index for this request");
				fprintf(stderr, "%s\n", spdu.data);
				sendto(s, &spdu, sizeof(spdu), 0,
					   (struct sockaddr *)&fsin, sizeof(fsin));
			}
			printStructs();
			break;

		case 'S':
			i = 0;
			bzero(spdu.data, sizeof(spdu.data));
			strcpy(spdu.data, "\0");
			bzero(readPeerName, sizeof(readPeerName));
			bzero(readContentName, sizeof(readContentName));
			int minLastUse = MAX_NUM_OF_PEER;
			int indexLastUsePeer = 0;
			int indexLastUseContent = 0;
			int searchSuccess = 0;
			fprintf(stderr, "Before readPeerName while \n");
			while (rpdu.data[i] != '$')
			{
				readPeerName[i] = rpdu.data[i];
				i++;
			}
			readPeerName[i] = '\0';
			fprintf(stderr, "readPeerName: %s\n", readPeerName);
			i++;
			t = 0;
			fprintf(stderr, "Before readContentName while \n");
			while (rpdu.data[i] != '$')
			{
				readContentName[t] = rpdu.data[i];
				i++;
				t++;
			}
			readContentName[t] = '\0';
			fprintf(stderr, "readContentName: %s\n", readContentName);
			for (i = 0; i < MAX_NUM_OF_CONTENT; i++)
			{
				if (strcmp(readContentName, contentList[i].contentName) == 0)
				{
					fprintf(stderr, "Is duplicate");
					for (j = 0; j < MAX_NUM_OF_PEER; j++)
					{
						if (contentList[i].peerList[j].lastUsed == 1)
						{
							indexLastUsePeer = j;
							indexLastUseContent = i;
							searchSuccess = 1;
							break;
						}
					}
				}
			}

			if (indexLastUsePeer > 0)
			{
				init_lastUsed(indexLastUseContent);
				contentList[indexLastUseContent].peerList[indexLastUsePeer].lastUsed = 0;
				contentList[indexLastUseContent].peerList[indexLastUsePeer - 1].lastUsed = 1;
			}
			if (searchSuccess == 1)
			{
				spdu.type = 'S';
				strcat(spdu.data, contentList[indexLastUseContent].peerList[indexLastUsePeer].peerName);
				strcat(spdu.data, "$");
				strcat(spdu.data, contentList[indexLastUseContent].peerList[indexLastUsePeer].address);
				strcat(spdu.data, "$");
				strcat(spdu.data, contentList[indexLastUseContent].peerList[indexLastUsePeer].port);
				(void)sendto(s, &spdu, sizeof(spdu), 0, (struct sockaddr *)&fsin, sizeof(fsin));
			}
			else
			{
				spdu.type = 'E';
				strcpy(spdu.data, "Error: No content exists in index for this request");
				fprintf(stderr, "%s\n", spdu.data);
				sendto(s, &spdu, sizeof(spdu), 0,
					   (struct sockaddr *)&fsin, sizeof(fsin));
			}

			break;
		case 'O':
			// Case O, show registered files
			fprintf(stderr, "Case O\n");
			bzero(spdu.data, sizeof(spdu.data));
			strcpy(spdu.data, "\0");
			spdu.type = 'O';
			
			//Parse throught the contentent database
			for (i = 0; i < MAX_NUM_OF_CONTENT; i++)
			{
				if (strcmp(contentList[i].contentName, "") != 0)
				{
					fprintf(stderr, "Found Content\n");
					strcat(spdu.data, contentList[i].contentName);
					strcat(spdu.data, "\n");
				}
			}
			//Send Packet with the registered content name. 
			fprintf(stderr, "Sending: %c %s", spdu.type, spdu.data);
			sendto(s, &spdu, sizeof(spdu), 0, (struct sockaddr *)&fsin, sizeof(fsin));
			break;
		case 'E':
			// Case E, ERROR
			fprintf(stderr, "Case E\n");
			fprintf(stderr, "Error Received from Client: %s\n", rpdu.data);
			spdu.type = 'E';
			strcpy(spdu.data, "Error: Error case");
			fprintf(stderr, "%s\n", spdu.data);
			sendto(s, &spdu, sizeof(spdu), 0,
				   (struct sockaddr *)&fsin, sizeof(fsin));
			break;
		default:
			// Case default, ERROR should not get here
			fprintf(stderr, "Default\n");
			spdu.type = 'E';
			strcpy(spdu.data, "Error: Default should not get here");
			fprintf(stderr, "%s\n", spdu.data);
			sendto(s, &spdu, sizeof(spdu), 0,
				   (struct sockaddr *)&fsin, sizeof(fsin));
			break;
		}
	}
}

// print structures debug
// For debug purposes to print the contents of the INDEX server
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
// init_lastUsed value
// change lastUsed value to 0
void init_lastUsed(int contentNum)
{
	printf("Looking for the next unused Peer.\n");
	for (int j = 0; j < MAX_NUM_OF_PEER; j++)
	{
		contentList[contentNum].peerList[j].lastUsed = 0;
	}
}