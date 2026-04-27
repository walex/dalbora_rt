#include "worker.hpp"

Worker::Worker(std::vector<unsigned int> core_ids, std::function<void()> wcb)
	: mWorkerCB(wcb)
	, mCoreIds(core_ids)
	, mThread(std::thread(&Worker::threadProc, std::ref(*this))) {

	}

Worker::~Worker() {

	mIsRunning.store(false);
	if (mThread.joinable() == true)
		mThread.join();
}

void Worker::onStartWorker() {
	
	set_thread_affinity(mCoreIds);
}

bool Worker::isRunning() {
	return mIsRunning.load();
}

void Worker::threadProc(Worker& instance) {

	instance.mIsRunning.store(true);
	instance.onStartWorker();
	while (instance.mIsRunning.load())
		instance.mWorkerCB();
	instance.onStopWorker();
}