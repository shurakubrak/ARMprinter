#include <iostream>
#include "client.h"

using namespace std;

#define PER 10	//msec
extern atomic<bool> Terminated;

// поток чнения для сокета клиента
void thread_client_read(client_t* arg)
{
	while (!arg->conn() && Terminated == false)
		ssleep(3);
	while (arg->m_status) 
	{
		switch (arg->recive_data()) {
		case SOCK_BUF_EMPTY:/*пусто*/
			msleep(100);
			break;
		case 0:/*прочитали*/
			this_thread::yield();
			break;
		case FAIL:/*ошибка*/
			while (!arg->conn())
				ssleep(3);
		}
		this_thread::yield();
	}
	arg->close_sock();
}

bool client_t::conn()
{
	int err;
	close_sock();

#ifdef _WIN32
	/*инициализация библиотеки WinSock*/
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		printf("WSAStartup failed with error: %d\n", err);
		return false;
	}
#endif // _WIN32
	// Сокет
	m_client_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_client_sock < 0)
	{
		cout << "Socket init error: " + to_string(errno)
			+ "  " + m_server_addr + " - " + to_string(m_port) << endl;
		return false;
	}

	m_to_sin.sin_family = AF_INET;
	m_to_sin.sin_port = htons(m_port);
	int s = inet_pton(AF_INET, m_server_addr.c_str(), &m_to_sin.sin_addr);
	if (s <= 0) {
		if (s == 0)
			cout << "Socket addr not in presentation format" << endl;
		else
			cout << "Socket addr error in 'inet_pton'" << endl;
		return false;
	}


	//Открываем соединение с сервером
	if (connect(m_client_sock, reinterpret_cast<struct sockaddr*>(&m_to_sin), 
			sizeof(m_to_sin)) != 0) 
	{
		perror("socket clent connection error: ");
		cout << "CONNECTION ERR: " + to_string(m_client_sock)
			+ "/" + m_server_addr + " - " + to_string(m_port) << endl;
		return false;
	}
#ifdef _WIN32
	/*неблокирующий сокет (только windows)*/
	unsigned long int on = 1;
	::ioctlsocket(ClientSock, FIONBIO, &on);
#endif // WIN32

	cout << "Socket connect OK: " + to_string(m_client_sock)
		+ "/" + m_server_addr + " - " + to_string(m_port) << endl;
	m_status = true;

	return true;
}

void client_t::start_sock()
{
	thread thr = thread(thread_client_read, this);
	thr.detach();
}

void client_t::close_sock()
{
	if (m_client_sock > -1)
	{
		shutdown(m_client_sock, SHUT_RDWR);
#ifdef _WIN32
		closesocket(ClientSock);
#else
		//close(client_sock);
#endif
	} 
	m_status = false;
}

bool client_t::status_sock()
{
	int error = -1;
	socklen_t len = sizeof(error);
	getsockopt(m_client_sock, SOL_SOCKET, SO_ACCEPTCONN, (char*)&error, &len);
	if (error != 0 || !m_status)
	{
		cout << "Socket: " + to_string(m_client_sock) + "/"
			+ m_server_addr + " port:" + to_string(m_port) +
			+" status faul, error code: " + to_string(errno)
			+ ", connection will be destrow" << endl;
		return false;
	}
	return true;
}

bool client_t::send_data(const char* Message, size_t size)
{
	if (status_sock()) {
		int err = send(m_client_sock, Message, size, MSG_DONTROUTE);
		if (err < 0) {
			cout << "Socket " + to_string(m_client_sock)
				+ " send faul, error code: " + to_string(errno)
				+ ", connection will be destrow" << endl;
			close_sock();
			return false;
		}
		return true;
	}
	return false;
}

int	client_t::recive_data()
{
	const int len_buffer = 1024;
	char sz_buffer[len_buffer];

#ifdef _WIN32
	if (m_client_sock < 0) return false;
	int count = recv(m_client_sock, sz_buffer, len_buffer, 0);
	if (count < 0) {
		int err = WSAGetLastError();
		if (errno != SOCK_BUF_EMPTY
			&& err != WSAEWOULDBLOCK) {
			Log.ToLog("Socket " + m_server_addr
				+ " read faul, error code: " + itoa(errno)
				+ ", connection will be destrow");
			return FAIL;
		}
		else
			return SOCK_BUF_EMPTY;
	}
#else
	int count = recv(m_client_sock, sz_buffer, len_buffer, MSG_DONTWAIT);
	if (count < 0) {
		if (errno != SOCK_BUF_EMPTY) {
			perror("socket read error: ");
			cout << "Socket " + m_server_addr
				+ " - " + to_string(m_port)
				+ ", read faul, error code: " + to_string(errno)
				+ ", connection will be destrow" << endl;
			return FAIL;
		}
		else
			return SOCK_BUF_EMPTY;
	}
#endif // WIN32
	else if (count == 0) {
		perror("socket read error: ");
		cout << "Socket " + m_server_addr
			+ " - " + to_string(m_port)
			+ ", read 0 bytes, connection will be destrow" << endl;
		return FAIL;
	}
	else if (count < 1001)
		for (size_t i = 0; i < count; i++)
			analys(sz_buffer[i], this);
	return 0;
}

bool client_t::ls_in_acc(vector<char>* msg, acc_t acc)
{
    unique_lock<mutex> locker(m_mtx_in);
	switch (acc)
	{
	case acc_t::add:	
		m_ls_in.push_back(*msg);
		return true;
	case acc_t::get:	
		if (m_ls_in.size() > 0) {
			*msg = m_ls_in.front();
			m_ls_in.pop_front();
			return true;
		}
		else return false;
	default:
		break;
	}
	return false;
}