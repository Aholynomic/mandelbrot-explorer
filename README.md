# mandelbrot-explorer

The application creates an interactive Mandelbrot that allows you to zoom into two pre-specified coordinates. It uses the SFML library to render the Mandelbrot image, computed using a CPU farming pattern, to be viewed on the display. Once the Mandelbrot has been fully computed, it will then signal the render task using a channel to display the image to the user. This was created for a module called CMP202.

Grade: A

## Contents
*  project202 - config and source files for Mandelbrot
*  Presentation

## Controls

Using the UP key you can zoom in to the pre-specified part of the Mandelbrot set.
Using DOWN allows you to zoom out to view the full-set.

## Installation

The application uses SFML to render the image to the display. This is the only third-party library that I installed. The source comes from here:

https://www.sfml-dev.org/download/sfml/2.5.1

In this, I installed the graphics, system and windows dependencies. In my case I used VS Studio 2019 and placed the download folder into the root directory. The SFML instructions can be followed to achieve the same outcome within VS Studio 2019. 
