#pragma once
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <list>
#include <array>

#define READ 0
#define WRITE 1

#define BUF_HEX_SIZE 1024
#define BUF_CYCLE_SIZE 10

enum class target_log_t
{
	cons_only = 1,
	file_only,
	cons_file,
	no_target
};

class log_msg_t
{
public:
	target_log_t m_target = target_log_t::no_target;
	std::string m_msg = "";
	std::vector<char> m_buf;

	log_msg_t()
	{
		m_buf.clear();
	}
};
/********************************************/

using pr_log_ls_acc_t = std::pair<bool, std::unique_ptr<log_msg_t>>;

class loger_t
{
public:
	bool m_terminated = false;
	loger_t();
	void to_log(std::string valueMSG, target_log_t target = target_log_t::cons_only,
				std::string value_buf = nullptr);
	bool from_log();

private:
	target_log_t m_target = target_log_t::cons_only;
	std::thread m_thr_loger;
	std::mutex m_mtx;
	std::list<std::unique_ptr<log_msg_t>> m_log_list;

	void write_to_file_log(log_msg_t *msg);
	pr_log_ls_acc_t list_acc(int rw, std::unique_ptr<log_msg_t> add_msg);
	std::string log_get_time();
	std::string log_get_date();
};
