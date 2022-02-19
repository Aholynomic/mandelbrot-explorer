#pragma once
#include <mutex>
#include <list>
#include <condition_variable>
#include <SFML/Graphics.hpp>

using std::unique_lock;
using std::mutex;
using std::list;
using std::condition_variable;

class Channel {
public:
	// have a mutex that locks whenever read or write of the result
	mutex arr_mutex;
	sf::Image arr;
	condition_variable cv;

	// flag variable that indicates whether the result has been written yet
	bool result_ready = false;

	void write(sf::Image imgArray)
	{
		unique_lock<mutex> lck(arr_mutex);

		// write the result
		arr = imgArray;
		
		// set the flag
		result_ready = true;

		// wake up all threads waiting on the data
		cv.notify_all();
	}

	sf::Image read() {
		unique_lock<mutex> lck(arr_mutex);

		cv.wait(lck, [&]() {
			return result_ready;
			}
		);

		// we have finished communicating, set the flag back to false
		// if we are further communicating
		result_ready = false;
		return arr;
	}
};