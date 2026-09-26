cmake_minimum_required(VERSION 3.20)
project(gxana VERSION 1.0.0 LANGUAGES CXX)

# ROOT >= 6.20 (RooJohnson). ROOT_USE_FILE brings ROOT's compiler flags,
# including its C++ standard, and the ROOT_GENERATE_DICTIONARY macro.
find_package(ROOT 6.20 REQUIRED COMPONENTS Core RIO Hist Graf Gpad Tree RooFit RooFitCore ROOTDataFrame)
include(${ROOT_USE_FILE})
find_program(ROOT_EXE root HINTS ${ROOT_BINDIR} REQUIRED)

# ROOT 6.24's ROOT_GENERATE_DICTIONARY macro installs the rootmap/pcm via
# CMAKE_INSTALL_LIBDIR, which GNUInstallDirs defines; include it so the
# dictionary target configures on both 6.24 (CI/containers) and 6.40 (local).
include(GNUInstallDirs)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

enable_testing()
add_subdirectory(packages/common)
add_subdirectory(packages/xsection)
