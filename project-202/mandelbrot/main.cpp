#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <list>
#include <stdlib.h>
#include <algorithm>

#include "Farm.h"
#include "Task.h"
#include "Compute_Slice.h"
#include "Channel.h"

using std::list;
using std::string;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::microseconds;
using std::chrono::nanoseconds;
using std::cout;
using std::endl;
using std::ofstream;
using std::sort;

typedef std::chrono::steady_clock the_clock;

// this value is modifiable if we zoom further into the set
int MAX_ITERATIONS = 500;

// the dimensions of the image
const int WIDTH = 1920;
const int HEIGHT = 1200;

Channel channel1;

// global we are using to check if the window state has changed, zoomed in or out using input
bool windowState = true;

struct param {
	double left = -2.0;
	double right = 1.0;
	double top = 1.125;
	double bottom = -1.125;
	int height = 0;
};

// very simple function that sorts our times and writes them to csv files
void write_csv(string filename, list<int> arr)
{
	arr.sort();
	ofstream myFile(filename);
	for (auto i : arr) {
		myFile << i << endl;
	}
	myFile.close();
}

void render(sf::RenderWindow* window, sf::Texture* texture, sf::Sprite* sprite)
{
	// render is taking control of the open gl context
	window->setActive(true);

	// read in the image from the channel
	sf::Image imag = channel1.read();

	texture->loadFromImage(imag);
	sprite->setTexture(*texture);

	while (window->isOpen()) { 
		
		// if the user hasn't changed the state of the Mandelbrot keep displaying it
		if (windowState == true)
		{
			window->clear(sf::Color::Black);
			window->draw(*sprite);
			window->display();

			// comment out for interactive
			//window->close();
		}
		// the user has zoomed in or out so we need to update the new Mandelbrot
		else
		{
			sf::Image imag = channel1.read();
			texture->loadFromImage(imag);
			sprite->setTexture(*texture);
			windowState = true;
		}
	}
}

