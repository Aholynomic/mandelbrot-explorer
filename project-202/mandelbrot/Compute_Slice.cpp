#include "Compute_Slice.h"

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>

#include <complex>
#include <chrono>
#include <iostream>
#include <mutex>
#include <list>

using std::list;
using std::mutex;
using std::cout;
using std::endl;
using std::complex;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::microseconds;

typedef std::chrono::steady_clock the_clock;

Compute_Slice::Compute_Slice(
	sf::Image &img,
	list<int> &_times,
	double _left,
	double _right,
	double _top,
	double _bottom,
	int _width,
	int _height,
	int _totalHeight,
	int _max_iter) 
: 
	_image(img),
	_listTimes(_times),
	left(_left),
	right(_right),
	top(_top),
	bottom(_bottom),
	height(_height),
	TOTALHEIGHT(_totalHeight),
	WIDTH(_width),
	MAX_ITERATIONS(_max_iter)
{
}

Compute_Slice::~Compute_Slice()
{
}

mutex times_mutex;

// Render the Mandelbrot set into an sf::Image.
// Works out the corresponding coordinates in the Mandelbrot set for each pixel. 
// Then it performs the standard algorithm for testing whether a point is in the Mandelbrot Set
// Each line is split into tasks, which is why I got rid of y coordinate
void Compute_Slice::run()
{
	the_clock::time_point slice_start = the_clock::now();

	// loop through width as we are calculating horizontal slices
	for (int x = 0; x < WIDTH; x++)
	{
		// Work out the point in the complex plane that
		// corresponds to this pixel in the output image.
		complex<double> c(left + (x * (right - left) / WIDTH),
			top + (height * (bottom - top) / TOTALHEIGHT));

		// Start off z at (0, 0).
		complex<double> z(0.0, 0.0);

		// Iterate z = z^2 + c until z moves more than 2 units
		// away from (0, 0), or we've iterated too many times.
		int iterations = 0;
		while (abs(z) < 2.0 && iterations < MAX_ITERATIONS) // 2.0
		{
			z = (z * z) + c;

			++iterations;
		}

		// these are not in the set

		if (iterations < 25) {
			// height acts as our y coordinate
			_image.setPixel(x, height, sf::Color::Black);
		}

		// this is barely touching

		else if (iterations < 50) {
			_image.setPixel(x, height, sf::Color::Cyan);
		}

		// this is in the set
		else if (iterations == MAX_ITERATIONS) {
			// this point is in the mandelbrot set
			_image.setPixel(x, height, sf::Color::Blue);
		}
	}
	the_clock::time_point slice_end = the_clock::now();

	// have a mutex that protects our vector storing our times
	times_mutex.lock();
	auto _time = duration_cast<microseconds>(slice_end - slice_start).count();
	_listTimes.push_back(_time);
	times_mutex.unlock();
}