#include <fstream>
#include <cassert>
#include "includes/main.h"
#include "includes/loger.h"
#include "includes/utils.h"

using namespace std;

atomic<bool> Terminated(false);

loger_t log;

/*поток таймера*/
void thread_Timer(printer_t* arg)
{
	assert(arg != nullptr);
	while (!Terminated) {
		if (arg->dev.m_fl_line_print)
			if (!wait4(&arg->dev.m_fl_line_print, 2000)) {
				arg->dev.m_fl_print_fail = true;
				request_3(arg, to_string(arg->order_current), to_string(arg->cmd_current),
					arg->count_line_print, arg->count_char_print, 1);
				arg->count_line_print = 0;
				arg->count_char_print = 0;
				arg->dev.m_fl_line_print = false;
			}
		this_thread::yield();
	}
}

/*поток печати*/
void thread_printer(printer_t* arg)
{
	assert(arg != nullptr);
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
		if (arg->doc.get_doc(&doc)) {
			arg->count_line_print = 0;
			arg->count_char_print = 0;
			arg->order_current = atoi(doc.order_id.c_str());
			arg->cmd_current = atoi(doc.cmd_id.c_str());
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
						arg->count_line_print++;
						arg->count_char_print += arg->doc.length_str_no_space(*it);
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
							arg->count_line_print++;
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
					arg->count_line_print++;
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
				request_3(arg, doc.order_id, doc.cmd_id, arg->count_line_print,
					arg->count_char_print, 2);
			else/*выполнено*/
				request_3(arg, doc.order_id, doc.cmd_id, arg->count_line_print,
					arg->count_char_print, 0);
			arg->clt.m_printer_del = false;
			err = false;
			arg->count_line_print = 0;
			arg->count_char_print = 0;
			doc.clear();
		}
		else
			this_thread::yield();
	}
}

/*поток разбора данных слиента*/
void thread_clt_data_parse(printer_t* pnt)
{
	string str = "";
	size_t pos = 0;

	assert(pnt != nullptr);
	vector<char> message;
	while (!Terminated)
	{
		if(pnt->clt.get_in(&message))
		switch (message[1])
		{
		case '1':
			request_1(pnt);
			break;

		case '2':
			pnt->dev.m_fl_print_fail = true;
			pnt->doc.del_doc();
			break;

		case '3':
			pnt->dev.m_fl_print_fail = false;
			pnt->doc.m_document.clear();
			/*order_id*/
			for (size_t i = 3; i < message.size(); i++) {
				if (message[i] != SEPARATOR_PACK && message[i] != END_PACK)
					pnt->doc.m_document.order_id += message[i];
				else {
					pos = i + 1;
					break;
				}
			}
			/*cmd_id*/
			for (size_t i = pos; i < message.size(); i++) {
				if (message[i] != SEPARATOR_PACK && message[i] != END_PACK)
					pnt->doc.m_document.cmd_id += message[i];
				else {
					pos = i + 1;
					break;
				}
			}
			/*copies*/
			str = "";
			for (size_t i = pos; i < message.size(); i++) {
				if (message[i] != SEPARATOR_PACK && message[i] != END_PACK)
					str += message[i];
				else {
					pos = i + 1;
					break;
				}
			}
			pnt->doc.m_document.copies = atoi(str.c_str());
			/*confirm*/
			str = "";
			for (size_t i = pos; i < message.size(); i++) {
				if (message[i] != SEPARATOR_PACK && message[i] != END_PACK)
					str += message[i];
				else {
					pos = i + 1;
					break;
				}
			}
			pnt->doc.m_document.confirm = atob(str);
			/*lines number*/
			str = "";
			for (size_t i = pos; i < message.size(); i++) {
				if (message[i] != SEPARATOR_PACK && message[i] != END_PACK)
					str += message[i];
				else {
					pos = i + 1;
					break;
				}
			}
			pnt->doc.m_document.line_num = atoi(str.c_str());
			/*MarginCount - отступ снизу*/
			str = "";
			for (size_t i = pos; i < message.size(); i++) {
				if (message[i] != SEPARATOR_PACK && message[i] != END_PACK)
					str += message[i];
				else {
					pos = i + 1;
					break;
				}
			}
			pnt->doc.m_document.mergin_count = atoi(str.c_str());
			/*MarginLeft - отступ слевау*/
			str = "";
			for (size_t i = pos; i < message.size(); i++) {
				if (message[i] != SEPARATOR_PACK)
					str += message[i];
				else {
					pos = i + 1;
					break;
				}
			}
			pnt->doc.m_document.mergin_left = atoi(str.c_str());
			/*MarginCopy - пропуск между копиями*/
			str = "";
			for (size_t i = pos; i < message.size(); i++) {
				if (message[i] != SEPARATOR_PACK)
					str += message[i];
				else {
					pos = i + 1;
					break;
				}
			}
			pnt->doc.m_document.mergin_copy = atoi(str.c_str());
			/*MaxNumSimbol - кол. символов в строке*/
			str = "";
			for (size_t i = pos; i < message.size(); i++) {
				if (message[i] != SEPARATOR_PACK)
					str += message[i];
				else {
					pos = i + 1;
					break;
				}
			}
			pnt->doc.m_document.max_num_simbol = atoi(str.c_str());
			/*date-time*/
			for (size_t i = pos; i < message.size(); i++) {
				if (message[i] != SEPARATOR_PACK && message[i] != END_PACK)
					pnt->doc.m_document.dt += message[i];
				else {
					pos = i + 1;
					break;
				}
			}
			break;

		case '4':
			if (pnt->doc.m_document_begin) {
				for (int i = 3; i < message.size() - 1; i++)
					str += message[i];
				pnt->doc.m_document.lines.push_back(str);
				if (pnt->doc.m_document.lines.size() >= pnt->doc.m_document.line_num) {
					pnt->doc.add_doc(&pnt->doc.m_document);
					pnt->doc.m_document_begin = false;
					pnt->doc.m_document.clear();
				}
			}
			break;
		}
		this_thread::yield();
	}
}

