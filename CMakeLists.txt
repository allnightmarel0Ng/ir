cmake_minimum_required(VERSION 3.15)
project(SearchEngine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)
include(FetchContent)

# Find MongoDB (installed system-wide)
find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)

# httplib (header-only)
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.14.3
)
FetchContent_GetProperties(httplib)
if(NOT httplib_POPULATED)
    FetchContent_Populate(httplib)
    add_library(httplib INTERFACE)
    target_include_directories(httplib INTERFACE ${httplib_SOURCE_DIR})
endif()

# jsoncpp
set(JSONCPP_WITH_TESTS OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_WARNING_AS_ERROR OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_PKGCONFIG_SUPPORT OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    jsoncpp
    GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
    GIT_TAG 1.9.5
)
FetchContent_MakeAvailable(jsoncpp)

# Source files
set(SOURCES
    src/main.cpp
    src/text_processing/utf8_converter.cpp
    src/text_processing/tokenizer.cpp
    src/text_processing/query_tokenizer.cpp
    src/text_processing/stemmer.cpp
    src/search/boolean_search.cpp
    src/search/query_parser.cpp
    src/database/mongodb_client.cpp
    src/indexing/indexer.cpp
    src/web/server.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries - note the different target names
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    mongo::mongocxx_shared
    mongo::bsoncxx_shared
    httplib
    jsoncpp_lib
)
