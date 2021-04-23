/*
 * IdleDialog.cpp
 *
 *  Created on: 2008-sep-06
 *      Author: hoglet
 */

#include "idle_dialog.h"
#include <iostream>
#include <string>
#include <sstream>
#include <glibmm/i18n.h>
#include "libtimeit/timer.h"
#include <libtimeit/utils.h>

namespace GUI
{

using namespace::std;
using namespace libtimeit;

IdleDialog::IdleDialog(
		Timer& timer,
		Database& database)
		:
		Timer_observer(timer),
		m_timer(timer),
		taskAccessor(database)
{

	//Setting start time to now in case nobody will set the idle time later.
	m_idleStartTime = libtimeit::now();

	set_deletable(false);
	setText();
	get_vbox()->pack_start(text);
	// This is one answer to the question "No activity have
	// been detected for X minutes. What should we do?"
	add_button(_("Revert and stop"), RESPONSE_REVERT);
	// This is one answer to the question "No activity have
	// been detected for X minutes. What should we do?"
	add_button(_("Revert and continue"), RESPONSE_REVERTANDCONTINUE);
	// This is one answer to the question "No activity has
	// been detected for X minutes. What should we do?"
	add_button(_("Continue"), RESPONSE_CONTINUE);


	signal_response().connect(sigc::mem_fun(this, &IdleDialog::responseHandler));

	//set_type_hint(Gdk::WindowTypeHint::WINDOW_TYPE_HINT_DIALOG);
	show_all_children();
	set_keep_above(true);
}

void IdleDialog::attach(action_observer *observer)
{
	observers.push_back(observer);
}
void IdleDialog::detach(action_observer *observer)
{
	observers.remove(observer);
}

void IdleDialog::setIdleStartTime(time_t idleStartTime)
{
	m_idleStartTime = idleStartTime;
}

void IdleDialog::setActiveTaskList(vector<int64_t> activeTaskIDs)
{
	std::stringstream text;
	int i = 0;
	for (int64_t taskID : activeTaskIDs)
	{
		auto task = taskAccessor.by_ID(taskID);
		if (i++ > 0)
		{
			text << ", ";
		}
		text << task->name;
	}
	taskString = text.str();
}

void IdleDialog::on_signal_10_seconds()
{
	setText();
}

void IdleDialog::show()
{
	setText();
	Gtk::Dialog::show();
}

void IdleDialog::setText()
{
	std::stringstream str;
	int minutesIdle = (libtimeit::now() - m_idleStartTime) / 60;

	// %d represents the time
	std::string format_str = ngettext("No activity has been detected for %d minute. What should we do?",
			"No activity has been detected for %d minutes. What should we do?",
			minutesIdle);
	str << libtimeit::string_printf(format_str, minutesIdle);

	if (taskString.size() > 0)
	{
		str << "\n\n";
		//Context: Before this string will be "No activity has been detected for %d minutes. What should we do?" and after this txt it will be a list of tasks
		str << ngettext("Task affected: ", "Tasks affected: ", taskString.size());
		str << taskString << std::endl;
	}

	text.set_text(str.str());
}

void IdleDialog::responseHandler(int result)
{
	std::list<action_observer*>::iterator iter;
	std::list<action_observer*> observers = this->observers;
	for (iter = observers.begin(); iter != observers.end(); ++iter)
	{
		action_observer *observer = *iter;
		switch (result)
		{
		case (RESPONSE_REVERT):
			observer->on_action_revertAndStop();
			break;
		case (RESPONSE_REVERTANDCONTINUE):
			observer->on_action_revertAndContinue();
			break;
		case (RESPONSE_CONTINUE):
			observer->on_action_continue();
			break;
		default:
			cout << "Unexpected button clicked." << std::endl;
			break;
		}
	}
	hide();
}

}