#include "pch.h"
#include "alg.h"
#include <algorithm>
#include <cmath>

using namespace std;

inline complex<double> squared(const complex<double>& num) {
	const auto re = real(num), im = imag(num);

  return {
	re * re - im * im, 2 * re*im
  };
}

// Escape time algorithm
double mandelbrot(const complex<double>& c, const unsigned int iteration_count) {
  auto zn = c;
  unsigned int n = 1;

  for (; n < iteration_count && abs(zn) < 2.0; ++n) {
    zn = zn*zn + c;
  }

  return n == iteration_count ? n : n + 1 - log(log(abs(zn)))/log(2);
}