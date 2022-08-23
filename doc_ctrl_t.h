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

class doc_ctrl_t
{
public:
	doc_ctrl_t(){}
	bool ls_print_acc(doc_t* doc, acc_t acc);

private:
	std::mutex m_mx_ls_print;
	std::list<doc_t> m_ls_print;
};

