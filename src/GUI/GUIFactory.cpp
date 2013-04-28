/*
 * GUIFactory.cpp
 *
 *  Created on: 2008-nov-22
 *      Author: hoglet
 */

#include "GUIFactory.h"
#include "internal/MainWindow/MainWindow.h"
#include "internal/TimeItAboutDialog.h"
#include "internal/EditTaskDialog.h"
#include "internal/AddTime.h"
#include "internal/IdleDialog.h"
#include "internal/StatusIcon.h"
#include "internal/DetailsDialog.h"
#include "internal/PreferenceDialog.h"
#include <memory>

namespace GUI
{
using namespace GUI::Internal;

std::shared_ptr<Gtk::Main> GUIFactory::main;

GUIFactory::GUIFactory(std::shared_ptr<ITimeKeeper>& timekeeper, std::shared_ptr<DB::Database>& database,
		const std::shared_ptr<Timer>& timer)
{
	this->timekeeper = timekeeper;
	this->timer = timer;
	this->database = database;

}

GUIFactory::~GUIFactory()
{
	main.reset();
}

void GUIFactory::init(int argc, char *argv[])
{
	main = std::shared_ptr<Gtk::Main>(new Gtk::Main(argc, argv));
}
void GUIFactory::quit()
{
	Gtk::Main::quit();
}

void GUIFactory::run()
{
	if (main)
	{
		main->run();
	}
}

WidgetPtr GUIFactory::getWidget(EWidget widget)
{
	WidgetPtr retVal;
	switch (widget)
	{
	case MAIN_WINDOW:
		if (mainWindow == 0)
		{
			std::shared_ptr<MainWindow> window(new MainWindow( database));
			window->signal_hide().connect(sigc::mem_fun(this, &GUIFactory::on_mainWindow_hide));
			this->mainWindow = window;

		}
		retVal = mainWindow;
		break;
	case ADD_TASK_DIALOG:
/*		if (addTaskInstance == 0)
		{
			std::shared_ptr<AddTaskDialog> dialog(new AddTaskDialog(database));
			dialog->signal_hide().connect(sigc::mem_fun(this, &GUIFactory::on_addTaskDialog_hide));
			addTaskInstance = dialog;
		}
		retVal = addTaskInstance;
		break;*/
	case EDIT_TASK_DIALOG:
		if (editTaskDialogInstace == 0)
		{
			std::shared_ptr<EditTaskDialog> dialog(new EditTaskDialog(database));
			dialog->signal_hide().connect(sigc::mem_fun(this, &GUIFactory::on_editTask_hide));
			editTaskDialogInstace = dialog;
		}
		retVal = editTaskDialogInstace;
		break;
	case ABOUT_DIALOG:
		if (aboutDialogInstance == 0)
		{
			std::shared_ptr<TimeItAboutDialog> dialog(new TimeItAboutDialog());
			dialog->signal_response().connect(sigc::mem_fun(this, &GUIFactory::on_aboutDialog_response));
			aboutDialogInstance = dialog;
		}
		retVal = aboutDialogInstance;
		break;
	case IDLE_DIALOG:
		if (idleDialogInstance == 0)
		{
			std::shared_ptr<IdleDialog> dialog(new IdleDialog(timer));
			dialog->signal_hide().connect(sigc::mem_fun(this, &GUIFactory::on_idleDialog_hide));
			idleDialogInstance = dialog;
		}
		retVal = idleDialogInstance;
		break;
	case DETAILS_DIALOG:
		if (detailsDialogInstance == 0)
		{
			std::shared_ptr<DetailsDialog> dialog = DetailsDialog::create(database);
			dialog->signal_hide().connect(sigc::mem_fun(this, &GUIFactory::on_detailsDialog_hide));
			detailsDialogInstance = dialog;
		}
		retVal = detailsDialogInstance;
		break;
	case PREFERENCE_DIALOG:
		if(preferenceDialogInstance == 0)
		{
			std::shared_ptr<PreferenceDialog> dialog(new PreferenceDialog(database));
			dialog->signal_hide().connect(sigc::mem_fun(this, &GUIFactory::on_preferenceDialog_hide));
			preferenceDialogInstance = dialog;
		}
		retVal = preferenceDialogInstance;
		break;
	case MAX_WIDGETS:
	default:
		break;
	}
	return retVal;
}
void GUIFactory::on_mainWindow_hide()
{
	//mainWindow.reset();
}
void GUIFactory::on_addTaskDialog_hide()
{
	addTaskInstance.reset();
}

void GUIFactory::on_idleDialog_hide()
{
	idleDialogInstance.reset();
}

void GUIFactory::on_detailsDialog_hide()
{
	detailsDialogInstance.reset();
}

void GUIFactory::on_aboutDialog_response(int)
{
	aboutDialogInstance->hide();
	aboutDialogInstance.reset();
}

void GUIFactory::on_editTask_hide()
{
	editTaskDialogInstace.reset();
}

void GUIFactory::on_preferenceDialog_hide()
{
	preferenceDialogInstance.reset();
}


IStatusIcon& GUIFactory::getStatusIcon()
{
	static IStatusIcon* statusIcon = 0;
	if (statusIcon == 0)
	{
		std::shared_ptr<ITaskAccessor> taskaccessor=database->getTaskAccessor();
		std::shared_ptr<ITimeAccessor> timeaccessor=database->getTimeAccessor();

		statusIcon = (IStatusIcon*) (new StatusIcon(timekeeper, taskaccessor, timeaccessor ));
	}
	return *statusIcon;
}
WidgetPtr GUIFactory::getAddTime(int64_t taskID)
{
	if (!addTimeInstance)
	{
		if (mainWindow == 0)
		{
			getWidget(MAIN_WINDOW);
		}
		ICalendar& calendar = std::dynamic_pointer_cast<IMainWindow>(mainWindow)->getCalendar();
		std::shared_ptr<AddTime> addTime(new AddTime(taskID, calendar, database));
		addTime->signal_response().connect(sigc::mem_fun(this, &GUIFactory::on_addTime_response));
		addTimeInstance = addTime;
	}
	return addTimeInstance;
}
void GUIFactory::on_addTime_response(int)
{
	addTimeInstance->hide();
	addTimeInstance.reset();
}

}

