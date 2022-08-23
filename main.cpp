#include <atomic>
#include <fstream>
#include "main.h"
#include "loger.h"
#include "device.h"
#include "client.h"
#include "utils.h"

using namespace std;

atomic<bool> Terminated(false);

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






