#include <atomic>
#include <fstream>
#include "main.h"
#include "loger.h"
#include "utils.h"
#include "device.h"
#include "doc_ctrl_t.h"

using namespace std;

struct printer_t
{
	doc_ctrl_t doc;
	device_t dev;
	client_t clt;
};

atomic<bool> Terminated(false);
atomic<int> count_line_print(0);
atomic<int> count_char_print(0);
atomic<int> cmd_current(-1);
atomic<int> order_current(-1);
loger_t log;

/*поток таймера*/
void thread_Timer(printer_t* arg)
{
	while (!Terminated) {
		if (arg->dev.m_fl_line_print)
			if (!wait4(&arg->dev.m_fl_line_print, 2000)) {
				arg->dev.m_fl_print_fail = true;
				Request_3(to_string(order_current), to_string(cmd_current), 
					count_line_print, count_char_print, 1);
				count_line_print = 0;
				count_char_print = 0;
				arg->dev.m_fl_line_print = false;
			}
		msleep(10);
	}
}

/*поток печати*/
void thread_printer(printer_t* arg)
{
	doc_t doc;
	size_t pos;
	string left_space = "";
	string space = "";
	space += '\r';
	space += '\n';
	string head = "";
	string bottom = "";
	bool err = false;

	while (!Terminated) {
		if (arg->doc.ls_print_acc(&doc, acc_t::get)) {
			count_line_print = 0;
			count_char_print = 0;
			order_current = atoi(doc.order_id.c_str());
			cmd_current = atoi(doc.cmd_id.c_str());
			/*колонтитулы*/
			left_space = "";
			for (size_t i = 0; i < doc.mergin_left; i++)
				left_space += " ";

			doc.lines.push_front(space);
			head = left_space + "~~~~ Копия N ~~~~ Задание на пост передано " + doc.dt + " ~~~~~~~~~" + '\n';
			head = convert(head.c_str(), "utf-8", "cp1251");
			doc.lines.push_front(head);

			bottom = left_space + "~~~ Конец копии ~~~~~~ " + doc.dt + " ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" + '\n';
			bottom = convert(bottom.c_str(), "utf-8", "cp1251");
			doc.lines.push_back(bottom);
			doc.lines.push_back(space); /*пустая строка ввеху копии*/

			doc.line_num += 4;

			for (size_t i = 0; i < doc.copies; i++) {
				pos = doc.lines.front().find('N');
				if (pos != string::npos)
					doc.lines.front().replace(pos + 1, to_string(i + 1).length(), to_string(i + 1));
				/*печатаем*/
				for (auto it = doc.lines.begin(); it != doc.lines.end(); it++) {
					if (arg->clt.m_printer_del)
						goto PrDel;
					while (!arg->dev.get_USB())
						msleep(100);
					if (arg->clt.m_printer_del)
						goto PrDel;
					if (arg->dev.print_string(*it)) {
						count_line_print++;
						count_char_print += LengthStrNoSpace(*it);
						msleep(10);
					}
					else
						err = true;
				}

				/*отступ между копиями*/
				if (doc.copies - i > 1)/*кроме последней копии*/
					for (size_t c = 0; c < doc.mergin_copy; c++) {
						if (arg->clt.m_printer_del)
							goto PrDel;
						while (!arg->dev.get_USB())
							msleep(100);
						if (arg->clt.m_printer_del)
							goto PrDel;
						if (arg->dev.print_string(space)) {
							count_line_print++;
							msleep(10);
						}
						else
							err = true;
					}
			}
			/*отступ снизу*/
			for (size_t i = 0; i < doc.mergin_count; i++) {
				if (arg->clt.m_printer_del)
					goto PrDel;
				while (!arg->dev.get_USB())
					msleep(100);
				if (arg->clt.m_printer_del)
					goto PrDel;
				if (arg->dev.print_string(space)) {
					count_line_print++;
					msleep(10);
				}
				else
					err = true;
			}

			if (doc.confirm)
				arg->dev.led_ctrl(GP_HIGH);
		PrDel:
			if (err)
				log.to_log("Print error");
			else if (arg->clt.m_printer_del)/*отменено*/
				Request_3(doc.order_id, doc.cmd_id, count_line_print, count_char_print, 2);
			else/*выполнено*/
				Request_3(doc.order_id, doc.cmd_id, count_line_print, count_char_print, 0);
			arg->clt.m_printer_del = false;
			err = false;
			count_line_print = 0;
			count_char_print = 0;
			doc.Clear();
		}
		else
			msleep(50);
	}
}

