#ifndef FARM_H
#define FARM_H

#include "Task.h"
#include <queue>
#include <thread>

using std::queue;
using std::vector;

class Farm
{
public:
	Farm(int);
	~Farm();

	void add_task(Task* task);
	void run_tasks();
private:
	queue<Task*> queue_tasks;
	int WORKER_THREADS = 8;
};

#endif

