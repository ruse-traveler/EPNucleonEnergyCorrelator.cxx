# =============================================================================
# @file   CMakeLists.txt
# @author Derek Anderson, building on work by Kolja Kauder
# @date   07.11.2025
# -----------------------------------------------------------------------------
# Top CMakeLists file for EPNucleonEnergyCorrelator package.
# =============================================================================

cmake_minimum_required(VERSION 3.10)
project(EPNucleonEnergyCorrelator VERSION 0.1 LANGUAGES CXX )

# set EPNucleonEnergyCorrelator library as main target
add_library(libepnec SHARED src/EPNucleonEnergyCorrelator.cxx)

# link against ROOT
target_link_libraries(libepnec PUBLIC ROOT::Core ROOT::RIO ROOT::Rint ROOT::Tree ROOT::EG ROOT::Physics)

# set compile option
target_compile_options(libepnec PRIVATE -Wall -Wextra -pedantic -g)  

# build executables
add_executable(epnec src/EPNucleonEnergyCorrelator.cxx)
target_link_libraries(epnec libepnec)
target_include_directories(epnec PRIVATE ${ROOT_INCLUDE_DIRS})

# install library
install(TARGETS epnec DESTINATION bin)
install(TARGETS libepnec
  EXPORT libepnec-export
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

# install headers
install (DIRECTORY ${CMAKE_SOURCE_DIR}/src DESTINATION include/epnec)

# generate config
install(EXPORT libepnec-export
  FILE
  libepnecConfig.cmake
  NAMESPACE
    EPNucleonEnergyCorrelator::
  DESTINATION
  cmake
)

# FIXME DEBUG, REMOVE WHEN READY
message( "Compiled EPNucleonEnergyCorrelator!" )

# end =========================================================================
