#include <atomic>
#include <fstream>
#include "main.h"
#include "loger.h"
#include "device.h"
#include "utils.h"
#include "doc_ctrl_t.h"

using namespace std;

atomic<bool> Terminated(false);

/*поток печати*/
void thread_printer(doc_t* arg)
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

	try
	{
		while (!Terminated) {
			if (arg->ls_print_acc(&doc, acc_t::get)) {
				CountLinePrint = 0;
				CountCharPrint = 0;
				OrderCurrent = atoi(doc.order_id.c_str());
				CmdCurrent = atoi(doc.cmd_id.c_str());
				/*колонтитулы*/
				left_space = "";
				for (size_t i = 0; i < doc.MarginLeft; i++)
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
						if (PrintDel)
							goto PrDel;
						while (!getUSB())
							msleep(100);
						if (PrintDel)
							goto PrDel;
						if (PrintString(*it)) {
							CountLinePrint++;
							CountCharPrint += LengthStrNoSpace(*it);
							msleep(10);
						}
						else
							err = true;
					}

					/*отступ между копиями*/
					if (doc.copies - i > 1)/*кроме последней копии*/
						for (size_t c = 0; c < doc.MarginCopy; c++) {
							if (PrintDel)
								goto PrDel;
							while (!getUSB())
								msleep(100);
							if (PrintDel)
								goto PrDel;
							if (PrintString(space)) {
								CountLinePrint++;
								msleep(10);
							}
							else
								err = true;
						}
				}
				/*отступ снизу*/
				for (size_t i = 0; i < doc.MarginCount; i++) {
					if (PrintDel)
						goto PrDel;
					while (!getUSB())
						msleep(100);
					if (PrintDel)
						goto PrDel;
					if (PrintString(space)) {
						CountLinePrint++;
						msleep(10);
					}
					else
						err = true;
				}

				if (doc.confirm)
					LEDon(true);
			PrDel:
				fid = 14;
				if (err)
					Log.ToLog("Print error");
				else if (PrintDel)/*отменено*/
					Request_3(doc.order_id, doc.cmd_id, CountLinePrint, CountCharPrint, 2);
				else/*выполнено*/
					Request_3(doc.order_id, doc.cmd_id, CountLinePrint, CountCharPrint, 0);
				PrintDel = false;
				err = false;
				CountLinePrint = 0;
				CountCharPrint = 0;
				doc.Clear();
			}
			else
				msleep(50);
		}
	}
	catch (const std::exception&)
	{
		cout << "Error print" << endl;
	}
}

int main()
{
	loger_t log;
	log.to_log("START...");

	device_t dev;
	
	//PrintCtl();
	dev.led_ctrl(GP_HIGH);
	msleep(300);
	dev.led_ctrl(GP_LOW);
	string str = {7, 17, 27, 64, 27, 107, 48 };
	client_t clt(6030, read_addr(), cb_analys);
	if (clt.m_server_addr.empty())
	{
		log.to_log("ip addres fail");
		return -1;
	}
	clt.start_sock();
	thread thrPrint = thread(thread_printer, nullptr);
	thrPrint.detach();

	Terminated = true;
	clt.close_sock();
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
//--------------------------------------------

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
//--------------------------------------------------------------





