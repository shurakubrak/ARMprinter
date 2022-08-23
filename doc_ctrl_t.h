#pragma once
#include <iostream>
#include <list>
#include <mutex>
#include "client.h"

struct doc_t
{
	std::string order_id = "";
	std::string cmd_id = "";
	uint8_t copies = 0;
	bool confirm = false;
	uint16_t line_num = 0;
	uint8_t mergin_count = 0;
	uint8_t mergin_left = 0;
	uint8_t mergin_copy = 7;
	uint8_t max_num_simbol = 80;
	std::string dt = "";
	std::list<std::string> lines;

	void Clear() {
		order_id = "";
		cmd_id = "";
		copies = 0;
		confirm = false;
		line_num = 0;
		mergin_count = 0;
		mergin_left = 0;
		mergin_copy = 7;
		max_num_simbol = 80;
		lines.clear();
		dt = "";
	}
};

class doc_ctrl_t
{
public:
	doc_ctrl_t(){}
	bool ls_print_acc(doc_t* doc, acc_t acc);
	static size_t length_str_no_space(std::string str);

private:
	std::mutex m_mx_ls_print;
	std::list<doc_t> m_ls_print;
};

