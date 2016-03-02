#pragma once

#include <H5Location.h>

template<typename T>
struct attr_getter {
	static T get(const H5::H5Location& location, const std::string& attr); // generic not implemented
};

template<typename ELEM>
struct attr_getter<std::vector<ELEM>> {
	static std::vector<ELEM> get(const H5::H5Location& location, const std::string& attrName);
};

////////////

template<typename ELEM>
std::vector<ELEM> attr_getter<std::vector<ELEM>>::get(const H5::H5Location& location, const std::string& attrName) {
	H5::Attribute attr = location.openAttribute(attrName);

	const unsigned count = attr.getInMemDataSize() / sizeof(ELEM);
	const unsigned recordedCount = attr.getSpace().getSimpleExtentNpoints();
	if(count != recordedCount) {
		std::stringstream err;
		err << "Error fetching attribute " << attrName << " - size based on requested datatype is " << count << ", but recorded size is " << recordedCount;
		throw std::runtime_error(err.str());
	}

	ELEM values[count];
	H5::DataType type = attr.getDataType();
	attr.read(type, (void*)values);

	return std::vector<ELEM>(values, values+count);
}
