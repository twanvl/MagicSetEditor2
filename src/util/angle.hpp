//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2010 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_UTIL_ANGLE
#define HEADER_UTIL_ANGLE

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>

// ----------------------------------------------------------------------------- : Degrees & radians

typedef double Degrees;
typedef double Radians;

/// Convert radians to degrees
inline Degrees rad_to_deg(Radians rad) { return  rad * (180.0 / M_PI); }
/// Convert degrees to radians
inline Radians deg_to_rad(Degrees deg) { return  deg * (M_PI / 180.0); }

// ----------------------------------------------------------------------------- : Angle constants

const Radians rad0   = 0;
const Radians rad45  = 0.25*M_PI;
const Radians rad90  = 0.5*M_PI;
const Radians rad180 = M_PI;
const Radians rad270 = 1.5*M_PI;
const Radians rad360 = 2.0*M_PI;

/// Are two floating point numbers equal up to a small epsilon?
inline bool almost_equal(double x, double y) {
	return fabs(x-y) < 1e-10;
}
inline bool is_rad0(double x) {
	return almost_equal(x,0) || almost_equal(x,rad360);
}
inline bool is_rad90(double x) {
	return almost_equal(x,rad90);
}
inline bool is_rad180(double x) {
	return almost_equal(x,rad180);
}
inline bool is_rad270(double x) {
	return almost_equal(x,rad270);
}

// ----------------------------------------------------------------------------- : Angle functions

// mod as it should be: answer in range [0..m)
inline double sane_fmod(double x, double m) {
	double ans = fmod(x,m);
	if (ans < 0) return ans + m;
	else         return ans;
}

// constrain an angle to [0..2pi)
inline Radians constrain_radians(Radians angle) {
	return sane_fmod(angle, 2*M_PI);
}

/// Is an angle a multiple of 90 degrees?
inline bool is_straight(Radians angle) {
	return almost_equal(sane_fmod(angle+rad45,rad90), rad45);
}

/// Is an angle sideways (i.e. closer to 90 or 270 degrees than to 0 or 180 degrees)?
inline bool is_sideways(Radians angle) {
	double a = sane_fmod(angle,M_PI);
	return (a > 0.25*M_PI && a < 0.75*M_PI);
}


// ----------------------------------------------------------------------------- : EOF
#endif
