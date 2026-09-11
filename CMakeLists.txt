cmake_minimum_required(VERSION 3.28...3.30)

find_package(Qt6 REQUIRED Widgets)

add_library(nomad-tools MODULE)
add_library(OBS::nomad-tools ALIAS nomad-tools)

target_sources(nomad-tools PRIVATE group-recordings.cpp group-recordings.h nomad-tools.cpp)

target_compile_options(
  nomad-tools
  PRIVATE
    $<$<PLATFORM_ID:Darwin>:-Wno-comma>
    $<$<PLATFORM_ID:Darwin>:-Wno-quoted-include-in-framework-header>
)

target_link_libraries(nomad-tools PRIVATE OBS::frontend-api OBS::libobs Qt::Widgets)

set_target_properties_obs(nomad-tools PROPERTIES FOLDER plugins PREFIX "")
