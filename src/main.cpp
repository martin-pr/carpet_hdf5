#include <iostream>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/regex.hpp>

#include <H5Cpp.h>
#include <H5Location.h>
#include <H5File.h>

#include <openvdb.h>
#include <math/Transform.h>

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

	template<typename T>
	struct AttrGetter {
		static T get(const H5::H5Location& location, const std::string& attr); // generic not implemented
	};

	template<typename ELEM>
	struct AttrGetter<std::vector<ELEM>> {
		static std::vector<ELEM> get(const H5::H5Location& location, const std::string& attrName) {
			H5::Attribute attr = location.openAttribute(attrName);

			const unsigned count = attr.getInMemDataSize() / sizeof(ELEM);
			const unsigned recordedCount = attr.getSpace().getSimpleExtentNpoints();
			if(count != recordedCount) {
				std::stringstream err;
				err << "Error fetching attribute " << attrName << " - size based on requested datatype is " << count << ", but recorded size is " << recordedCount;
				throw std::runtime_error(err.str());
			}

			ELEM values[count];
			DataType type = attr.getDataType();
			attr.read(type, (void*)values);

			return std::vector<ELEM>(values, values+count);
		}
	};

	void printDataset(const H5::DataSet& dataset, const std::string& prefix = "") {
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

	void printContent(const H5::CommonFG& group, std::string prefix, bool printDetail, const boost::regex& datasetRegex) {
		for(hsize_t i = 0; i < group.getNumObjs(); ++i) {
			std::string type;
			auto typeId = group.getObjTypeByIdx(i, type);

			if(boost::regex_match(group.getObjnameByIdx(i), datasetRegex)) {
				cout << prefix << group.getObjnameByIdx(i) << "  ->  " << type << endl;

				if(typeId == H5G_GROUP)
					printContent(group.openGroup(group.getObjnameByIdx(i)), prefix + SPACING, printDetail, datasetRegex);
				else if(printDetail) {
					if(typeId == H5G_DATASET)
						printDataset(group.openDataSet(group.getObjnameByIdx(i)), prefix + SPACING);
				}
			}
		}
	}

	///////////////

	// template<typename T>
	// void printDatasetValues(const H5::DataSet& ds) {
	// 	const unsigned count = ds.getInMemDataSize() / sizeof(T);
	// 	T values[count];
	// 	DataType type = ds.getDataType();
	// 	ds.read((void*)values, type);

	// 	for(unsigned a=0;a<count;++a)
	// 		cout << values[a] << "  ";
	// }

	// void printDatasetContent(const H5File& file, const std::string datasetName) {
	// 	H5::DataSet ds = file.openDataSet(datasetName);
	// 	printDataset(ds);

	// 	cout << endl;

	// 	if(ds.getDataType().getClass() == H5T_INTEGER)
	// 		printDatasetValues<int>(ds);
	// 	else if(ds.getDataType().getClass() == H5T_FLOAT)
	// 		printDatasetValues<double>(ds);
	// 	else
	// 		cout << "(print not implemented)";
	// 	cout << endl;
	// }

	openvdb::FloatGrid::Ptr writevdb(const H5File& file, const std::string datasetName, openvdb::io::File& out, bool normalize, float offset) {
		H5::DataSet ds = file.openDataSet(datasetName);
		if(ds.getDataType().getClass() != H5T_FLOAT)
			throw std::runtime_error("OpenVDB output only supports float grids");

		H5::DataSpace space = ds.getSpace();
		if(space.getSimpleExtentNdims() != 3)
			throw std::runtime_error("OpenVDB output only supports 3D grids");

		hsize_t dims[3];
		space.getSimpleExtentDims(dims);

		const unsigned count = ds.getInMemDataSize() / sizeof(float);
		const unsigned computedSize = (dims[0]) * (dims[1]) * (dims[2]);

		if(count == 0)
			throw std::runtime_error("Empty grid cannot be used!");

		if(count != computedSize) {
			std::stringstream err;
			err << "Something wrong with the data count - getInMemDataSize gives " << count << "elements, while the computed size is " << computedSize << " elements.";
			throw std::runtime_error(err.str());
		}

		float values[count];
		DataType type = ds.getDataType();
		ds.read((void*)values, type);

		// normalize the values
		if(normalize) {
			float min = values[0];
			float max = values[0];
			for(unsigned a=0;a<count;++a) {
				min = std::min(values[a], min);
				max = std::max(values[a], max);
			}

			for(unsigned a=0;a<count;++a)
				values[a] = (values[a] - min) / (max - min) + offset;
		}
		else if(offset != 0.0f)
			for(unsigned a=0;a<count;++a)
				values[a] += offset;

		// create the grid
		openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create();
		grid->setName(datasetName);
		{
			// houdini hack for grid naming - Houdini has problems with many special symbols in grid name
			std::string gridName = datasetName;
			boost::replace_all(gridName, " ", "_");
			boost::replace_all(gridName, ":", "_");
			boost::replace_all(gridName, "=", "_");
			grid->setName(gridName);
		}

		// transformed by the origin and delta values
		//   based on http://www.carpetcode.org/hg/carpet/index.cgi/rev/245224d7a5ec
		//   and http://www.openvdb.org/documentation/doxygen/classopenvdb_1_1v3__1__0_1_1math_1_1Transform.html#details
		const std::vector<double> origin = AttrGetter<std::vector<double>>::get(ds, "origin");
		const std::vector<double> delta = AttrGetter<std::vector<double>>::get(ds, "delta");

		const openvdb::math::Transform::Ptr tr = openvdb::math::Transform::createLinearTransform(openvdb::Mat4R(
			(float)delta[0], 0.0f, 0.0f, 0.0f,
			0.0f, (float)delta[1], 0.0f, 0.0f,
			0.0f, 0.0f, (float)delta[2], 0.0f,
			(float)origin[0], (float)origin[1], (float)origin[2], 1.0f
		));
		grid->setTransform(tr);

		// write the values to the grid
		{
			openvdb::FloatGrid::Accessor accessor = grid->getAccessor();
			float* ptr = values;
			for(hsize_t x = 0; x < dims[0]; ++x)
				for(hsize_t y = 0; y < dims[1]; ++y)
					for(hsize_t z = 0; z < dims[2]; ++z) {
						openvdb::Coord xyz(z, y, x);
						accessor.setValue(xyz, *(ptr++));
					}
		}

		return grid;
	}
}

