/*
 * Utils.cpp
 *
 *  Created on: 20-Jun-08
 *      Author: hoglet
 */

#include "libtimeit/Utils.h"
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include "libtimeit/OSAbstraction.h"
#include "time.h"
#include <iomanip>
#include <langinfo.h>
#include <sys/stat.h>
#include <sys/time.h>

//using namespace Glib;
using namespace std;

namespace libtimeit
{
constexpr auto SECOND_PER_DAY = 24 * 60 * 60;

//LCOV_EXCL_START
std::string getImagePath()
{
	return PROGRAMNAME_DATADIR;
}

std::string get639LanguageString()
{
	std::string retVal = setlocale(LC_ALL, NULL);
	retVal = retVal.substr(0, 2);
	return retVal;
}

time_t getBeginingOfDay(const time_t &rawtime)
{
	struct tm *timeInfo;
	timeInfo = localtime(&rawtime);
	timeInfo->tm_sec = 0;
	timeInfo->tm_min = 0;
	timeInfo->tm_hour = 0;
	timeInfo->tm_isdst = -1;
	return mktime(timeInfo);
}
time_t getEndOfDay(const time_t &rawtime)
{
	struct tm *timeInfo;
	timeInfo = localtime(&rawtime);
	timeInfo->tm_sec = 59;
	timeInfo->tm_min = 59;
	timeInfo->tm_hour = 23;
	timeInfo->tm_isdst = -1;
	return mktime(timeInfo);
}

int getDayOfWeek(const time_t rawtime)
{
	struct tm *timeInfo = localtime(&rawtime);
	int dow = timeInfo->tm_wday;
	int retVal = dow;

	int weekStartsOnDay = *(nl_langinfo(_NL_TIME_FIRST_WEEKDAY)) - 1;
	retVal = retVal - weekStartsOnDay;
	if (retVal < 0)
	{
		retVal = retVal + 7;
	}
	return retVal;
}

time_t getBeginingOfWeek(const time_t &rawtime)
{
	struct tm *timeInfo;
	int dayOfWeek = getDayOfWeek(rawtime);
	time_t bow = rawtime - (dayOfWeek) * SECOND_PER_DAY;
	timeInfo = localtime(&bow);
	timeInfo->tm_sec = 0;
	timeInfo->tm_min = 0;
	timeInfo->tm_hour = 0;
	timeInfo->tm_wday = 1;
	timeInfo->tm_isdst = -1;
	return mktime(timeInfo);
}

time_t getEndOfWeek(const time_t &rawtime)
{
	struct tm *timeInfo;
	int dayOfWeek = getDayOfWeek(rawtime);
	time_t eow = rawtime + (6 - dayOfWeek) * SECOND_PER_DAY;
	timeInfo = localtime(&eow);
	timeInfo->tm_sec = 59;
	timeInfo->tm_min = 59;
	timeInfo->tm_hour = 23;
	timeInfo->tm_isdst = -1;
	return mktime(timeInfo);
}
time_t getBeginingOfMonth(const time_t &rawtime)
{
	struct tm *timeInfo;
	timeInfo = localtime(&rawtime);
	timeInfo->tm_sec = 0;
	timeInfo->tm_min = 0;
	timeInfo->tm_hour = 0;
	timeInfo->tm_mday = 1;
	timeInfo->tm_isdst = -1;
	return mktime(timeInfo);
}
time_t getEndOfMonth(const time_t &rawtime)
{
	struct tm *timeInfo = localtime(&rawtime);
	timeInfo->tm_sec = 59;
	timeInfo->tm_min = 59;
	timeInfo->tm_hour = 23;
	timeInfo->tm_mday = getDaysInMonth(rawtime);
	timeInfo->tm_isdst = -1;
	return mktime(timeInfo);
}

time_t getBeginingOfYear(const time_t &rawtime)
{
	struct tm *timeInfo;
	timeInfo = localtime(&rawtime);
	timeInfo->tm_sec = 0;
	timeInfo->tm_min = 0;
	timeInfo->tm_hour = 0;
	timeInfo->tm_mday = 1;
	timeInfo->tm_mon = 0;
	timeInfo->tm_isdst = -1;
	return mktime(timeInfo);
}
time_t getEndOfYear(const time_t &rawtime)
{
	struct tm *timeInfo = localtime(&rawtime);
	timeInfo->tm_sec = 59;
	timeInfo->tm_min = 59;
	timeInfo->tm_hour = 23;
	timeInfo->tm_mday = 31;
	timeInfo->tm_mon = 11;
	timeInfo->tm_isdst = -1;
	return mktime(timeInfo);
}

int getDaysInMonth(const time_t &rawtime)
{
	struct tm *timeInfo = localtime(&rawtime);
	timeInfo->tm_sec = 59;
	timeInfo->tm_min = 59;
	timeInfo->tm_hour = 23;
	switch (timeInfo->tm_mon)
	{
	case 1:
		if ((timeInfo->tm_year % 4 == 0 && timeInfo->tm_year % 100 != 0) || timeInfo->tm_year % 400 == 0)
		{
			timeInfo->tm_mday = 29;
		}
		else
		{
			timeInfo->tm_mday = 28;
		}
		break;
	case 3:
	case 5:
	case 8:
	case 10:
		timeInfo->tm_mday = 30;
		break;
	default:
		timeInfo->tm_mday = 31;
		break;
	}
	return timeInfo->tm_mday;
}

time_t getTime(int year, int month, int day, int hour, int min, int sec)
{
	time_t rawtime;
	time(&rawtime);
	struct tm *timeInfo;
	timeInfo = localtime(&rawtime);
	timeInfo->tm_year = year - 1900;
	timeInfo->tm_mon = month;
	timeInfo->tm_mday = day;
	timeInfo->tm_hour = hour;
	timeInfo->tm_min = min;
	timeInfo->tm_sec = sec;
	timeInfo->tm_isdst = -1;
	return mktime(timeInfo);
}

bool onDifferentDays(const time_t &one, const time_t &other)
{
	struct tm oneTime = *localtime(&one);
	struct tm otherTime = *localtime(&other);
	return oneTime.tm_year != otherTime.tm_year || oneTime.tm_mon != otherTime.tm_mon || oneTime.tm_mday != otherTime.tm_mday;
}

std::string dayOfWeekAbbreviation(const time_t &rawtime)
{
	struct tm *timeInfo = localtime(&rawtime);
	std::array<char,15> abbreviation;
	int length = strftime(abbreviation.data(), abbreviation.size(), "%a", timeInfo);
	return length ? abbreviation.data() : "☼";
}

std::string seconds2hhmm(int64_t s)
{
	stringstream retVal;
	int64_t minutes;
	int64_t hours;
	hours = s / (60 * 60);
	s -= hours * (60 * 60);
	minutes = s / 60;
	retVal << (hours < 10 ? "\u2007" : "") << hours << " h " << (minutes < 10 ? hours ? "0" : "\u2007" : "") << minutes << " m";
	return retVal.str();
}

std::string smallNumbers(std::string s)
{
	stringstream retVal;
	for (auto c: s)
	{
		switch (c) {
			case '0': retVal << "₀"; break;
			case '1': retVal << "₁"; break;
			case '2': retVal << "₂"; break;
			case '3': retVal << "₃"; break;
			case '4': retVal << "₄"; break;
			case '5': retVal << "₅"; break;
			case '6': retVal << "₆"; break;
			case '7': retVal << "₇"; break;
			case '8': retVal << "₈"; break;
			case '9': retVal << "₉"; break;
			case '-': retVal << "₋"; break;
			default: retVal << c;
		}
	}
	return retVal.str();
}

std::string createDurationString(const time_t &from, const time_t &to)
{
	stringstream retVal;
	struct tm fromTime = *localtime(&from);
	struct tm toTime = *localtime(&to);
	bool acrossDays = fromTime.tm_year != toTime.tm_year || fromTime.tm_mon != toTime.tm_mon || fromTime.tm_mday != toTime.tm_mday;
	retVal << (fromTime.tm_year + 1900) << "-" << setfill('0') << setw(2) << fromTime.tm_mon + 1 << "-" << setfill('0') << setw(2) << fromTime.tm_mday << (acrossDays ? " " : "\u2003")
			<< setfill('0') << setw(2) << fromTime.tm_hour << ":" << setfill('0') << setw(2) << fromTime.tm_min;
	retVal << " → ";
	if (acrossDays)
	{
		retVal << (toTime.tm_year + 1900) << "-" << setfill('0') << setw(2) << toTime.tm_mon + 1 << "-" << setfill('0') << setw(2) << toTime.tm_mday << " ";
	}
	retVal << setfill('0') << setw(2) << toTime.tm_hour << ":" << setfill('0') << setw(2) << toTime.tm_min;
	retVal << (acrossDays ? " " : "\u2003") << "= " << seconds2hhmm(difftime(to, from)) << (acrossDays ? "" : "\u2003");
	return retVal.str();
}

std::string createIdlingString(const time_t &to, const time_t &next)
{
	stringstream retVal;
	int64_t toNextMin = difftime(next, to) / 60;
	retVal << "⇣ " << smallNumbers(std::to_string(toNextMin)) << " ₘ";
	return retVal.str();
}

time_t now()
{
	return time(NULL);
}

uint64_t millisecondsSinceEpoch()
{
	struct timeval tv;
	gettimeofday(&tv, 0);

	return uint64_t(tv.tv_sec) * 1000 + tv.tv_usec / 1000;
}

}