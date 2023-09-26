cmake_minimum_required(VERSION 3.25)
project(OurDataframe)

set(CMAKE_CXX_STANDARD 14)

# include(CTest)
# enable_testing()
add_library(ODf SHARED ODf.cpp)

option(EIGEN_ADD "whether to add eigen dependency or not" ON)

if(EIGEN_ADD)
  message(STATUS "Adding Eigen...")
  find_package (Eigen3 REQUIRED NO_MODULE)
  target_link_libraries (ODf Eigen3::Eigen)
endif()
