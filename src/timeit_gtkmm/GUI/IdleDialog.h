/*
 * IdleDialog.h
 *
 *  Created on: 2008-sep-06
 *      Author: hoglet
 */

#ifndef IDLEDIALOG_H_
#define IDLEDIALOG_H_

#include <gtkmm.h>
#include <libtimeit/Timer.h>
#include <IWidget.h>
#include <memory>
#include <libtimeit/db/TaskAccessor.h>

namespace GUI
{
using namespace libtimeit;

class IIdleDialog
{
public:
	virtual ~IIdleDialog() {};
	virtual void setIdleStartTime(time_t idleStartTime) = 0;
	virtual void setActiveTaskList(std::vector<int64_t> activeTaskIDs) = 0;
	virtual void show() = 0;
	virtual void attach(ActionObserver*) = 0;
	virtual void detach(ActionObserver*) = 0;
};

enum IdleDialogResponse{
	RESPONSE_REVERT=1,
	RESPONSE_REVERTANDCONTINUE =2,
	RESPONSE_CONTINUE=3
} ;

class IdleDialog : public Gtk::Dialog, public TimerObserver, public IIdleDialog, public IWidget
{
public:
	IdleDialog(Timer& timer, Database& database);
	virtual void setIdleStartTime(time_t idleStartTime);
	virtual void setActiveTaskList(std::vector<int64_t> activeTaskIDs);
	virtual ~IdleDialog();
	virtual void attach(ActionObserver*);
	virtual void detach(ActionObserver*);
	// IWidget interface
	virtual void show();
	virtual void hide() { Gtk::Dialog::hide(); }
	virtual void move(int x, int y) { Gtk::Dialog::move(x,y); };
	virtual bool is_visible() { return Gtk::Dialog::is_visible(); } ;
	virtual void get_position(int& Window_x, int& Window_y) { Gtk::Dialog::get_position(Window_x, Window_y); };
private:
	virtual void on_signal_1_second() {};
	virtual void on_signal_10_seconds();
	void setText();
	void updater();
	void run();
	Gtk::Label  text;
	Gtk::Button revertButton;
	Gtk::Button revertAndContinueButton;
	Gtk::Button continueButton;
	Timer& m_timer;
	time_t m_idleStartTime;
	std::string taskString;
	TaskAccessor taskAccessor;

	std::list<ActionObserver*> observers;
	void responseHandler(int result);
};

}
#endif /* IDLEDIALOG_H_ */
