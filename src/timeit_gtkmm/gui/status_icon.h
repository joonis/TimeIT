/*
 * StatusIcon.h
 *
 *  Created on: 21-Jun-08
 *      Author: hoglet
 */

#ifndef STATUSICON_H_
#define STATUSICON_H_

#include <gtkmm.h>

#include "libtimeit/logic/time_keeper.h"
#include <libtimeit/db/task_accessor.h>
#include <libtimeit/db/time_accessor.h>
#include "action_observer.h"
#include "IStatusIcon.h"

namespace GUI
{
using namespace libtimeit;

class StatusIcon: public Time_keeper_observer, public IStatusIcon, public Event_observer
{
public:
	virtual ~StatusIcon();
	StatusIcon(Time_keeper&, Database&, Notifier& notifier);
	virtual void show()
	{
	}
	; //Shown directly on creation. Might change
	virtual void attach(action_observer *observer);
	virtual void detach(action_observer *observer);
private:
	void on_activate();
	void setIcon();
	void setTooltip();
	void on_running_changed();
	void on_activity_resumed()
	{};
	void on_idle_detected()
	{};
	void on_popup_menu(guint button, guint32 activate_time);
	void toggleMainWindow();
	void on_menu_file_popup_quit();
	void on_menu_file_popup_open();
	void on_menu_stop_all_timers();
	void on_menu_about();
	void on_menu_preferences();
	void on_menu_toggle_task1();
	void on_menu_toggle_task2();
	void on_menu_toggle_task3();
	void on_menu_toggle_task4();
	void on_menu_toggle_task5();
	void populateContextMenu();
	void toggleTask(int64_t id);
	std::string completeTaskPath(int64_t id);

	virtual void on_task_added(int64_t)
	{
	}
	;
	virtual void on_task_updated(int64_t);
	virtual void on_task_name_changed(int64_t);
	virtual void on_task_time_changed(int64_t);

	virtual void on_task_removed(int64_t)
	{
	}
	;
	virtual void on_complete_update();
	Glib::RefPtr<Gtk::StatusIcon> m_statusIcon;
	Glib::RefPtr<Gdk::Pixbuf> m_defaultIcon;
	Glib::RefPtr<Gdk::Pixbuf> m_runningIcon;
	Glib::RefPtr<Gdk::Pixbuf> idleIconSmall;
	Glib::RefPtr<Gdk::Pixbuf> runningIconSmall;

	Gtk::Menu m_Menu_Popup;
	Time_keeper& m_timeKeeper;
	Task_accessor m_taskaccessor;
	Time_accessor m_timeaccessor;
	std::list<action_observer*> observers;
	std::vector<int64_t> latestTasks;

};

}
#endif /* STATUSICON_H_ */