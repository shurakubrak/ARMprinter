#pragma once

#include <iostream>
#include "client.h"
#include "docctrl.h"
#include "device.h"


#define TIMER_PER_LINE		0.5
#define SEPARATOR_PACK		0x01
#define BEGIN_PACK			0x02
#define END_PACK			0x03
#define LENGH_PACK			250
#define MIN_LENGH_PACK		5
#define BLINK_TIME			3000


struct printer_t
{
	doc_ctrl_t doc;
	device_t dev;
	client_t clt;
	std::atomic<int> count_line_print;
	std::atomic<int> count_char_print;
	std::atomic<int> cmd_current;
	std::atomic<int> order_current;
	printer_t()
	{
		count_line_print.store(0);
		count_char_print.store(0);
		cmd_current.store(-1);
		order_current.store(-1);
	}
};


std::string read_addr();
void cb_analys(char bt, client_t* clt);
void clt_data_parse(std::vector<char> message, client_t* clt);
void request_1(printer_t* pnt);
void request_2(printer_t* pnt);
void request_3(printer_t* pnt, std::string OrderCur, std::string CmdCur,
	int line_num, int char_num, uint8_t status);