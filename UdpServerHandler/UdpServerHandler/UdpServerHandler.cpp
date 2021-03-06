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

#define UNICODE 1		// UNICODE 1 isn't required here - take out after working.
#define _UNICODE 1

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define wszDrive L"\\\\.\\PhysicalDrive0"
#define wszSDCARD L"\\\\.\\E:"

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
#include <winioctl.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

BOOL GetDriveGeometry(LPWSTR wszPath, DISK_GEOMETRY *pdg)
{
	HANDLE hDevice = INVALID_HANDLE_VALUE;
	BOOL bResult = FALSE;
	DWORD junk = 0;

	hDevice = CreateFile(wszPath,
		0,
		FILE_SHARE_READ |
		FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (hDevice == INVALID_HANDLE_VALUE) {
		return (FALSE);
	}

	bResult = DeviceIoControl(hDevice,
		IOCTL_DISK_GET_DRIVE_GEOMETRY,
		NULL, 0,
		pdg, sizeof(*pdg),
		&junk,
		(LPOVERLAPPED)NULL);

	CloseHandle(hDevice);

	return bResult;
}

int main()
{

	WSADATA WSAData;
	SOCKET server, client;
	SOCKADDR_IN	serverAddr, clientAddr;

	string answer;

	TCHAR szBuffer[256];
	char DataBuffer[] = "This is some test data to write to the file on the SD card via USB3. Ronnie Wong was just here!\r\n";
	DWORD dwBytesToWrite = (DWORD)strlen(DataBuffer);	// length of DataBuffer
	DWORD dwBytesWritten = 0;
	BOOL bErrorFlag = FALSE;

	DWORD dwBytesToWriteETH;
	DWORD dwBytesWrittenETH;

	DISK_GEOMETRY pdg = { 0 };
	BOOL bResult = FALSE;
	ULONGLONG DiskSize = 0;

	bResult = GetDriveGeometry(wszSDCARD, &pdg);

	if (bResult) {
		printf("Drive path      = %12ws\n",wszSDCARD);
		printf("Cylinders       = %12ld\n",pdg.Cylinders);
		printf("Tracks/cylinder = %12ld\r\n",(ULONG)pdg.TracksPerCylinder);
		printf("Sectors/track	= %12ld\r\n",(ULONG)pdg.SectorsPerTrack);
		printf("Bytes/sector    = %12ld\r\n",(ULONG)pdg.BytesPerSector);
		
		DiskSize = pdg.Cylinders.QuadPart * (ULONG)pdg.TracksPerCylinder *
			(ULONG)pdg.SectorsPerTrack * (ULONG)pdg.BytesPerSector;

		printf("Disk size       = % 10lld (Bytes)\r\n"
			   "                = %12.2f (Gb)\n",
			DiskSize, (double)DiskSize / (1024 * 1024 * 1024));
	}
	else {
		wprintf(L"GetDriveGeometry failed. Error %ld.\n", GetLastError());
	}

	HANDLE hFile;
	hFile = CreateFile(L"\\\\.\\E:\\ETHSendTest.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	//hFile = CreateFile("\\\\.\\E:\\ETHSendTest.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

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

	//CloseHandle(hFile);


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
			bErrorFlag = FALSE;

			printf("Client connected!\n\r");
			cout << "Client says: " << buffer << endl;
			//memset(buffer, 0, sizeof(buffer));	// this line commented for SD writing to work
			
			dwBytesToWriteETH = (DWORD)strlen(buffer);
			dwBytesWrittenETH = 0;

			printf("Buffer length is: %d\n\r", dwBytesToWriteETH);

			printf("Writing %d bytes to SD card.\n\r", dwBytesToWriteETH);

			bErrorFlag = WriteFile(hFile, buffer, dwBytesToWriteETH, &dwBytesWrittenETH, NULL);

			if (FALSE == bErrorFlag) {
				printf("Terminal failure: Unable to write to file.\n\r");
			}
			else {
				if (dwBytesWrittenETH != dwBytesToWriteETH) {
					printf("Number of bytes written: %d\n\r", dwBytesWrittenETH);
					printf("Error: dwBytesToWrite != dwBytesWritten\n");
				}
				else {
					printf("Wrote ETH packets successfully to SD Card.\n\r");
				}
			}

			/*
			cout << "Do you wish to continue?";
			getline(cin, answer);
			if (answer == "no") {
				CloseHandle(hFile);
				break;
			}
			*/
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