#pragma once

#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;

class JobItem
{
public:
	virtual ~JobItem(){}
	virtual void execute() = 0;
};

class ThreadPool {
public:
	ThreadPool(unsigned char thrCount = 0) : done(false)
	{
		auto numberOfThreads = std::thread::hardware_concurrency();
		if (numberOfThreads == 0) {
			numberOfThreads = 1;
		}

		if (thrCount > 0 && thrCount < numberOfThreads)
		{
			m_workerCount = thrCount;
		}
		else
		{
			m_workerCount = numberOfThreads;
		}

		for (unsigned i = 0; i < m_workerCount; ++i) {
			m_threads.push_back(std::thread(&ThreadPool::doWork, this));
		}
	}

	~ThreadPool() {
		done = true;
		m_queueCondVariable.notify_all();
		for (auto& thread : m_threads) {
			if (thread.joinable()) {
				thread.join();
			}
		}

		while(!m_workQueue.empty())
		{
			JobItem *job = m_workQueue.front();
			m_workQueue.pop();
			delete job;
		}
		
	}

	void queueWork(JobItem *job) {
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_workQueue.push(job);
		}

		// Notify one thread that there are requests to process
		m_queueCondVariable.notify_one();
	}

	int getWorkerCount()
	{
		return m_workerCount;
	}

private:
	int m_workerCount;
	std::condition_variable_any m_queueCondVariable;
	std::vector<std::thread> m_threads;
	std::mutex m_mutex;
	std::queue<JobItem*> m_workQueue;

	bool done;
	void doWork() {

		while (!done) {
			JobItem *job;
			
			{
				std::unique_lock<std::mutex> g(m_mutex);
				if (!done && m_workQueue.empty())
				{
					m_queueCondVariable.wait(g);
				}

				if (done) {
					break;
				}

				if (m_workQueue.size() == 0) 
					continue;
				job = m_workQueue.front();
				m_workQueue.pop();
			}

			job->execute();
			delete job;
		}
	}
};