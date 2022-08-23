#pragma once

#include <cstdint>
#include <functional>
#include <vector>
#include <list>
#include <mutex>
#include <atomic>
#include <thread>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "utils.h"
#include "sock_utils.h"

class client_t
{
public:
	int m_client_sock = -1;
	sockaddr_in m_to_sin;
	std::string m_server_addr = "127.0.0.1";
	uint16_t m_port = 6000;
	std::vector<char> m_message;
	bool m_begin;
	std::atomic<bool> m_is_recive;
	std::atomic<bool> m_status;
	
	client_t(std::function<void(char, client_t*)> analys)
	{
		m_status.store(false);
		this->analys = analys;
	};
	client_t(int32_t port, std::string s_addr, 
			std::function<void(char, client_t*)> analys)
	{
		m_status.store(false);
		this->analys = analys;
		this->m_port = port;
		m_server_addr = s_addr;
	};
	
	bool	conn(); 
	void	start_sock();
	void	close_sock();
	bool	status_sock();
	bool 	send_data(const char* Message, size_t size);
	int		recive_data();
	void	add_in(std::vector<char>* msg){ls_in_acc(msg, acc_t::add);}
	bool	get_in(std::vector<char>* msg){return ls_in_acc(msg, acc_t::get);}

private:
	std::mutex m_mtx_in;
	std::list<std::vector<char>> m_ls_in;
	std::function<void(char, client_t*)> analys;	
	bool ls_in_acc(std::vector<char>* msg, acc_t acc);
};
