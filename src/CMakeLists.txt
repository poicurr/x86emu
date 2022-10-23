add_executable(main.bin main.cpp)

target_include_directories(main.bin
  PRIVATE ${PROJECT_SOURCE_DIR}/include
)

#target_link_libraries(main.bin PUBLIC -lSDL2 -lm)
