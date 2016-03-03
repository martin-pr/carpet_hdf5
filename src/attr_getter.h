#pragma once

#include <vector>
#include <array>

#include <H5Location.h>

template<typename T>
struct attr_getter {
	static T get(const H5::H5Location& location, const std::string& attr); // generic not implemented
};

template<typename ELEM>
struct attr_getter<std::vector<ELEM>> {
	static std::vector<ELEM> get(const H5::H5Location& location, const std::string& attrName);
};

template<typename ELEM, size_t DIM>
struct attr_getter<std::array<ELEM, DIM>> {
	static std::array<ELEM, DIM> get(const H5::H5Location& location, const std::string& attrName);
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

template<typename ELEM, size_t DIM>
std::array<ELEM, DIM> attr_getter<std::array<ELEM, DIM>>::get(const H5::H5Location& location, const std::string& attrName) {
	H5::Attribute attr = location.openAttribute(attrName);

	const unsigned count = attr.getInMemDataSize() / sizeof(ELEM);
	const unsigned recordedCount = attr.getSpace().getSimpleExtentNpoints();
	if(count != DIM || recordedCount != DIM) {
		std::stringstream err;
		err << "Error fetching attribute " << attrName << " - size based on requested datatype is " << count << ", recorded size is " << recordedCount << " and expected size is " << DIM;
		throw std::runtime_error(err.str());
	}

	std::array<ELEM, DIM> result;
	H5::DataType type = attr.getDataType();
	attr.read(type, (void*)(&result));

	return result;
}
