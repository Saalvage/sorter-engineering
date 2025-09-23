cmake_minimum_required(VERSION 3.19)

project(AlgEng-Uebung)

find_package(Threads REQUIRED)

add_executable(Sorter framework/runner.cpp
  src/container.cpp src/container.hpp
  src/sorter.cpp src/sorter.hpp)
target_link_libraries(Sorter PUBLIC Threads::Threads)

target_compile_features(Sorter PRIVATE cxx_std_20)

find_package(
  Python
  COMPONENTS Interpreter
  REQUIRED)

set(result_file ${CMAKE_CURRENT_LIST_DIR}/result.txt)

add_custom_command(
  OUTPUT ${result_file}
  COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/eval.py run
          --build-dir ${CMAKE_CURRENT_BINARY_DIR} ${result_file}
  COMMENT "Running experiment"
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  DEPENDS Sorter eval.py)

add_custom_target(run DEPENDS ${result_file})

set(plot_file ${CMAKE_CURRENT_LIST_DIR}/plot.pdf)

add_custom_command(
  OUTPUT ${plot_file}
  COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/eval.py plot
          ${result_file}
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  COMMENT "Plotting results"
  DEPENDS ${result_file} eval.py)

add_custom_target(plot DEPENDS ${plot_file})

set(submission_file ${CMAKE_CURRENT_LIST_DIR}/submission.zip)

add_custom_command(
  OUTPUT ${submission_file}
  COMMAND
    ${CMAKE_COMMAND} -E tar cv ${submission_file} --format=zip
    ${CMAKE_CURRENT_LIST_DIR}/src/container.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/container.hpp
    ${CMAKE_CURRENT_LIST_DIR}/src/sorter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/sorter.hpp
    ${plot_file}
    ${CMAKE_CURRENT_LIST_DIR}/description.md
  COMMENT "Creating submission"
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  DEPENDS ${plot_file} ${CMAKE_CURRENT_LIST_DIR}/description.md)


add_custom_target(submission DEPENDS ${submission_file})

set(dist_file_list
  CMakeLists.txt
  CMakePresets.json
  description.md
  eval.py
  framework/runner.cpp
  Pipfile
  README.md
  src/container.cpp
  src/container.hpp
  src/sorter.cpp
  src/sorter.hpp
)

set(framework_dist_file "ae-sorting.zip")

add_custom_command(
  OUTPUT ${framework_dist_file}
  COMMAND
  ${CMAKE_COMMAND} -E tar cv ${framework_dist_file} --format=zip
  ${dist_file_list}
  COMMENT "Creating framework dist archive"
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

add_custom_target(framework DEPENDS ${framework_dist_file})
