#include <sstream>
#include <libtimeit/db/task_accessor.h>
#include <libtimeit/utils.h>

namespace libtimeit
{

using namespace std;

Task_accessor::Task_accessor(	Database& op_database )
		:
		database(op_database)
{
}



void Task_accessor::create_table()
{
	database.execute( R"Query(
		CREATE TABLE IF NOT EXISTS
 			tasks
			 (
				id         INTEGER PRIMARY KEY,
				name        VARCHAR,
				parent      INTEGER,
				changed     INTEGER,
				uuid        TEXT UNIQUE,
				expanded    BOOL DEFAULT    0,
				deleted     BOOL DEFAULT    0,
			 	completed   BOOL DEFAULT    0,
				idle        INTEGER DEFAULT 0,
				quiet       BOOL DEFAULT    0,
			 FOREIGN KEY(parent) REFERENCES tasks(id)
			)
			)Query"
			);
}

void Task_accessor::enable_notifications(bool state)
{
	database.enable_notifications(state);
}

vector<Task> Task_accessor::by_parent_ID(int64_t parent)
{
	vector<Task> return_value;
	stringstream statement;

	statement << "SELECT id, parent, name, completed, uuid, changed, idle, quiet"
			"  FROM "
			"    tasks"
			"  WHERE deleted=0";
	if (parent > 0)
	{
		statement << " AND parent=" << parent;
	}
	else
	{
		statement << " AND parent IS NULL ";
	}

	Query_result rows = database.execute(statement.str());
	for (vector<Data_cell> row : rows)
	{
		int id = row[0].integer();
		parent = 0;
		if (row[1].has_value())
		{
			parent = row[1].integer();
		}
		string name        = row[2].text();
		bool completed     = row[3].boolean();
		auto l_uuid        = UUID::from_string(row[4].text());
		time_t last_change = row[5].integer();
		int    idle        = row[6].integer();
		bool   quiet       = row[7].boolean();
		if(l_uuid.has_value())
		{
			Task task(name, parent, *l_uuid, completed, id, last_change, uuid(parent), false, idle, quiet);
			return_value.push_back(task);
		}
	}
	return return_value;
}

optional<Task> Task_accessor::by_ID(int64_t taskID)
{
	Statement statement_getTask = database.prepare(
			R"(
				SELECT
					id,
					parent,
					name,
					completed,
					uuid,
					changed,
					deleted,
					idle,
					quiet
				FROM
					tasks
				WHERE
					id=?)");
	statement_getTask.bind_value(1, taskID);
	Query_result rows = statement_getTask.execute();
	shared_ptr<Task> task;
	if (rows.size() == 1)
	{
		vector<Data_cell> row = rows.at(0);
		int64_t id = row[0].integer();
		int64_t parent = 0;
		if (row[1].has_value())
		{
			parent = row[1].integer();
		}
		string name       = row[2].text();
		bool completed    = row[3].boolean();
		auto l_uuid       = UUID::from_string(row[4].text());
		time_t lastChange = row[5].integer();
		bool deleted      = row[6].boolean();
		int  idle         = row[7].integer();
		bool quiet        = row[8].boolean();
		if(l_uuid.has_value())
		{
			auto parentUUID = uuid(parent);
			return Task(name, parent, *l_uuid, completed, id, lastChange, parentUUID, deleted, idle, quiet);
		}
	}
	return {};
}

optional<Task> Task_accessor::get_task_unlimited(Task_ID taskID)
{
	Statement statement_getCompleteTask = database.prepare(R"(
		SELECT
			id,
			parent,
			name,
			completed,
			uuid,
			changed,
			deleted,
			idle,
			quiet
		FROM
			tasks
		WHERE
			id=?;
		)");
	statement_getCompleteTask.bind_value(1, taskID);
	Query_result rows = statement_getCompleteTask.execute();
	if (rows.size() == 1)
	{
		vector<Data_cell> row = rows.at(0);
		int64_t id = row[0].integer();
		int64_t parent = 0;
		if (row[1].has_value())
		{
			parent = row[1].integer();
		}
		string name       = row[2].text();
		bool   completed  = row[3].boolean();
		auto   l_uuid     = UUID::from_string(row[4].text());
		time_t lastChange = row[5].integer();
		bool   deleted    = row[6].boolean();
		int    idle       = row[7].integer();
		bool   quiet      = row[8].boolean();
		if(l_uuid.has_value())
		{
			return {Task(name, parent, *l_uuid, completed, id, lastChange, uuid(parent), deleted, idle, quiet)};
		}
	}
	return {};
}

