# 현재 디렉토리에 있는 모든 파일을 SRC_FILES 변수에 추가한다.
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cc
)
# # gRPC를 로컬에 설치한 경로
# set(CMAKE_PREFIX_PATH "/usr/bin")



add_executable(program_server ${SRC_FILES})
target_link_libraries(program_server PUBLIC server_lib)

