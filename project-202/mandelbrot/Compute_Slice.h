#pragma once
#include <mutex>
#include <list>
#include <SFML/Graphics.hpp>

#include "Task.h"

using std::mutex;
using std::vector;
using std::list;

class Compute_Slice : public Task
{
public:
	Compute_Slice(
		sf::Image &img,
		list<int> &_times,
		double _left,
		double _right,
		double _top,
		double _bottom,
		int _width,
		int _height,
		int _totalHeight,
		int _max_iter
	);
	~Compute_Slice();

	void run();

private:
	double left, right, top, bottom;
	const int WIDTH, TOTALHEIGHT;
	int height,	MAX_ITERATIONS;
	
	sf::Image& _image;

	list<int>& _listTimes;
};

