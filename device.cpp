#include <cstdio>
#include <fstream>
#include <wiringPi.h>
#include "device.h"
#include "utils.h"

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
	return digitalRead(KEY_MAIN);
}

int device_t::get_USB()
{
	bool lsu = false;
	FILE* fpipe;
	char c = 0;
	char* command = "lsusb | grep 04b8:0046";

	if (0 == (fpipe = (FILE*)popen(command, "r")))
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