vector<Task> Task_accessor::changed_since(time_t timestamp)
{
	vector<Task> retVal;
	stringstream statement;

	statement << R"(
			SELECT
				id,
				parent,
				name,
				completed,
				uuid,
				changed,
				deleted,
				idle,
				quiet
			FROM
			    tasks
			WHERE
				changed > )"
				<< timestamp;

	Query_result rows = database.execute(statement.str());
	for (std::vector<Data_cell> row : rows)
	{
		int id = row[0].integer();
		int parent = 0;
		if (row[1].has_value())
		{
			parent = row[1].integer();
		}
		string name       = row[2].text();
		bool   completed  = row[3].boolean();
		auto   l_uuid     = UUID::from_string(row[4].text());
		time_t lastChange = row[5].integer();
		bool   deleted    = row[6].boolean();
		int    idle       = row[7].integer();
		bool   quiet      = row[8].boolean();
		if (l_uuid.has_value())
		{
			auto parentUUID= uuid(parent);
			Task task(name, parent, *l_uuid, completed, id, lastChange, parentUUID, deleted, idle, quiet);
			retVal.push_back(task);
		}
	}
	return retVal;
}

Task_ID Task_accessor::ID(UUID uuid)
{

	Task_ID id = 0;
	Statement statement_uuidToId = database.prepare("SELECT id FROM tasks WHERE uuid=?;");
	statement_uuidToId.bind_value(1, uuid.c_str());
	Query_result rows = statement_uuidToId.execute();
	for (vector<Data_cell> row : rows)
	{
		id = row[0].integer();
	}
	return id;
}

optional<class UUID> Task_accessor::uuid(Task_ID id)
{
	if(id)
	{
		Statement statement_idToUuid = database.prepare("SELECT uuid FROM tasks WHERE id=?;");
		statement_idToUuid.bind_value(1, id);
		Query_result rows = statement_idToUuid.execute();
		for (vector<Data_cell> row : rows)
		{
			auto uuid = row[0].text();
			return UUID::from_string(uuid);
		}
	}
	return {};
}

void Task_accessor::_update(const Task &task)
{
	Statement statement_updateTask = database.prepare(
			R"(
				UPDATE
					tasks
				SET
					name = ?,
					parent = ? ,
					changed = ? ,
					deleted = ?,
					completed = ?,
					idle = ?,
					quiet = ?
				WHERE id=?;)"
						);
	statement_updateTask.bind_value(1, task.name);
	if (task.parent_id > 0)
	{
		statement_updateTask.bind_value(2, task.parent_id);
	}
	else
	{
		statement_updateTask.bind_null_value(2);
	}
	statement_updateTask.bind_value(3, task.last_changed);
	statement_updateTask.bind_value(4, task.deleted);
	statement_updateTask.bind_value(5, task.completed);
	statement_updateTask.bind_value(6, task.idle);
	statement_updateTask.bind_value(7, task.quiet);
	statement_updateTask.bind_value(8, task.id);

	statement_updateTask.execute();
}

void Task_accessor::notify(const Task &existingTask, const Task &task)
{
	bool parentChanged = (existingTask.parent_id != task.parent_id);
	bool taskDeleted = (existingTask.deleted != task.deleted && task.deleted == true);
	bool nameChanged = (existingTask.name != task.name);

	if (nameChanged)
	{
		database.send_notification(TASK_NAME_CHANGED, task.id);
	}
	if (parentChanged)
	{
		database.send_notification(TASK_PARENT_CHANGED, task.id);
	}
	if (taskDeleted)
	{
		database.send_notification(TASK_REMOVED, task.id);
	}
}

