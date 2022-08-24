#include <iostream>
#include "includes/loger.h"

using namespace std;

char en[] = {'\n'};

void thread_loger(loger_t *arg)
{
	if (arg == nullptr)
		return;
	while (arg->m_terminated == false)
	{
		while (arg->from_log())
			this_thread::yield();
	}
	arg->m_terminated = false;
}
//-----------------------------------------------------------------

loger_t::loger_t()
{
	m_thr_loger = thread(thread_loger, this);
	m_thr_loger.detach();
}
//-------------------------------------------------------------------------

void loger_t::to_log(std::string valueMSG, target_log_t target,
					 string value_buf)
{
	unique_ptr<log_msg_t> msg(new log_msg_t());
	msg->m_target = target;
	msg->m_msg = log_get_time() + "  " + valueMSG;
	if (!value_buf.empty())
	{
		for (int i:value_buf)
			msg->m_buf.push_back(value_buf[i]);
	}
	else
		msg->m_msg += '\n';
	list_acc(WRITE, move(msg));
}
//---------------------------------------------------------------------

string loger_t::log_get_time()
{
	time_t t = time(NULL);
	struct tm *ti(localtime(&t));

	string mon = to_string(ti->tm_mon + 1);
	if (ti->tm_mon + 1 < 10)
		mon = "0" + to_string(ti->tm_mon + 1);
	string day = to_string(ti->tm_mday);
	if (ti->tm_mday < 10)
		day = "0" + to_string(ti->tm_mday);
	string hour = to_string(ti->tm_hour);
	if (ti->tm_hour < 10)
		hour = "0" + to_string(ti->tm_hour);
	string min = to_string(ti->tm_min);
	if (ti->tm_min < 10)
		min = "0" + to_string(ti->tm_min);
	string sec = to_string(ti->tm_sec);
	if (ti->tm_sec < 10)
		sec = "0" + to_string(ti->tm_sec);

	string str = to_string(ti->tm_year + 1900) + "-" + mon + "-" + day + " " + hour + ":" + min + ":" + sec;
	return str;
}

string loger_t::log_get_date()
{
	time_t t = time(NULL);
	struct tm *ti(localtime(&t));

	string mon = to_string(ti->tm_mon + 1);
	if (ti->tm_mon + 1 < 10)
		mon = "0" + to_string(ti->tm_mon + 1);
	string day = to_string(ti->tm_mday);
	if (ti->tm_mday < 10)
		day = "0" + to_string(ti->tm_mday);

	string str = to_string(ti->tm_year + 1900) + "-" + mon + "-" + day + " ";
	return str;
}
//-------------------------------------------------------------------

void loger_t::write_to_file_log(log_msg_t *msg)
{
	FILE *p_file;
	string name_file_to_log = log_get_date() + ".log";
	p_file = fopen(name_file_to_log.c_str(), "a");
	if (p_file != nullptr)
	{
		fprintf(p_file, msg->m_msg.c_str());
		for (size_t i : msg->m_buf)
			fprintf(p_file, " x%x", msg->m_buf[i]);
		fprintf(p_file, en);
		fclose(p_file);
	}
}
//------------------------------------------------------------

pr_log_ls_acc_t loger_t::list_acc(int rw, unique_ptr<log_msg_t> add_msg)
{
	pr_log_ls_acc_t ret;
	ret.first = false;

	unique_lock<mutex> lck(m_mtx);

	if (rw == WRITE && add_msg != nullptr)
	{
		if (add_msg->m_target != target_log_t::no_target)
		{
			m_log_list.push_back(move(add_msg));
			ret.first = true;
			return ret;
		}
	}
	else
	{
		if (m_log_list.size() > 0)
		{
			ret.second = move(m_log_list.front());
			m_log_list.pop_front();
			ret.first = true;
			return ret;
		}
	}
	return ret;
}
//----------------------------------------------------------

bool loger_t::from_log()
{
	pr_log_ls_acc_t rs = 
		list_acc(READ, nullptr);

	if (rs.first)
	{
		if (rs.second == nullptr)
			return false;
		switch (rs.second->m_target)
		{
		case target_log_t::cons_only:
			printf(rs.second->m_msg.c_str());
				for (size_t i:rs.second->m_buf)
					printf(" x%x", rs.second->m_buf[i]);
				cout << endl;
			break;
		case target_log_t::file_only:
			write_to_file_log(rs.second.get());
			break;
		case target_log_t::cons_file:
			cout << rs.second->m_msg;
			for (size_t i:rs.second->m_buf)
				printf(" 0x%x", rs.second->m_buf[i]);
			cout << endl;
			write_to_file_log(rs.second.get());
			break;
		case target_log_t::no_target:
			break;
		}
		return true;
	}
	return false;
}
