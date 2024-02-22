# CMake 프로그램의 최소 버전
cmake_minimum_required(VERSION 3.8)

# 프로젝트 정보
project(
  CCTV_STREAMING
  VERSION 0.1
  DESCRIPTION "CCTV 실시간 영상을 위한 인프라 구축"
  LANGUAGES CXX)

# 라이브러리 파일은 빌드 디렉토리 안에 lib 폴더에 출력.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 실행 파일은 빌드 디렉토리 안에 bin 폴더에 출력.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 라즈베리파이에서 빌드 에러 해결을 위해 추가 (juno 24.02.22)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib)
add_subdirectory(app/client)
add_subdirectory(app/server)
add_subdirectory(tests)
