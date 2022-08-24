#include "includes/docctrl.h"

size_t doc_ctrl_t::length_str_no_space(std::string str)
{
	size_t size = 0;
	for (size_t i = 0; i < str.length(); i++)
		if (str[i] != ' '
			&& str[i] != '\n'
			&& str[i] != '\r')
			size++;
	return size;
}

bool doc_ctrl_t::ls_print_acc(doc_t* doc, acc_t acc)
{
	std::unique_lock<std::mutex> locker(this->m_mx_print);
	switch (acc)
	{
	case acc_t::add:
		m_ls_print.push_back(*doc);
		return true;
		break;
	case acc_t::get:
		if (m_ls_print.size())
		{
			*doc = m_ls_print.front();
			m_ls_print.pop_front();
			return true;
		}
		break;
	case acc_t::del:
		m_ls_print.clear();
		break;
	}
	return false;
}
