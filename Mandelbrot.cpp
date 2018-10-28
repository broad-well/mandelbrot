#include "pch.h"
#include "alg.h"
#include "HSL.hpp"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <tuple>
#include <complex>
#include "alg.h"

using complex = std::complex<double>;

static constexpr int kWindowHeight = 500;
static constexpr int kWindowWidth = 500;


static constexpr unsigned int kMaxIterationCount = 50;


static sf::RenderWindow window(sf::VideoMode(kWindowWidth, kWindowHeight), "Mandelbrot");

struct RectBounds {
	complex topLeft, bottomRight;

	RectBounds zoom(const double factor, const complex& center) const
	{
		const auto new_top_left = center + (topLeft - center) / factor,
			new_bottom_right = center + (bottomRight - center) / factor;

		return {
			new_top_left,
			new_bottom_right
		};
	}

	double width() const {
		return bottomRight.real() - topLeft.real();
	}
	double height() const {
		return topLeft.imag() - bottomRight.imag();
	}
	complex center() const {
		return {
		  (topLeft.real() + bottomRight.real()) / 2,
		  (topLeft.imag() + bottomRight.imag()) / 2
		};
	}

	friend RectBounds operator+(const RectBounds& left, const complex& right)
	{
		return {
			left.topLeft + right,
			left.bottomRight + right
		};
	}
};

static const RectBounds kInitialBounds{ {-2, 1}, {1, -1} };

static complex scale_coordinate(
	const RectBounds& bounds,
	const unsigned int x, const unsigned int y) {

	const auto dx = bounds.width(), dy = bounds.height();

	return {
	  static_cast<double>(x) / kWindowWidth * dx + bounds.topLeft.real(),
	  bounds.topLeft.imag() - static_cast<double>(y) / kWindowHeight * dy
	};
}

static void render(const RectBounds& bounds, sf::Image& image, const unsigned int iterationCount) {
	static sf::Uint8 pixel_buffer[kWindowHeight * kWindowWidth * 4];

	#pragma omp target teams distribute parallel for collapse(2)
	for (unsigned int x = 0; x < kWindowWidth; ++x) {
		for (unsigned int y = 0; y < kWindowHeight; ++y) {
			const auto coord = scale_coordinate(bounds, x, y);
			const auto iterations = mandelbrot(coord, iterationCount);
			const auto color = HSL(iterations / iterationCount * 360, 50, iterations == iterationCount ? 0 : 255).TurnToRGB();
			const auto idx_first = 4 * kWindowWidth * y + 4 * x;

			pixel_buffer[idx_first] = color.r;
			pixel_buffer[idx_first + 1] = color.g;
			pixel_buffer[idx_first + 2] = color.b;
			pixel_buffer[idx_first + 3] = color.a;
		}
	}

	image.create(kWindowWidth, kWindowHeight, pixel_buffer);
}

static void redraw(const double zoom_factor, const complex& zoom_center, sf::Image& image, sf::Texture& texture, const unsigned int iterationCount) {
	render(kInitialBounds.zoom(zoom_factor, zoom_center), image, iterationCount);
	texture.update(image);
}

int main() {

	auto zoomFactor = 1.0;
	auto zoomCenter = kInitialBounds.center();
	auto iterationCount = kMaxIterationCount;

	sf::Image image;
	sf::Texture texture;
	sf::Sprite sprite(texture);
	redraw(zoomFactor, zoomCenter, image, texture, iterationCount);

	texture.loadFromImage(image);
	texture.setSmooth(true);
	sprite.setTexture(texture, true);
	//std::cout << sprite.getPosition() << std::endl;

   // run the program as long as the window is open
	while (window.isOpen())
	{
		// check all the window's events that were triggered since the last iteration of the loop
		sf::Event event;
		while (window.pollEvent(event))
		{
			// "close requested" event: we close the window
			if (event.type == sf::Event::Closed)
				window.close();
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
			zoomCenter = { zoomCenter.real() - 1 / zoomFactor / 1.3, zoomCenter.imag() };
			redraw(zoomFactor, zoomCenter, image, texture, iterationCount);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
			zoomCenter = { zoomCenter.real() + 1 / zoomFactor / 1.3, zoomCenter.imag() };
			redraw(zoomFactor, zoomCenter, image, texture, iterationCount);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
			zoomCenter = { zoomCenter.real(), zoomCenter.imag() + 1 / zoomFactor };
			redraw(zoomFactor, zoomCenter, image, texture, iterationCount);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
			zoomCenter = { zoomCenter.real(), zoomCenter.imag() - 1 / zoomFactor };
			redraw(zoomFactor, zoomCenter, image, texture, iterationCount);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::I)) {
			zoomFactor *= 1.1;
			iterationCount += 2;
			redraw(zoomFactor, zoomCenter, image, texture, iterationCount);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::O)) {
			zoomFactor /= 1.1;
			iterationCount -= 2;
			redraw(zoomFactor, zoomCenter, image, texture, iterationCount);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Equal)) {
			iterationCount += 4;
			redraw(zoomFactor, zoomCenter, image, texture, iterationCount);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace)) {
			iterationCount -= 4;
			redraw(zoomFactor, zoomCenter, image, texture, iterationCount);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
			zoomFactor = 1.0;
			zoomCenter = kInitialBounds.center();
			iterationCount = kMaxIterationCount;
		}


		window.draw(sprite);
		window.display();
	}
}
