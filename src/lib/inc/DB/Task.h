#pragma once

#include <string>

namespace Test
{
class TestTaskC;
class Network_test;
}
;

namespace DB
{
class TaskAccessor;

class Task
{
	friend class TaskAccessor;
	friend class SyncManager;

public:
	Task(const std::string &name, int64_t parentID = 0);
	Task(const std::string &name,
			int64_t parentID,
			const std::string &uuid,
			bool completed,
			int64_t ID,
			time_t lastChange,
			const std::string &parentUUID,
			bool deleted);

	virtual ~Task();

	const std::string name()        const;
	int64_t           ID()          const;
	int64_t           parentID()    const;
	bool              completed()   const;
	std::string       UUID()        const;
	std::string       parentUUID()  const;
	time_t            lastChanged() const;
	bool              deleted()     const;

	Task              withName(std::string newName) const;
	Task              withParent(int64_t)           const;
	Task              withCompleted(bool)           const;
	Task              withDeleted(bool)             const;

	friend bool       operator==(const Task &op1, const Task &op2);
	friend bool       operator!=(const Task &op1, const Task &op2);

protected:
	Task();

	int64_t     ID_          {0};
	std::string name_;
	std::string uuid_;
	std::string parentUuid_;
	bool        completed_   {false};
	bool        deleted_     {false};
	int64_t     parentID_    {0};
	time_t      lastChanged_ {0};

private:
	void operator=(const Task&) {};
};

extern bool operator==(const Task &op1, const Task &op2);
extern bool operator!=(const Task &op1, const Task &op2);
}
