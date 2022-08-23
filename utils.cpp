#include <cassert>
#include <vector>
#include <iostream>
#include <thread>
#include <iconv.h>
#include <string.h>
#include "utils.h"

using namespace std;
using namespace std::chrono;

string get_data_str()
{
	time_t t = time(NULL);
	struct tm* ti(localtime(&t));

    assert(ti);
	string mon = to_string(ti->tm_mon + 1);
	if (ti->tm_mon + 1 < 10)
		mon = "0" + to_string(ti->tm_mon + 1);
	string day = to_string(ti->tm_mday);
	if (ti->tm_mday < 10)
		day = "0" + to_string(ti->tm_mday);

	string str = to_string(ti->tm_year + 1900) + "."
		+ mon + "."
		+ day + " ";
	return str;
}

string get_time_ms_str(bool add_ms)
{
	string ms;
	Time t = system_clock::now();
	uint64_t mm = get_time_ms(t);
	time_t dt = system_clock::to_time_t(t);
	struct tm* ti(localtime(&dt));

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
	if (add_ms)
		ms = to_string(mm - (dt * 1000));
	else
		ms = to_string(mm - (dt * 1000) + 1);

	string str = to_string(ti->tm_year + 1900) + "."
		+ mon + "."
		+ day + " "
		+ hour + ":"
		+ min + ":"
		+ sec + "."
		+ ms;
	return str;
}

uint64_t get_time_ms()
{
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

uint64_t get_time_s()
{
	return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

uint64_t get_time_ms(Time dt)
{
	return duration_cast<milliseconds>(dt.time_since_epoch()).count();
}
uint64_t get_time_s(Time dt)
{
	return duration_cast<seconds>(dt.time_since_epoch()).count();
}

int32_t twoint16_oneint32(int16_t h, int16_t l)
{
    return static_cast<int32_t>(((static_cast<int32_t>(l)) & 0xffff) 
            | (static_cast<int32_t>(h) << 16));
}

void oneint32_twoint16(int16_t &h, int16_t &l, int32_t val)
{
	l = (int16_t)(val & 0xffff);
	h = (int16_t)((val >> 16) & 0xffff);
}

int get_random_number(int min, int max)
{
    static const double fraction = 1.0 / (static_cast<double>(RAND_MAX) + 1.0);
	return static_cast<int>(rand() * fraction * (max - min + 1) + min);
}

void ssleep(size_t sec)
{
	this_thread::sleep_for(milliseconds(sec * 1000));
}

void msleep(size_t msec)
{
	this_thread::sleep_for(milliseconds(msec));
}

bool wait4(bool* flag, uint64_t msec)
{
	uint64_t start = get_time_ms();
	bool fl_start = *flag;
	while ((get_time_ms() - start) < msec)
		if (*flag != fl_start)
			return true;
		else
			msleep(5);
	return false;
}

bool wait4(atomic<bool>* flag, uint64_t msec)
{
	uint64_t start = get_time_ms();
	bool fl_start = flag->load();
	while ((get_time_ms() - start) < msec)
		if (flag->load() != fl_start)
			return true;
		else
			msleep(5);
	return false;
}

bool atob(string str)
{
	if (str == "Y" || str == "y" || str == "Yes" || str == "yes" || str == "1" || str == "t" || str == "T" || str == "True" || str == "true"
		|| str == "true" || str == "FALSE")
		return true;
	else
		return false;
}

char* convert(const char* s, const char* from_cp, const char* to_cp)
{
	iconv_t ic = iconv_open(to_cp, from_cp);

	if (ic == (iconv_t)(-1)) 
		return "";
	
	char* out_buf = (char*)calloc(strlen(s) + 1, 1);
	char* out = out_buf;
	char* in = (char*)malloc(strlen(s) + 1);
	strcpy(in, s);

	size_t in_ln = strlen(s);
	size_t out_ln = in_ln;
	size_t k = iconv(ic, &in, &in_ln, &out, &out_ln);
	if (k == (size_t)-1)
		fprintf(stderr, "iconv: %u of %d converted\n", k, strlen(s));
	iconv_close(ic);
	return out_buf;
}