int main()
{
	log.to_log("START...");
	printer_t printer;
	
	//PrintCtl();
	printer.dev.led_ctrl(GP_HIGH);
	msleep(300);
	printer.dev.led_ctrl(GP_LOW);
	string str = {7, 17, 27, 64, 27, 107, 48 };
	printer.clt.m_port = 6030;
	printer.clt.m_server_addr = read_addr();
	printer.clt.set_analys(cb_analys);
	if (printer.clt.m_server_addr.empty())
	{
		log.to_log("ip addres fail");
		return -1;
	}
	printer.clt.start_sock();
	thread thr_print = thread(thread_printer, &printer);
	thr_print.detach();
	thread thr_tmr = thread(thread_printer, &printer);
	thr_tmr.detach();

	Terminated = true;
	printer.clt.close_sock();
	ssleep(1);
	log.m_terminated = true;
	ssleep(2);
	return 0;
}
/**********************************************/

string read_addr()
{
	string addr = "";
	char buf[20];

	ifstream f;
	f.open("/home/orangepi/settngs.txt");
	getline(f, addr);
	return addr;
}


void cb_analys(char bt, client_t* clt)
{
	if (clt->m_message.size() >= LENGH_PACK)
		clt->m_begin = false;
	else {
		switch (bt)
		{
		case BEGIN_PACK:
			clt->m_message.clear();
			clt->m_message.push_back(bt);
			clt->m_begin = true;
			break;
		case END_PACK:
			clt->m_begin = false;
			clt->m_message.push_back(bt);
			clt_data_parse(clt->m_message);
			break;
		default:
			if (clt->m_begin) 
				clt->m_message.push_back(bt);
			break;
		}
	}
}

void clt_data_parse(vector<char> message)
{
	string str = "";
	size_t pos = 0;

	switch (message[1])
	{
	case '1':
		break;
	}
}

void Request_1()
{
	char buf[] = { BEGIN_PACK,'1',SEPARATOR_PACK,'1',END_PACK };
	buf[3] = (getUSB()) ? '1' : '2'; /*2 - нет принтера*/
	if (fl_PrintFail)
		buf[3] = '3'; /*ошибка принтера*/
	sClient.Send(buf, sizeof(buf));

}

void Request_2()
{
	string pack;
	pack += BEGIN_PACK;
	pack += '2';
	pack += SEPARATOR_PACK;
	pack += to_string(cmd_current.load());
	pack += END_PACK;
	sClient.Send(pack.c_str(), pack.length());
}

void Request_3(string OrderCur, string CmdCur,
	int line_num, int char_num, uint8_t status)
{
	string send_pack = "";
	send_pack += (char)BEGIN_PACK;
	send_pack += "3";
	send_pack += (char)SEPARATOR_PACK;
	send_pack += OrderCur;
	send_pack += (char)SEPARATOR_PACK;
	send_pack += CmdCur;
	send_pack += (char)SEPARATOR_PACK;
	send_pack += to_string(line_num);
	send_pack += (char)SEPARATOR_PACK;
	send_pack += to_string(char_num);
	send_pack += (char)SEPARATOR_PACK;
	switch (status)
	{
	case 0:
		send_pack += "8";
		break;
	case 1:
		send_pack += "10";
		break;
	case 2:
		send_pack += "14";
		break;
	}
	send_pack += (char)END_PACK;

	sClient.Send(send_pack.c_str(), send_pack.length());
}



