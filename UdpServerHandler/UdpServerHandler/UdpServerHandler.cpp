/*
SERVER: start this first!
SOCKET: endpoint-mechanism in which an OS connects to the Network
Revisions:
January 8, 2018
- implementing file access R/W to SD Card
- need partition (FAT32 or NTFS)
- source: https://technet.microsoft.com/en-us/library/cc938440.aspx
- FAT32 is faster access compared to NTFS for small files
*/

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <Windows.h>
#include <winioctl.h>
#include <WinSock2.h>
#include <iphlpapi.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <tchar.h>
#include <strsafe.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{

	WSADATA WSAData;
	SOCKET server, client;
	SOCKADDR_IN	serverAddr, clientAddr;

	string answer;

	TCHAR szBuffer[256];
	char DataBuffer[] = "This is some test data to write to the file on the SD card via USB3. Ronnie Wong was just here!";
	DWORD dwBytesToWrite = (DWORD)strlen(DataBuffer);	// lenght of DataBuffer
	DWORD dwBytesWritten = 0;
	BOOL bErrorFlag = FALSE;
	
	// Update: need to concatenate _tchar type strings
	
	LPCWSTR fname = L"test5.txt";
	string prepend = "\\\\.\\E:\\";
	string filename, concat;

	printf("Please enter filename to save predefined string: ");
	getline(cin, filename);

	concat = prepend + filename;

	HANDLE hFile;
	hFile = CreateFile(concat,GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	//hFile = CreateFile("\\\\.\\E:\\test6.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE) {
		printf("Terminal failure: Unable to open file for writing.\n\r");
		printf("Press enter to exit()\n\r");
		getchar();
		return 1;
	}
	else {
		printf("Connected to SD Reader/Writer on E:\n\r");
		printf("Writing %d bytes to SD card.\n\r", dwBytesToWrite);
	}

	bErrorFlag = WriteFile(hFile, DataBuffer, dwBytesToWrite, &dwBytesWritten, NULL);

	if (FALSE == bErrorFlag) {
		printf("Terminal failure: Unable to write to file.\n\r");
	}
	else {
		if (dwBytesWritten != dwBytesToWrite) {
			printf("Number of bytes written: %d\n\r", dwBytesWritten);
			printf("Error: dwBytesToWrite != dwBytesWritten\n");
		}
		else {
			printf("Wrote successfully to SD Card.\n\r");
		}
	}

	CloseHandle(hFile);


	// file test  
	//FILE *infile;

	// prepare file for writing
	//infile = fopen_s("mydata.txt", "w");
	//fprintf(infile, "Test File : January 4, 2018\n\r");

	// Initialize Winsock:
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) {
		printf("Failed. Error code: %d", WSAGetLastError());
		return 1;
	}
	printf("Winsock initialized\n\r");

	// Open socket where clients will connect to:
	if ((server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
		printf("Could not create socket: %d", WSAGetLastError());
		return 1;
	}
	printf("Socket created.\n\r");

	// Bind socket to an address:
	//serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);			// since this is the server, bind to any IP on this computer
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(11000);
	if (bind(server, (SOCKADDR *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		printf("Bind failed with error code: %d", WSAGetLastError());
	}
	puts("Bind successful.\n\r");
	printf("Listening for incoming connections...\n\r");

	// If connected with client:
	char buffer[64];
	int clientAddrSize = sizeof(clientAddr);

	while (1) {
		if (recv(server, buffer, sizeof(buffer), 0) > 0) {
			printf("Client connected!\n\r");
			cout << "Client says: " << buffer << endl;
			memset(buffer, 0, sizeof(buffer));
			//fprintf(infile, buffer);
			cout << "Do you wish to continue?";
			getline(cin, answer);
			if (answer == "no") break;  
		}
		else {
			printf("SOCKET ERROR CODE: %d", WSAGetLastError());
			printf("Press key to exit...");
			getchar();
			return 1;
		}
	}

	// close file
	//fclose(infile);

	// Exit routine:
	printf("Press enter to continue/exit()\n\r");
	getchar();

	return 0;
}