#include <cstdio>
#include <fstream>
#include <wiringPi.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <linux/lp.h>
#include <sys/ioctl.h>
#include "includes/device.h"
#include "includes/utils.h"

using namespace std;

device_t::device_t()
{
	wiringPiSetup();
	pinMode(LED_MAIN, OUTPUT);
	pinMode(KEY_MAIN, INPUT);
}

void device_t::led_ctrl(bool state)
{
	digitalWrite(LED_MAIN, state);
}

int device_t::key_state()
{
	if (digitalRead(KEY_MAIN))
	{
		m_key_click = true;
		return true;
	}
	return false;
}

int device_t::get_USB()
{
	bool lsu = false;
	FILE* fpipe;
	char c = 0;
	string command = "lsusb | grep 04b8:0046";

	if (0 == (fpipe = (FILE*)popen(command.c_str(), "r")))
	{
		perror("popen() failed.");
		return false;
	}
	while (fread(&c, sizeof c, 1, fpipe)) 
		lsu = true;
	pclose(fpipe);
	return lsu;// LSU;
}

bool device_t::print_string(string line)
{
	m_fl_line_print = true;

	ofstream printer("/dev/usb/lp0");
	if (printer.is_open()) {
		printer << line;
		printer.close();
		msleep(100);
		m_fl_print_fail = false;
		return true;
	}
	else {
		return false;
	}
}

void device_t::print_ctrl()
{
	int printer = open("/dev/usb/lp0", O_RDONLY);
	int* arg;
	if (printer != -1) {
		int stat = ioctl(printer, LPGETSTATUS, arg);
		cout << "LP state: " << to_string(stat) << endl;
		close(printer);
	}
	else
		cout << "open ERROR: " + to_string(errno) << endl;
}
