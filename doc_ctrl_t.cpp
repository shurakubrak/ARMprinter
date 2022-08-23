#include "doc_ctrl_t.h"

bool doc_ctrl_t::ls_print_acc(doc_t* doc, acc_t acc)
{
	std::unique_lock<std::mutex> locker(m_mx_ls_print);
	switch (acc)
	{
	case acc_t::add :
		m_ls_print.push_back(*doc);
		return true;
		break;
	case acc_t::get :
		if (m_ls_print.size()) {
			*doc = m_ls_print.front();
			m_ls_print.pop_front();
			return true;
		}
		break;
	case acc_t::del :
		m_ls_print.clear();
		break;
	}
	return false;
}