bool Task_accessor::update(const Task &task)
{
	int id = task.id;
	if (id == 0)
	{
		id = ID(task.uuid);
		if (id == 0)
		{
			stringstream message;
			message << "Unable to find task with UUID=" << task.uuid.c_str();
			throw db_exception(message.str());
		}
	}
	auto existingTask = get_task_unlimited(id);
	if (existingTask.has_value() &&  task != *existingTask && task.last_changed >= existingTask->last_changed)
	{

		_update(task);
		notify(*existingTask, task);
		return true;
	}
	return false;
}

Task_ID Task_accessor::create(const Task &task)
{
	Statement statement_newTask = database.prepare(
			R"Query(
				INSERT INTO
					tasks (name,parent,changed,uuid,completed,deleted,idle,quiet)
				VALUES (?,?,?,?,?,?,?,?);
				)Query");
	statement_newTask.bind_value(1, task.name);
	if (task.parent_id > 0)
	{
		statement_newTask.bind_value(2, task.parent_id);
	}
	else
	{
		statement_newTask.bind_null_value(2);
	}
	statement_newTask.bind_value(3, task.last_changed);
	statement_newTask.bind_value(4, task.uuid.c_str());
	statement_newTask.bind_value(5, task.completed);
	statement_newTask.bind_value(6, task.deleted);
	statement_newTask.bind_value(7, task.idle);
	statement_newTask.bind_value(8, task.quiet);

	statement_newTask.execute();
	Task_ID id = database.ID_of_last_insert();

	database.send_notification(TASK_ADDED, id);
	return id;
}

void Task_accessor::set_parent_id(Task_ID taskID, Task_ID parentID)
{
	time_t now = time(nullptr);
	stringstream statement;
	if (parentID == 0)
	{
		statement << "UPDATE tasks SET parent = NULL ";
	}
	else
	{
		statement << "UPDATE tasks SET parent = " << parentID;
	}
	statement << " , changed = " << now;
	statement << " WHERE id=" << taskID;
	database.execute(statement.str());
	database.send_notification(TASK_PARENT_CHANGED, taskID);
}

void Task_accessor::remove(int64_t taskID)
{
	time_t now = time(nullptr);
	stringstream statement;
	statement << "UPDATE tasks SET deleted = " << true;
	statement << " ,changed = " << now;
	statement << " WHERE id = " << taskID;
	database.execute(statement.str());
	database.send_notification(TASK_REMOVED, taskID);
}

void Task_accessor::upgrade_to_DB5()
{
	time_t now = time(nullptr);
//Update Tasks to new design
	database.execute("ALTER TABLE tasks RENAME TO tasks_backup");
	create_table();
	database.execute("UPDATE tasks_backup SET deleted = 0 WHERE deleted != 1");
	database.execute("UPDATE tasks_backup SET parent = NULL WHERE parent = 0");
	Statement statement = database.prepare("SELECT id, parent, name, deleted FROM  tasks_backup");
	Query_result rows = statement.execute();
	for (vector<Data_cell> row : rows)
	{
		int64_t id = row[0].integer();
		int64_t parent = 0;
		if (row[1].has_value())
		{
			parent = row[1].integer();
		}
		string name      = row[2].text();
		bool   deleted   = row[3].boolean();
		bool   completed = false;
		time_t changed   = now;

		Task task(name, parent, UUID(), completed, id, changed, {}, deleted, 0, false);
		create(task);
	}
	database.execute("DROP TABLE tasks_backup");

}

void Task_accessor::upgrade()
{
	if ( database.current_DB_version() <5 )
	{
		upgrade_to_DB5();
	}
	if( ! database.column_exists("tasks", "idle") )
	{
		auto query = R"Query(
			ALTER TABLE
				tasks
			ADD
				idle        INTEGER
			DEFAULT 0
				;
			)Query";
		database.execute( query );
		auto query2 = R"Query(
			ALTER TABLE
				tasks
			ADD
				quiet      BOOL
			DEFAULT 0
				;
			)Query";
		database.execute( query2 );
	}
}



void Task_accessor::set_task_expanded(Task_ID taskID, bool expanded)
{
	auto statement = string_printf(
			R"Query(
				UPDATE
					tasks
				SET
					expanded = %d
				WHERE
					id= %d ;
			)Query"
					, expanded, taskID);
	database.execute(statement);
}

}

