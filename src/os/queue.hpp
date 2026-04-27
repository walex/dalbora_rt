#ifndef __QUEUE_HPP__
#define __QUEUE_HPP__

#include "platform.hpp"

namespace stdext {
	template <typename T, int maxElements = (std::numeric_limits<int>::max)()>
	class Queue : private std::queue<T>
	{
	public:
		T& emplace(uint64_t c, float v) {
			std::lock_guard lck(mtx);
			if (reinterpret_cast<std::queue<T>*>(this)->size() < maxElements) {
				reinterpret_cast<std::queue<T>*>(this)->emplace(c, v);
				sem.release();
				return reinterpret_cast<std::queue<T>*>(this)->back();
			}
		}
		bool push(const T& t) {
			std::lock_guard lck(mtx);
			if (reinterpret_cast<std::queue<T>*>(this)->size() < maxElements) {
				reinterpret_cast<std::queue<T>*>(this)->push(std::move(t));
				sem.release();
				return true;
			}
			return false;
		}

		bool pop(T& t, uint64_t tout = 10000000000) {

			constexpr uint64_t one_second_ms = 500;
			while (tout > 0) {

				const uint64_t sleep_time_delta_ms = std::min(tout, one_second_ms);
				if (sem.try_acquire_for(std::chrono::milliseconds(sleep_time_delta_ms))) {

					std::lock_guard lck(mtx);
					if (this->size() > 0) {

						t = std::move(this->front());
						reinterpret_cast<std::queue<T>*>(this)->pop();
						return true;
					}
				}
				tout -= sleep_time_delta_ms;
			}
			return false;
		}
		void pop_all(std::function<void(T&)> cb) {

			std::lock_guard lck(mtx);
			while (sem.try_acquire());
			T t;
			while (this->size() > 0) {
				t = std::move(this->front());
				reinterpret_cast<std::queue<T>*>(this)->pop();
				if (cb)
					cb(t);
			}
		}
		void clear() {
			this->pop_all(nullptr);
		}
		size_t size() {
			return reinterpret_cast<std::queue<T>*>(this)->size();
		}
	private:
		std::recursive_mutex mtx;
		std::counting_semaphore<maxElements> sem{ 0 };
	};
}

#endif