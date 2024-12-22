function(NANOPBJSON_GENERATE_CPP)
  cmake_parse_arguments(NANOPBJSON_GENERATE_CPP "" "RELPATH;TARGET" "" ${ARGN})

  if(NOT NANOPBJSON_GENERATE_CPP_UNPARSED_ARGUMENTS)
    return()
  endif()

  set (_proto_srcs)
  if(NANOPBJSON_GENERATE_CPP_APPEND_PATH)
    # Create an include path for each file specified
    foreach(FIL ${NANOPBJSON_GENERATE_CPP_UNPARSED_ARGUMENTS})
      get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
      get_filename_component(ABS_PATH ${ABS_FIL} PATH)
      list(APPEND _nanopb_include_path "-I${ABS_PATH}")
      list(APPEND _proto_srcs ${ABS_FIL})
    endforeach()
  else()
    list(APPEND _nanopb_include_path "-I${CMAKE_CURRENT_SOURCE_DIR}")
  endif()

  set(NANOPB_JSON_GENERATOR_EXECUTABLE ${NANOPBJSON_GENERATOR_SOURCE_DIR}/nanopb_generator.py)

  set (NANOPB_JSON_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/nanopb_json")
  file(MAKE_DIRECTORY ${NANOPB_JSON_GENERATED_DIR})
    

  set (SRCS)
  set (HDRS)
  foreach(FIL ${NANOPBJSON_GENERATE_CPP_UNPARSED_ARGUMENTS})
    # get_filename_component(ABS_FIL ${FIL} ABSOLUTE)  
    get_filename_component(FIL_WE ${FIL} NAME_WLE)
    list(APPEND SRCS "${NANOPB_JSON_GENERATED_DIR}/${FIL_WE}.pb.c")
    list(APPEND HDRS "${NANOPB_JSON_GENERATED_DIR}/${FIL_WE}.pb.h")
  endforeach()

  set_source_files_properties(${SRCS} ${HDRS} PROPERTIES GENERATED TRUE)

  if(NANOPBJSON_GENERATE_CPP_TARGET)
    add_library(${NANOPBJSON_GENERATE_CPP_TARGET} STATIC EXCLUDE_FROM_ALL ${SRCS} ${HDRS})
    target_include_directories(${NANOPBJSON_GENERATE_CPP_TARGET} PUBLIC ${NANOPB_JSON_GENERATED_DIR})
    target_link_libraries(${NANOPBJSON_GENERATE_CPP_TARGET} nanopb_json)
  endif()


  # message(STATUS "SRCS: ${SRCS}")
  # message(STATUS "HDRS: ${HDRS}")
  # message(STATUS "_proto_srcs: ${_proto_srcs}")

  add_custom_command(
    OUTPUT ${SRCS} ${HDRS}
    COMMAND ${CMAKE_COMMAND} -E echo "Generating nanopb json files"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${NANOPB_JSON_GENERATED_DIR}
    COMMAND python ${NANOPB_JSON_GENERATOR_EXECUTABLE} ${_nanopb_include_path} --output-dir=${NANOPB_JSON_GENERATED_DIR} ${_proto_srcs}
    DEPENDS ${NANOPB_JSON_GENERATOR_EXECUTABLE} ${_proto_srcs}
    COMMENT "Running nanopb json generator"
  )

  
endfunction()



#
# Main.
#

# By default have NANOPB_GENERATE_CPP macro pass -I to protoc
# for each directory where a proto file is referenced.
if(NOT DEFINED NANOPBJSON_GENERATE_CPP_APPEND_PATH)
  set(NANOPBJSON_GENERATE_CPP_APPEND_PATH TRUE)
endif()

# Make a really good guess regarding location of NANOPB_JSON_SRC_ROOT_FOLDER
if(NOT DEFINED NANOPB_JSON_SRC_ROOT_FOLDER)
  get_filename_component(NANOPB_JSON_SRC_ROOT_FOLDER
                         ${CMAKE_CURRENT_LIST_DIR}/.. ABSOLUTE)
endif()

# Parse any options given to find_package(... COMPONENTS ...)
# foreach(component ${Nanopb_FIND_COMPONENTS})
#   list(APPEND NANOPB_OPTIONS "--${component}")
# endforeach()

# Find the include directory
find_path(NANOPB_JSON_INCLUDE_DIRS
    pb/json.h
    PATHS ${NANOPB_JSON_SRC_ROOT_FOLDER}/include
    NO_CMAKE_FIND_ROOT_PATH
)
mark_as_advanced(NANOPB_JSON_INCLUDE_DIRS)

# message(STATUS "NANOPB_JSON_INCLUDE_DIRS: ${NANOPB_JSON_INCLUDE_DIRS}")

# Find nanopb source files
set(NANOPB_JSON_SRCS)
set(NANOPB_JSON_HDRS)
list(APPEND _nanopb_json_srcs pbjson_decode.c pbjson_encode.c)
list(APPEND _nanopb_json_hdrs json.h)

foreach(FIL ${_nanopb_json_srcs})
  find_file(${FIL}__nano_pb_file NAMES ${FIL} PATHS ${NANOPB_JSON_SRC_ROOT_FOLDER}/src ${NANOPB_JSON_INCLUDE_DIRS} NO_CMAKE_FIND_ROOT_PATH)
  list(APPEND NANOPB_JSON_SRCS "${${FIL}__nano_pb_file}")
  mark_as_advanced(${FIL}__nano_pb_file)
endforeach()

foreach(FIL ${_nanopb_json_hdrs})
  find_file(${FIL}__nano_pb_file NAMES ${FIL} PATHS ${NANOPB_JSON_INCLUDE_DIRS}/pb NO_CMAKE_FIND_ROOT_PATH)
  mark_as_advanced(${FIL}__nano_pb_file)
  list(APPEND NANOPB_JSON_HDRS "${${FIL}__nano_pb_file}")
endforeach()

# message(STATUS "NANOPB_JSON_SRCS: ${NANOPB_JSON_SRCS}")
# message(STATUS "NANOPB_JSON_HDRS: ${NANOPB_JSON_HDRS}")

# Create the library target
add_library(nanopb_json STATIC EXCLUDE_FROM_ALL ${NANOPB_JSON_SRCS})
target_compile_features(nanopb_json PUBLIC c_std_11)
target_include_directories(nanopb_json PUBLIC ${NANOPB_JSON_INCLUDE_DIRS})




# Find nanopb generator source dir
find_path(NANOPBJSON_GENERATOR_SOURCE_DIR
    NAMES nanopb_generator.py
    DOC "nanopb json generator source"
    PATHS
    ${NANOPB_JSON_SRC_ROOT_FOLDER}/scripts
    NO_DEFAULT_PATH
    NO_CMAKE_FIND_ROOT_PATH
)
mark_as_advanced(NANOPBJSON_GENERATOR_SOURCE_DIR)



include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NanopbJson DEFAULT_MSG
  NANOPB_JSON_INCLUDE_DIRS
  NANOPB_JSON_SRCS NANOPB_JSON_HDRS
  )
