#include "grid_collection.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>

using std::cout;
using std::endl;

grid_collection::grid_collection(const std::string& name, const std::array<double, 3>& origin, const std::array<double, 3>& delta, const std::array<int, 3>& iorigin) :
m_origin(origin), m_delta(delta), m_iorigin(iorigin) {
	boost::algorithm::split(m_nameBits, name, boost::algorithm::is_any_of("\t "), boost::token_compress_on);
}

std::array<double, 3> grid_collection::scale() const {
	return m_delta;
}

std::array<double, 3> grid_collection::origin() const {
	return m_origin;
}

std::array<int, 3> grid_collection::iorigin() const {
	return m_iorigin;
}

bool grid_collection::isConsistentWith(const grid_collection& gc) const {
	double diff = 0.0f;
	for(unsigned a=0;a<3;++a) {
		const double d1elem = fmod(m_origin[a], m_delta[a]) / m_delta[a];
		const double d2elem = fmod(gc.m_origin[a], gc.m_delta[a]) / gc.m_delta[a];

		diff += abs(d1elem - d2elem);
	}

	return (m_delta == gc.m_delta) && (diff < 1e-5) /*&& m_ioffsetdenom == gc.m_ioffsetdenom*/;
}

std::string grid_collection::name() const {
	return boost::algorithm::join(m_nameBits, " ");
}

grid_collection grid_collection::operator +(const grid_collection& gc) const {
	assert(isConsistentWith(gc));

	for(unsigned a=0;a<3;++a) {
		const double origStart = abs(m_origin[a] - (double)m_iorigin[a]*m_delta[a]);
		const double newStart = abs(gc.m_origin[a] - (double)gc.m_iorigin[a]*gc.m_delta[a]);

		if(origStart != newStart)
			throw std::runtime_error("starts of coordinate systems don't match");
		// cout << origStart << "  " << newStart << endl;
	}

	// result is a copy of *this
	grid_collection result(*this);
	// but its origins are min of both arguments
	for(unsigned a=0;a<3;++a) {
		result.m_origin[a] = std::min(result.m_origin[a], gc.m_origin[a]);
		result.m_iorigin[a] = std::min(result.m_iorigin[a], gc.m_iorigin[a]);
	}
	// and its name is assembled piece-wise
	result.m_nameBits.clear();
	for(unsigned a=0; a<std::min(m_nameBits.size(), gc.m_nameBits.size()); ++a)
		if(m_nameBits[a] == gc.m_nameBits[a])
			result.m_nameBits.push_back(m_nameBits[a]);
		else
			result.m_nameBits.push_back("");

	return result;
}

std::ostream& operator << (std::ostream& out, const grid_collection& gc) {
	out << gc.name() << ":" << endl;
	out << "  origin = " << gc.m_origin[0] << " " << gc.m_origin[1] << " " << gc.m_origin[2] << endl;
	out << "  delta = " << gc.m_delta[0] << " " << gc.m_delta[1] << " " << gc.m_delta[2] << endl;
	out << "  iorigin = " << gc.m_iorigin[0] << " " << gc.m_iorigin[1] << " " << gc.m_iorigin[2] << endl;

	return out;
}
