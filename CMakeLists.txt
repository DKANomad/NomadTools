project(nomad-tools)

set(CMAKE_BUILD_TYPE Debug)

set(nomad-tools_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/UI/obs-frontend-api
    ${CMAKE_SOURCE_DIR}/libobs
    ${CMAKE_CURRENT_SOURCE_DIR}/deps)

set(nomad-tools_HEADERS
    nomad-tools.h
    group-recordings.h)

set(nomad-tools_SOURCES
    nomad-tools.cpp
    group-recordings.cpp)

add_library(nomad-tools MODULE ${nomad-tools_SOURCES} ${nomad-tools_HEADERS})

target_include_directories(nomad-tools PUBLIC ${nomad-tools_INCLUDE_DIRS})
target_link_libraries(nomad-tools
      libobs
      obs-frontend-api
      Qt5::Widgets)

set_target_properties(nomad-tools PROPERTIES FOLDER
		"plugins")

install_obs_plugin_with_data(nomad-tools data)
