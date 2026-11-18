# kybernesis: steering a system known through evidence. The architecture of the loop from evidence to action declared as
# data and verified (categorically, through kategoria, and by its other constraints); blocks whose contracts methods must
# meet to be admitted; predefined methods, formal and statistical, adapted from the sibling libraries; and the loop run.
#
# Every module is a header-only target kybernesis::<module>. kybernesis uses the targets of kategoria (verification),
# tekmerion (interpretation and construal), tyche (decisions under partial observation), apodeixis (verdicts), morphe (the
# grammar of architectures) and through them peira, nomos, heuresis, kinesis, physis and calx: inside the infrastructure, the
# siblings under foundations/ (each added here when no larger project has added it). Built on its own, kybernesis builds its
# checks (KYBERNESIS_CHECKS); added to a larger project, only when asked.

cmake_minimum_required(VERSION 3.20)
project(kybernesis VERSION 0.1.0 LANGUAGES CXX)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(KYBERNESIS_TOP_LEVEL ON)
else()
  set(KYBERNESIS_TOP_LEVEL OFF)
endif()
option(KYBERNESIS_CHECKS "Build kybernesis's checks" ${KYBERNESIS_TOP_LEVEL})

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

# kybernesis_sibling(<directory> <a target it defines>): the sibling library, added once.
function(kybernesis_sibling directory target)
  if(NOT TARGET ${target})
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../${directory}/CMakeLists.txt)
      add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../${directory} ${CMAKE_BINARY_DIR}/${directory})
    else()
      message(FATAL_ERROR "kybernesis needs ${directory}: build it inside the infrastructure (foundations/${directory}).")
    endif()
  endif()
endfunction()

kybernesis_sibling(calx calx::linalg)
kybernesis_sibling(morphe morphe::formal)
kybernesis_sibling(physis physis::units)
kybernesis_sibling(tyche tyche::probability)
kybernesis_sibling(peira peira::synthesis)
kybernesis_sibling(nomos nomos::measurement)
kybernesis_sibling(apodeixis apodeixis::interpretation)
kybernesis_sibling(heuresis heuresis::evolution)
kybernesis_sibling(kinesis kinesis::prisms)
kybernesis_sibling(tekmerion tekmerion::construal)
kybernesis_sibling(kategoria kategoria::script)
if(KYBERNESIS_CHECKS)
  enable_testing()
endif()

set(KYBERNESIS_DATA ${CMAKE_CURRENT_SOURCE_DIR})

# kybernesis_module(<name> [LIBRARIES ...] [CHECK_LIBRARIES ...]): the target kybernesis::<name> and, with the checks,
# check_kybernesis_<stem> for each tests/check_<stem>.cpp, which knows where the architectures and the schema are kept.
function(kybernesis_module name)
  cmake_parse_arguments(ARG "" "" "LIBRARIES;CHECK_LIBRARIES" ${ARGN})
  add_library(kybernesis_${name} INTERFACE)
  add_library(kybernesis::${name} ALIAS kybernesis_${name})
  target_include_directories(kybernesis_${name} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/${name}/include)
  target_compile_features(kybernesis_${name} INTERFACE cxx_std_20)
  target_link_libraries(kybernesis_${name} INTERFACE ${ARG_LIBRARIES})
  if(KYBERNESIS_CHECKS)
    file(GLOB checks CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${name}/tests/check_*.cpp)
    foreach(source ${checks})
      get_filename_component(stem ${source} NAME_WE)
      string(REGEX REPLACE "^check_" "" stem ${stem})
      calx_add_check(check_kybernesis_${stem} SOURCES ${source} LIBRARIES kybernesis::${name} calx::random ${ARG_CHECK_LIBRARIES})
      target_compile_definitions(check_kybernesis_${stem} PRIVATE KYBERNESIS_DATA="${KYBERNESIS_DATA}")
    endforeach()
  endif()
endfunction()

# The modules in their order (decision 0003).
kybernesis_module(kernel LIBRARIES apodeixis::kernel)
kybernesis_module(architecture LIBRARIES kybernesis::kernel morphe::formal kategoria::script)
kybernesis_module(blocks LIBRARIES kybernesis::architecture kategoria::monoidal)
kybernesis_module(methods LIBRARIES kybernesis::blocks tyche::decision tekmerion::construal tekmerion::interpretation)
kybernesis_module(loop LIBRARIES kybernesis::methods calx::random)
