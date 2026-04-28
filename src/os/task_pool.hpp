#ifndef __os_task_pool_hpp__
#define __os_task_pool_hpp__

#include "platform.hpp"
#include "queue.hpp"
#include "worker.hpp"

template <typename TaskData>
class TaskPool {

public:
	TaskPool(unsigned int core_count = std::thread::hardware_concurrency())
		: mCoreCount(core_count) {
		this->startWorkers();
	}

	virtual ~TaskPool() {
		this->stopWorkers();
	}

	void startWorkers() {

		if (mCoreCount > std::thread::hardware_concurrency()) {
			mCoreCount = std::thread::hardware_concurrency();
			printf("Warning: requested core count %d exceeds hardware concurrency %d, using %d detected cores count instead.\n", mCoreCount, std::thread::hardware_concurrency(), std::thread::hardware_concurrency());
		}
		else if (mCoreCount < 1) {
			mCoreCount = 1;
			printf("Warning: requested core count %d is less than 1, using 1 core instead.\n", mCoreCount);
		}
		mWorkers.reserve(mCoreCount);
		for (unsigned int i = 0; i < mCoreCount; ++i) {
			
			std::vector<unsigned int> cores = { i };
			mWorkers.emplace_back(std::make_shared<Worker>(cores, [this]() {
				TaskData task;
				if (mTaskQueue.pop(task)) {
					this->onTask(task);
				}
			}));
		}
	}
	
	void setCores(int count) {
		bool running = mWorkers.size() > 0;
		this->stopWorkers();
		mCoreCount = count;
		if (running == true)
			this->startWorkers();
	}

	void newTask(TaskData data) {
		mTaskQueue.push(data);
	}
	
	virtual void onTask(const TaskData data) = 0;

	void stopWorkers() {

		mTaskQueue.clear();
		mWorkers.clear();
	}
	
private:
	unsigned int mCoreCount;
	stdext::Queue<TaskData> mTaskQueue;
	std::vector<std::shared_ptr<Worker>> mWorkers;
};

#endif