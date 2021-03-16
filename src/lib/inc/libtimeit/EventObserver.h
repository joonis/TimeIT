#ifndef TIMEIT_EVENTOBSERVER_H_
#define TIMEIT_EVENTOBSERVER_H_

#include <cstdint>
#include <string>

namespace libtimeit
{

enum class EventType
{
	ERROR_MESSAGE,
	INFO_MESSAGE
};

class EventObserver
{
public:
	virtual      ~EventObserver() = default;
	virtual void on_message(EventType type, const std::string headline, const std::string message) const {};
	virtual void on_taskParentChanged(int64_t)  {};
	virtual void on_taskAdded(int64_t)          {};
	virtual void on_taskUpdated(int64_t)        {};
	virtual void on_taskRemoved(int64_t)        {};
	virtual void on_completeUpdate()            {};
	virtual void on_taskNameChanged(int64_t)    {};
	virtual void on_taskTimeChanged(int64_t)    {};
	virtual void on_action_showMainWindow()     {};
};

}
#endif // TIMEIT_EVENTOBSERVER_H_