int main(int argc, char* argv[])
{
	// constant
	
	// slice times (microseconds)
	list<int> horizontalSlices;
	// farm times (milliseconds)
	list<int> farmSlices;
	// render times (microseconds)
	list<int> renderSlices;
	// overall times (milliseconds)
	list<int> ovTimes;

	// adjust this
	int worker_threads = 8;
	Farm f(worker_threads);

	//

	// set up SFML
	sf::String title = "Mandelbrot Explorer";
	sf::Texture texture;
	sf::Image img;
	sf::Sprite sprite;
	img.create(WIDTH, HEIGHT, sf::Color::White);

	// adjust this depending on your monitor size
	sf::RenderWindow window(sf::VideoMode(WIDTH - 240, HEIGHT - 240), title, sf::Style::Default);
	window.setFramerateLimit(30);

	// set opengl context 
	window.setActive(false);

	for (int i = 0; i < 1; i++) {

		param mandelbrot;

		// time overall
		the_clock::time_point ov_start = the_clock::now();

		// time the computation
		the_clock::time_point mandl_start = the_clock::now();

		// set up our farm and add our tasks, looping through height

		for (int j = 0; j < HEIGHT; j += 1)
		{
			mandelbrot.height = j;
			f.add_task(new Compute_Slice(
				// constructors 
				img,
				horizontalSlices,
				mandelbrot.left,
				mandelbrot.right,
				mandelbrot.top,
				mandelbrot.bottom,
				WIDTH,
				mandelbrot.height,
				HEIGHT,
				MAX_ITERATIONS)
			);
		}

		// Run the tasks for the Mandelbrot set
		f.run_tasks();

		the_clock::time_point mandl_end = the_clock::now();

		// calculate computation time
		auto _mandl = duration_cast<milliseconds>(mandl_end - mandl_start).count();

		// display to screen
		cout << "Computing the Mandelbrot set took " << _mandl << " ms." << endl;

		farmSlices.push_back(_mandl);

		// write our mandelbrot image to the render task
		channel1.write(img);

		// start our render task

		the_clock::time_point rend_st = the_clock::now();

		sf::Thread rend([&] {
			render(&window, &texture, &sprite);
		});

		rend.launch();

		the_clock::time_point rend_ed = the_clock::now();

		auto _rend = duration_cast<microseconds>(rend_ed - rend_st).count();

		cout << "Render task: " << _rend << endl;

		renderSlices.push_back(_rend);

		// event handling
		while (window.isOpen())
		{
			// window event
			sf::Event event;
			while (window.pollEvent(event))
			{
				// closing the window
				if (event.type == sf::Event::Closed) {
					window.close();
				}
			}

			// if up is pressed we know the user wants to zoom in
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			{
				// zoom in
				// reset our image
				img.create(WIDTH, HEIGHT, sf::Color::White);

				// change to zoomed in coordinates

				mandelbrot.left = -0.751085;
				mandelbrot.right = -0.734975;
				mandelbrot.top = 0.118378;
				mandelbrot.bottom = 0.134488;
				mandelbrot.height = 0;

				// tune our max_iterations since we're zooming further into the set

				MAX_ITERATIONS = 800;

				// set up our farm and add our tasks, looping through height

				for (int j = 0; j < HEIGHT; j += 1)
				{
					mandelbrot.height = j;
					f.add_task(new Compute_Slice(
						img,
						horizontalSlices,
						mandelbrot.left,
						mandelbrot.right,
						mandelbrot.top,
						mandelbrot.bottom,
						WIDTH,
						mandelbrot.height,
						HEIGHT,
						MAX_ITERATIONS)
					);
				}

				// Run the tasks for the Mandelbrot set
				f.run_tasks();

				// let render task know we are changing the image
				windowState = false;
				// write the new image to the channel
				channel1.write(img);
			}

			// down is pressed, zoom out
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			{
				// zoom out

				img.create(WIDTH, HEIGHT, sf::Color::White);

				mandelbrot.left = -2.0;
				mandelbrot.right = 1.0;
				mandelbrot.top = 1.125;
				mandelbrot.bottom = -1.125;
				mandelbrot.height = 0;

				MAX_ITERATIONS = 500;

				// set up our farm and add our tasks, looping through height

				for (int j = 0; j < HEIGHT; j += 1)
				{
					mandelbrot.height = j;
					f.add_task(new Compute_Slice(
						img,
						horizontalSlices,
						mandelbrot.left,
						mandelbrot.right,
						mandelbrot.top,
						mandelbrot.bottom,
						WIDTH,
						mandelbrot.height,
						HEIGHT,
						MAX_ITERATIONS)
					);
				}

				f.run_tasks();

				windowState = false;
				channel1.write(img);
			}
		}

		the_clock::time_point ov_end = the_clock::now();

		// calculate overall time
		auto _time = duration_cast<milliseconds>(ov_end - ov_start).count();

		// display to screen
		cout << "Overall: " << _time << " ms." << endl;

		ovTimes.push_back(_time);
	}

	cout << "Horizontal Slices: " << horizontalSlices.size() << endl;
	cout << "Farm: " << farmSlices.size() << endl;
	cout << "Rendering: " << renderSlices.size() << endl;
	cout << "Overall: " << ovTimes.size() << endl;

	// 1 worker thread and 1 render thread

	/*write_csv("horiSl1.csv", horizontalSlices);
	write_csv("farmOv1.csv", farmSlices);
	write_csv("rendOv1.csv", renderSlices);
	write_csv("overal1.csv", ovTimes);*/

	// 2 worker threads and 1 render thread

	/*write_csv("horiSl2.csv", horizontalSlices);
	write_csv("farmOv2.csv", farmSlices);
	write_csv("rendOv2.csv", renderSlices);
	write_csv("overal2.csv", ovTimes);*/

	// 4 worker threads and 1 render thread

	/*write_csv("horiSl4.csv", horizontalSlices);
	write_csv("farmOv4.csv", farmSlices);
	write_csv("rendOv4.csv", renderSlices);
	write_csv("overal4.csv", ovTimes);*/

	// 8 worker threads and 1 render thread

	/*write_csv("horiSl8.csv", horizontalSlices);
	write_csv("farmOv8.csv", farmSlices);
	write_csv("rendOv8.csv", renderSlices);
	write_csv("overal8.csv", ovTimes);*/

	return 0;
}