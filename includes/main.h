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

struct doc_t
{
	std::string order_id = "";
	std::string cmd_id = "";
	uint8_t copies = 0;
	bool confirm = false;
	uint16_t line_num = 0;
	uint8_t margin_count = 0;
	uint8_t margin_left = 0;
	uint8_t margin_copy = 7;
	uint8_t max_num_simbol = 80;
	std::string dt = "";
	std::list<std::string> lines;
	 
	void copy_to(doc_t* doc) {
		doc->order_id = order_id;
		doc->cmd_id = cmd_id;
		doc->copies = copies;
		doc->confirm = confirm;
		doc->line_num = line_num;
		doc->margin_count = margin_count;
		doc->margin_left = margin_left;
		doc->margin_copy = margin_copy;
		doc->max_num_simbol = max_num_simbol;
		doc->lines = lines;
		doc->dt = dt;
	}
	void Clear() {
		order_id = "";
		cmd_id = "";
		copies = 0;
		confirm = false;
		line_num = 0;
		margin_count = 0;
		margin_left = 0;
		margin_copy = 7;
		max_num_simbol = 80;
		lines.clear();
		dt = "";
	}
};

std::string read_addr();
void cb_analys(char bt, client_t* clt);
void clt_data_parse(std::vector<char> message);
