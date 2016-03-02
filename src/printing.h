#pragma once

#include <iostream>

#include <boost/regex.hpp>

#include <H5Cpp.h>
#include <H5Location.h>

template<typename T>
void printArray(const H5::Attribute& attr);

void printString(const H5::Attribute& attr);

void printAttributes(const H5::H5Location& location, const std::string& prefix);

void printDataset(const H5::DataSet& dataset, const std::string& prefix = "");

void printContent(const H5::CommonFG& group, std::string prefix, bool printDetail, const boost::regex& datasetRegex);

////

template<typename T>
void printArray(const H5::Attribute& attr) {
	const unsigned count = attr.getInMemDataSize() / sizeof(T);
	T values[count];
	H5::DataType type = attr.getDataType();
	attr.read(type, (void*)values);

	for(unsigned a=0;a<count;++a)
		std::cout << values[a] << "  ";
}
