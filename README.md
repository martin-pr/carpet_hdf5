# Carpet HDF5 to OpenVDB convertor

This simple program converts hierarchical grids in Carpet HDF5 format (used by the physics community to represent multiset grid data) to a more common OpenVDB format usable in VFX tools.

It is a simple command line tool, designed to compile in any modern Linux distro with no special packages needed (requires HDF5-cpp and OpenVDB libs, both in dev version).

## The Carpet HDF5 file format

The carpet format's grids are spread out into multiple HDF5 files, each containing a small patch to construct a set of large grids. A single frame and subdivision level of the simulation can potentially be spread-out between a large number of files.

To assemble a file, it is necessary to identify the patches required from the files by their metadata. For most practical purposes, these metadata are included in patch names, so a regular expression on all patch names is robust-enough mechanism for this job. By processing all the patch files at once, the final VDB file contains a set of full grids, one for each resolution.

## Getting info about grids

The simplest usage is to just query the grids present in a list of files. To do that, just run the application only with --input parameter:
```
./carpet_hdf5 --input temperature*
```

This will print out a potentially large number of lines, each containing a single dataset in a single file. This can then be used to assemble a regular expression to filter for a single frame, e.g.:
```
./carpet_hdf5 --input temperature* --dataset_regex '.*it=917504.*tl=0.*m=0.*'
```

This should print out a significantly reduced list of grids.

To print out a detailed description of a set of patches (including all their metadata, sampling deltas, dimensions and others), use --detail option (usually useful only for debugging).

## Extracting an OpenVDB file

To extract an .vdb file, just run the program with --writevdb option.

The resulting VDB will contain one grid for each resolution level in the source data. All patches detected as a single level will be unified, and overlaps discarded.

## References:

Carpet internals: http://www.carpetcode.org/doc/internals.pdf

HDF5 C++ docs: https://www.hdfgroup.org/HDF5/doc/cpplus_RM
