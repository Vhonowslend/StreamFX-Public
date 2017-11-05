# OBS Stream Effects
Bring your stream to life with more modern effects! Stream Effects adds several much needed features to OBS Studio, such as real time Blur and 3D Transform. Now you can blur out sources you think may be questionable, add sick 3D effects or recreate the Heroes of the Storm displacement overlay with the Displacement filter.

## Filters
- 3D Transform
- Blur
- Displacment

## Transitions

# Building the Project
Building the project is fairly easy and relies on CMake to ease the process.

## Prerequisites
- Visual Studio 2013, 2015 or 2017
- CMake
- A built version of obs-studio

## Configuration
The project supports both command line and GUI versions of cmake, these are the parameters than can be configured:

- PATH_OBSStudio: Path to the root of a completely built obs-studio version.
- INSTALL_DIR: Where the INSTALL target installs files to.
- PACKAGE_PREFIX: The prefix for the PACKAGE_* generated files.
- PACKAGE_SUFFIX: The suffix for the PACKAGE_* generated files, defaults to the version number.

## Building
Building is simply handled by CMake. If you generated an IDE project, just use the IDE provided options for building.

## Installing
You can directly install files into an existing obs-studio installation or put them in a separate directory for later packaging and releasing. Simply point INSTALL_DIR to the directory where you want the files to be installed to and then run the INSTALL target (which will also build the project if it hasn't been built yet).

## Packaging
Packaging has two options, 7zip and zip. Simply run the targets PACKAGE_ZIP and/or PACKAGE_7ZIP and you will have a build that you can distribute. 

# Continuous Integration
To ease detection of bad commits and PRs, CI support has been added to the project. Currently the following CI providers are supported:

- AppVeyor (for Windows on any support Visual Studio version)
