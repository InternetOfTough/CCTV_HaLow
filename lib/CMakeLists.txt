
# FetchContent 를 사용하기 위해 include.
include(FetchContent)

# fmt 라이브러리를 불러온다.
FetchContent_Declare(
  Fmt
  GIT_REPOSITORY "https://github.com/fmtlib/fmt"
  GIT_TAG "7.1.3"
  )
FetchContent_MakeAvailable(Fmt)

# Build gRPC using FetchContent or add_subdirectory
# grpc++ 라이브러리를 불러온다.
message(STATUS "Using gRPC via add_subdirectory (FetchContent).")
option(USE_SYSTEM_GRPC "Use system installed gRPC" OFF)
if(USE_SYSTEM_GRPC)
  # Find system-installed gRPC
  find_package(gRPC CONFIG REQUIRED)
else()
  set(ABSL_ENABLE_INSTALL ON)
  FetchContent_Declare(
    gRPC
    GIT_REPOSITORY https://github.com/grpc/grpc
    GIT_TAG "v1.60.0"  # e.g v1.28.0
  )
  set(FETCHCONTENT_QUIET OFF)
  FetchContent_MakeAvailable(gRPC)
endif()

set(_PROTOBUF_LIBPROTOBUF libprotobuf)
set(_REFLECTION grpc++_reflection)
set(_PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
set(_GRPC_GRPCPP grpc++)
if(CMAKE_CROSSCOMPILING)
  find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
  set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
endif()

# Proto file
get_filename_component(hw_proto "../protos/streaming.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)


SET(proto_gen_dir ${CMAKE_CURRENT_BINARY_DIR}/protos_gen)

# Create the output directory if it doesn't exist
file(MAKE_DIRECTORY ${proto_gen_dir})

# Generated sources
set(hw_proto_srcs "${proto_gen_dir}/streaming.pb.cc")
set(hw_proto_hdrs "${proto_gen_dir}/streaming.pb.h")
set(hw_grpc_srcs "${proto_gen_dir}/streaming.grpc.pb.cc")
set(hw_grpc_hdrs "${proto_gen_dir}/streaming.grpc.pb.h")


find_program(PROTOC protoc)
if(NOT PROTOC)
    message(FATAL_ERROR "protoc not found. Make sure it is installed or specify the path.")
endif()

# Use PROTOC in your CMake commands

find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)
if(NOT GRPC_CPP_PLUGIN)
    message(FATAL_ERROR "grpc_cpp_plugin not found. Make sure it is installed or specify the path.")
endif()


add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${CMAKE_CURRENT_BINARY_DIR}/../bin/protoc
      ARGS --grpc_out "${proto_gen_dir}"
        --cpp_out "${proto_gen_dir}"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc=${CMAKE_CURRENT_BINARY_DIR}/../bin/grpc_cpp_plugin
        # --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
        "${hw_proto}"
      DEPENDS "${hw_proto}")



# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}/protos_gen")
include_directories("${_REFLECTION}")
include_directories("${_GRPC_GRPCPP}")
include_directories("${_PROTOBUF_LIBPROTOBUF}")

# hw_grpc_proto
add_library(hw_grpc_proto
  ${hw_grpc_srcs}
  ${hw_grpc_hdrs}
  ${hw_proto_srcs}
  ${hw_proto_hdrs})
target_link_libraries(hw_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# client 디렉토리에 있는 모든 파일을 CLIENT_SRC_FILES 변수에 추가한다.
file(GLOB_RECURSE CLIENT_SRC_FILES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/client/*.cc
)

# server 디렉토리에 있는 모든 파일을 CLIENT_SRC_FILES 변수에 추가한다.
file(GLOB_RECURSE SERVER_SRC_FILES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/server/*.cc
)

add_library(client_lib STATIC ${CLIENT_SRC_FILES})
add_library(server_lib STATIC ${SERVER_SRC_FILES})



# client_lib 의 include 경로 지정
target_include_directories(client_lib PUBLIC ${CMAKE_SOURCE_DIR}/include/client ${proto_gen_dir})

# server 의 include 경로 지정
target_include_directories(server_lib PUBLIC ${CMAKE_SOURCE_DIR}/include/server ${proto_gen_dir})

# find_library(GRPC_LIBRARY grpc++)
# if(GRPC_LIBRARY)
#     message(STATUS "grpc path: ${GRPC_LIBRARY}")
# else()
#     message(WARNING "grpc library not found.")
# endif()
# find_library(PROTO_LIBRARY libprotobuf)
# if(PROTO_LIBRARY)
#     message(STATUS "proto path: ${PROTO_LIBRARY}")
# else()
#     message(WARNING "proto library not found.")
# endif()

# lib 의 컴파일 옵션 지정
target_compile_options(client_lib PRIVATE -Wall -Werror)
target_compile_options(server_lib PRIVATE -Wall -Werror)

# lib 를 C++ 17 로 컴파일
target_compile_options(client_lib PRIVATE -Wall -Werror)
target_compile_features(server_lib PRIVATE cxx_std_17)


# 만일 어떤 라이브러리 A 를 참조한다고 할 때
#     A 를 헤더 파일과 구현 내부에서 모두 사용한다면 : PUBLIC
#     A 를 내부 구현에서만 사용하고 헤더 파일에서는 사용하지 않는다면 : PRIVATE
#     A 를 헤더 파일에서만 사용하고 내부 구현에서는 사용하지 않는다면 : INTERFACE

#opencv 라이브러리 찾기
find_package( OpenCV REQUIRED )
# include_directories( ${OpenCV_INCLUDE_DIRS} ) -> ${OpenCV_LIBS}를 PUBLIC으로 추가했기 때문에 이건 할 필요가 없음


# client_lib 에 링크
target_link_libraries(client_lib PUBLIC 
    fmt 
    ${OpenCV_LIBS}
    grpc++     
    hw_grpc_proto
    absl::flags
    absl::flags_parse
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})


# server_lib 에 링크
target_link_libraries(server_lib PUBLIC 
    ${OpenCV_LIBS}
    grpc++     
    hw_grpc_proto
    absl::flags
    absl::flags_parse
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})

