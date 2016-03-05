#include <iostream>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

// #define H5_NO_NAMESPACE
// #define H5_NO_STD
// #define H5std_string ::string

#include <hdf5/serial/H5Cpp.h>
#include <hdf5/serial/H5Location.h>
#include <hdf5/serial/H5File.h>

using namespace H5;

namespace po = boost::program_options;

using std::cout;
using std::endl;

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

	cout << "hello world" << endl;

	H5File file(vm["input"].as<std::string>().c_str(), H5F_ACC_RDONLY );

	return 0;
}
