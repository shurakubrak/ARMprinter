#pragma once

#ifdef _WIN32
    #include <WinSock2.h>
    #include <WinBase.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif 

#define SOCK_BUF_EMPTY		11/*ошибка чтения когда в буфере нет данных*/
#define FAIL                -1
#define OK                  0
#define B_PACK              0x01
#define E_PACK              0x02
#define MAX_MSG              500

enum class acc_t 
{
	add,
	get,
    get_next,
    del
};