/**********************************************************/
int main()
{
	log.to_log("START...");
	printer_t printer;
	
	printer.dev.print_ctrl();
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
	thread thr_parse = thread(thread_clt_data_parse, &printer);
	thr_parse.detach();

	uint64_t tm = get_time_ms();
	request_2(&printer);
	while (!Terminated)
	{
		if (printer.dev.key_state() && printer.dev.m_key_click) {
			printer.dev.led_ctrl(false);
			request_2(&printer);
			printer.dev.m_key_click = false;
		}

		/*контроль принтера*/
		if (printer.dev.m_fl_print_fail) {
			if (!printer.dev.get_USB())
				printer.dev.m_fl_print_fail = false;
		}

		/*маяк*/
		if (get_time_ms() - tm > BLINK_TIME) {
			request_1(&printer);
			tm = get_time_ms();
		}
		msleep(50);
	}

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
	f.open("./settings.txt");
	if (f.is_open())
	{
		getline(f, addr);
		f.close();
		return addr;
	}
	return "";
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
			clt->add_in(&clt->m_message);
			break;
		default:
			if (clt->m_begin) 
				clt->m_message.push_back(bt);
			break;
		}
	}
}

void request_1(printer_t* pnt)
{
	char buf[] = { BEGIN_PACK,'1',SEPARATOR_PACK,'1',END_PACK };
	buf[3] = (pnt->dev.get_USB()) ? '1' : '2'; /*2 - нет принтера*/
	if (pnt->dev.m_fl_print_fail)
		buf[3] = '3'; /*ошибка принтера*/
	pnt->clt.send_data(buf, sizeof(buf));
}

void request_2(printer_t* pnt)
{
	string pack;
	pack += BEGIN_PACK;
	pack += '2';
	pack += SEPARATOR_PACK;
	pack += to_string(pnt->cmd_current.load());
	pack += END_PACK;
	pnt->clt.send_data(pack.c_str(), pack.length());
}

void request_3(printer_t* pnt, string OrderCur, string CmdCur,
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

	pnt->clt.send_data(send_pack.c_str(), send_pack.length());
}



