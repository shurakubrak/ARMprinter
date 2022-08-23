#pragma once

#include <iostream>
#include <list>
#include "client.h"


#define TIMER_PER_LINE		0.5
#define SEPARATOR_PACK		0x01
#define BEGIN_PACK			0x02
#define END_PACK			0x03
#define LENGH_PACK			250
#define MIN_LENGH_PACK		5
#define BLINK_TIME			3000



struct pack_t {
	char tpe;
	const char* data;
	char c;
};

struct comd_t {
	char command = 0;
	std::string data = "";
	std::string str = "";
	
	void copy_to(comd_t* pack) {
		pack->command = command;
		pack->data = data;
		pack->str = str;
	}
};




std::string read_addr();
void cb_analys(char bt, client_t* clt);
void clt_data_parse(std::vector<char> message);
void Request_1();
void Request_2();
void Request_3(string OrderCur, string CmdCur,
	int line_num, int char_num, uint8_t status);