int main(int argc, char* argv[]) {
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("input", po::value<std::vector<std::string>>()->multitoken(), "input hdf5 file(s) - can process arbitrary number of files at the same time")
		("detail", "print out details about each grid, not just names")
		("writevdb", po::value<std::string>(), "write all datasets into an openvdb file")
		("dataset_regex", po::value<std::string>()->default_value(std::string(".*")), "read only datasets matching a regex (optional)")
		("normalize", "normalize the output to be between 0 and 1")
		("offset", po::value<float>()->default_value(0.0f), "offset the data by a given amount")
	;

	// process the options
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if(vm.count("help") || (!vm.count("input"))) {
		cout << desc << "\n";
		return 1;
	}

	// get the regex to match processed things
	const boost::regex datasetRegex(vm["dataset_regex"].as<std::string>());

	if(vm.count("writevdb")) {
		openvdb::initialize();

		openvdb::io::File out(vm["writevdb"].as<std::string>());
		openvdb::GridPtrVec grids;

		for(auto& filename : vm["input"].as<std::vector<std::string>>()) {
			H5File file(filename, H5F_ACC_RDONLY );

			for(hsize_t i = 0; i < file.getNumObjs(); ++i) {
				std::string type;
				auto typeId = file.getObjTypeByIdx(i, type);

				if((typeId == H5G_DATASET) && (boost::regex_match(file.getObjnameByIdx(i), datasetRegex))) {
					cout << "Writing grid " << file.getObjnameByIdx(i) << " to " << out.filename() << "..." << endl;
					auto tmp = writevdb(file, file.getObjnameByIdx(i), out, vm.count("normalize"), vm["offset"].as<float>());
					grids.push_back(tmp);
					cout << "done." << endl;
				}
			}
		}

		out.write(grids);
		out.close();
	}
	else {
		for(auto& filename : vm["input"].as<std::vector<std::string>>()) {
			H5File file(filename, H5F_ACC_RDONLY);

			printContent(file, "", vm.count("detail"), datasetRegex);
		}
	}

	return 0;
}

