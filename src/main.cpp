#include <iostream>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <H5Cpp.h>
#include <H5Location.h>
#include <H5File.h>

using namespace H5;

namespace po = boost::program_options;

using std::cout;
using std::endl;

#define SPACING "\t"

namespace {
	std::string translateClass(H5T_class_t c) {
		switch(c) {
			case H5T_NO_CLASS: return "H5T_NO_CLASS";
			case H5T_INTEGER: return "H5T_INTEGER";
			case H5T_FLOAT: return "H5T_FLOAT";
			case H5T_TIME: return "H5T_TIME";
			case H5T_STRING: return "H5T_STRING";
			case H5T_BITFIELD: return "H5T_BITFIELD";
			case H5T_OPAQUE: return "H5T_OPAQUE";
			case H5T_COMPOUND: return "H5T_COMPOUND";
			case H5T_REFERENCE: return "H5T_REFERENCE";
			case H5T_ENUM: return "H5T_ENUM";
			case H5T_VLEN: return "H5T_VLEN";
			case H5T_ARRAY: return "H5T_ARRAY";
			default: return "unknown";
		}
	}

	template<typename T>
	void printArray(const H5::Attribute& attr) {
		const unsigned count = attr.getInMemDataSize() / sizeof(T);
		T values[count];
		DataType type = attr.getDataType();
		attr.read(type, (void*)values);

		for(unsigned a=0;a<count;++a)
			cout << values[a] << "  ";
	}

	void printString(const H5::Attribute& attr) {
		DataType type = attr.getDataType();
		std::string value;
		attr.read(type, value);

		cout << value;
	}

	void printAttributes(const H5::H5Location& location, const std::string& prefix) {
		for(int aid = 0; aid < location.getNumAttrs(); ++aid) {
			H5::Attribute attr = location.openAttribute(aid);

			std::string value;
			attr.read(attr.getDataType(), value);

			cout << prefix << attr.getName() << " (" << translateClass(attr.getDataType().getClass()) << ", " << attr.getInMemDataSize() << ")  =  ";

			if(attr.getDataType().getClass() == H5T_INTEGER)
				printArray<int>(attr);
			else if(attr.getDataType().getClass() == H5T_FLOAT)
				printArray<double>(attr);
			else if(attr.getDataType().getClass() == H5T_STRING)
				printString(attr);
			else
				cout << "(print not implemented)";
			cout << endl;
		}
	}

	void printDataset(const H5::DataSet& dataset, const std::string& prefix) {
		DataSpace space = dataset.getSpace();
		
		cout << prefix << "type: " << translateClass(dataset.getTypeClass()) << endl;
		cout << prefix << "attrs: " << dataset.getNumAttrs() << endl;
		printAttributes(dataset, prefix + SPACING);
		cout << prefix << "npoints: " << space.getSelectNpoints() << endl;
		cout << prefix << "dims: " << space.getSimpleExtentNdims() << endl;

		hsize_t dims[space.getSimpleExtentNdims()];
		hsize_t maxdims[space.getSimpleExtentNdims()];
		space.getSimpleExtentDims(dims, maxdims);

		space.selectAll();
		hsize_t start[space.getSimpleExtentNdims()];
		hsize_t end[space.getSimpleExtentNdims()];
		space.getSelectBounds(start, end);

		space.selectValid();
		hsize_t startValid[space.getSimpleExtentNdims()];
		hsize_t endValid[space.getSimpleExtentNdims()];
		space.getSelectBounds(startValid, endValid);

		for(int a=0;a<space.getSimpleExtentNdims();++a) {
			cout << prefix << SPACING << "dim #" << a << ":" << endl;
			cout << prefix << SPACING << SPACING << "dim=" << dims[a] << "   maxdim=" << maxdims[a] << endl;
			cout << prefix << SPACING << SPACING << "start=" << start[a] << "   end=" << end[a] << endl;
			cout << prefix << SPACING << SPACING << "start_valid=" << startValid[a] << "   end_valid=" << endValid[a] << endl;
		}
	}

	void printContent(const H5::CommonFG& group, std::string prefix = "") {
		for(hsize_t i = 0; i < group.getNumObjs(); ++i) {
			std::string type;
			auto typeId = group.getObjTypeByIdx(i, type);

			cout << prefix << group.getObjnameByIdx(i) << "  ->  " << type << endl;

			if(typeId == H5G_GROUP)
				printContent(group.openGroup(group.getObjnameByIdx(i)), prefix + SPACING);
			else if(typeId == H5G_DATASET)
				printDataset(group.openDataSet(group.getObjnameByIdx(i)), prefix + SPACING);
		}
	}
}

int main(int argc, char* argv[]) {
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("input", po::value<std::string>(), "input hdf5 file")
	;

	// process the options
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if(vm.count("help") || (!vm.count("input"))) {
		cout << desc << "\n";
		return 1;
	}

	H5File file(vm["input"].as<std::string>().c_str(), H5F_ACC_RDONLY );

	printContent(file);

	return 0;
}
