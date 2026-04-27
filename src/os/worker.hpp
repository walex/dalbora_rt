#ifndef __worker_hpp__
#define __worker_hpp__

#include "platform.hpp"

struct WorkerParams {
	virtual ~WorkerParams() = default;
	unsigned int coreId;
};


class Worker  {
public:
	Worker(const std::vector<unsigned int> core_ids, std::function<void()> wcb);
	virtual ~Worker();
	bool isRunning();
protected:
	virtual void onStartWorker();
	virtual void onStopWorker() {}
	const WorkerParams& getParameters() { return *mParameters.get(); }
private:
	static void threadProc(Worker& instance);

	std::shared_ptr<WorkerParams> mParameters;
	std::atomic<bool> mIsRunning{ false };
	std::thread mThread;
	std::function<void()> mWorkerCB;
	std::vector<unsigned int> mCoreIds;
};

#endif