//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_GFX_POLYNOMIAL
#define HEADER_GFX_POLYNOMIAL

/** @file gfx/polynomial.hpp
 *
 *  Solutions to polynomials, used by bezier curve algorithms
 */
 
// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

// ----------------------------------------------------------------------------- : Solving

/// Solve a linear equation a x + b = 0
/** Returns the number of real roots, and the roots themselfs in the output parameter.
 */ 
UInt solve_linear(double a, double b, double* root);

/// Solve a quadratic equation a x^2 + b x + c == 0
/** Returns the number of real roots, and the roots themselfs in the output parameter.
 */
UInt solve_quadratic(double a, double b, double c, double* roots);

// Solve a cubic equation a x^3 + b x^2 + c x + d == 0
/** Returns the number of real roots, and the roots themselfs in the output parameter.
 */
UInt solve_cubic(double a, double b, double c, double d, double* roots);

// Solve a cubic equation x^3 + a x^2 + b x + c == 0
/** Returns the number of real roots, and the roots themselfs in the output parameter.
 *  Based on http://en.wikipedia.org/wiki/Cubic_equation
 */
UInt solve_cubic(double a, double b, double c, double* roots);

// ----------------------------------------------------------------------------- : EOF
#endif
