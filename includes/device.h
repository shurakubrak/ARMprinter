#pragma once
#include <string>
#include <atomic>

/*?????? gpio*/
#define LED_MAIN	16
#define KEY_MAIN	4
#define GP_LOW		0
#define GP_HIGH		1

class device_t
{
public:
	bool m_fl_line_print = false;
	bool m_fl_print_fail = false;
	std::atomic<bool> m_key_click;

	device_t();
	void led_ctrl(bool state);
	int key_state();
	int get_USB();
	bool print_string(std::string line);
	void print_ctrl();
};

