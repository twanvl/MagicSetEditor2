//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2009 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gfx/polynomial.hpp>
#include <complex>

// ----------------------------------------------------------------------------- : Solving

UInt solve_linear(double a, double b, double* root) {
	if (a == 0) {
		if (b == 0) {
			root[0] = 0;
			return 1;
		} else {
			return 0;
		}
	} else {
		root[0] = -b / a;
		return 1;
	}
}

UInt solve_quadratic(double a, double b, double c, double* roots) {
	if (a == 0) {
		return solve_linear(b, c, roots);
	} else {
		double d = b*b - 4*a*c;
		if (d < 0)  return 0;
		roots[0] = (-b - sqrt(d)) / (2*a);
		roots[1] = (-b + sqrt(d)) / (2*a);
		return 2;
	}
}

UInt solve_cubic(double a, double b, double c, double d, double* roots) {
	if (a == 0) {
		return solve_quadratic(b, c, d, roots);
	} else {
		return solve_cubic(b/a, c/a, d/a, roots);
	}
}

// cubic root
template <typename T>
inline T curt(T x) { return pow(x, 1.0 / 3); }

UInt solve_cubic(double a, double b, double c, double* roots) {
	double p = b - a*a / 3;
	double q = c + (2 * a*a*a - 9 * a * b) / 27;
	if (p == 0 && q == 0) {
		roots[0] = -a / 3;
		return 1;
	}
	complex<double> u;
	if (q > 0) {
		u = curt(q/2 + sqrt(complex<double>(q*q / 4  +  p*p*p / 27)));
	} else {
		u = curt(q/2 - sqrt(complex<double>(q*q / 4  +  p*p*p / 27)));
	}
	// now for the complex part
	//              rot1(1,    0)
	complex<double> rot2(-0.5, sqrt(3.0) / 2);
	complex<double> rot3(-0.5, -sqrt(3.0) / 2);
	complex<double> x1 = p / (3.0 * u)         -  u        -  a / 3.0;
	complex<double> x2 = p / (3.0 * u * rot2)  -  u * rot2 -  a / 3.0;
	complex<double> x3 = p / (3.0 * u * rot3)  -  u * rot3 -  a / 3.0;
	// check if the solutions are real
	UInt count = 0;
	if (abs(x1.imag()) < 0.00001) {
		roots[count] = x1.real();
		count += 1;
	}
	if (abs(x2.imag()) < 0.00001) {
		roots[count] = x2.real();
		count += 1;
	}
	if (abs(x3.imag()) < 0.00001) {
		roots[count] = x3.real();
		count += 1;
	}
	return count;
}
