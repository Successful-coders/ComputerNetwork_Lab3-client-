#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment (lib,"Ws2_32.lib")
#include <WinSock2.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <string>

using namespace std;


const int M = 1000;
const int N = 256;

bool fin;


DWORD WINAPI receive(LPVOID clientSocket)
{
	int retVal;
	SOCKET clientSock;
	clientSock = *((SOCKET*)clientSocket);
	char Resp[M];
	//сообщение от сервера
	retVal = recv(clientSock, Resp, M, 0);
	//если это сообщение закрытия сервера
	if (!strcmp(Resp, "Server shutdown"))
	{
		printf("Server shutdown\n");
		fin = true;
		return 0;
	}
	//если сервер переполнен
	if (!strcmp(Resp, "Sorry, too much people on the line"))
	{
		printf("Sorry, too much people on the line\n");
		fin = true;
		return 0;
	}
	//если клиент все еще работает
	if (!fin)
	{
		if (retVal == SOCKET_ERROR)
		{
			//ошибка, завершение работы
			retVal = 0;
			printf("Unable to recv\n");
			fin = true;
			return 0;
		}
		else
		{
			//напечатать сообщение сервера
			printf("%s\n", Resp);
		}
	}
	return 1;
}

DWORD WINAPI send(LPVOID clientSocket)
{
	int retVal;
	char pBuf[M];
	SOCKET clientSock;
	clientSock = *((SOCKET*)clientSocket);
	gets_s(pBuf);
	//если введенная строка - закрытие клиента
	if (!strcmp(pBuf, "EXIT"))
	{
		fin = true;
		retVal = send(clientSock, pBuf, M, 0);
		return 0;
	}
	else
	{
		//отправить сообщение серверу
		retVal = send(clientSock, pBuf, M, 0);
		if (retVal == SOCKET_ERROR)
		{
			printf("Unable to send\n");
			WSACleanup();
			system("pause");
			return 0;
		}
	}
	return 1;
}

int main()
{
	WSADATA wsaData;
	int retVal = 0;
	fin = false;
	char name[N];
	WORD ver = MAKEWORD(2, 2);
	WSAStartup(ver, (LPWSADATA)&wsaData);
	LPHOSTENT hostEnt;
	hostEnt = gethostbyname("localhost");
	if (!hostEnt)
	{
		cout << "unable to collect receivehostbyname" << endl;
		WSACleanup();
		return 1;
	}
	SOCKET clientSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSock == SOCKET_ERROR)
	{
		printf("Unable to create socket\n");
		WSACleanup();
		system("pause");
		return 1;
	}
	string ip;
	cout << "ip>";
	cin >> ip;
	cin.ignore();
	SOCKADDR_IN serverInfo;
	serverInfo.sin_family = PF_INET;
	serverInfo.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
	serverInfo.sin_port = htons(2008);
	retVal = connect(clientSock, (LPSOCKADDR)&serverInfo, sizeof(serverInfo));
	if (retVal == SOCKET_ERROR)
	{
		printf("Unable to connect\n");
		WSACleanup();
		system("pause");
		return 1;
	}
	printf("Connection made successfully\n");
	printf("Enter your name: ");
	scanf("%s", name);
	//отправить имя серверу
	retVal = send(clientSock, name, N, 0);
	if (retVal == SOCKET_ERROR)
	{
		cout << "unable to send" << endl;
		WSACleanup();
		return 1;
	}
	printf("Enter 's' to finish chat\n");
	//пока не завершена работа клиента/сервера
	while (!fin)
	{
		DWORD threadID;
		CreateThread(NULL, NULL, send, &clientSock, NULL, &threadID);
		CreateThread(NULL, NULL, receive, &clientSock, NULL, &threadID);
	}
	closesocket(clientSock);
	WSACleanup();
	return 0;
}