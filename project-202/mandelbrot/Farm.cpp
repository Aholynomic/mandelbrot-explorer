#include "Farm.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <mutex>

using std::thread;
using std::mutex;
using std::unique_lock;
using std::cout;
using std::endl;
using std::condition_variable;
using std::this_thread::sleep_for;

mutex queue_mutex;

Farm::Farm(int num_threads)
{
	WORKER_THREADS = num_threads;
}

Farm::~Farm()
{
}

void Farm::add_task(Task* task)
{
	// Use mutex to protect the queue from being overwritten
	queue_mutex.lock();
	queue_tasks.push(task);
	queue_mutex.unlock();
}

void Farm::run_tasks()
{
	// create a vector to hold our threads
	vector<thread*> s;
	for (int i = 0; i < WORKER_THREADS; i++)
	{
		s.push_back(new thread([&] {
			Task* temp;
			while (true)
			{
				queue_mutex.lock();
				// if there are no more tasks
				if (queue_tasks.empty())
				{
					// unlock the mutex
					queue_mutex.unlock();
					return;
				}
				temp = queue_tasks.front();
				queue_tasks.pop();
				queue_mutex.unlock();
				// run the task
				temp->run();
				delete temp;
			}
		}));
	}

	// join the threads
	for (auto a : s) {
		a->join();
		delete a;
	}
}