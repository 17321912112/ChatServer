#定义SRC_LIST包含了目录下所有的源文件
aux_source_directory(. SRC_LIST)

#
add_executable(ChatClient client.cpp)
target_link_libraries(ChatClient muduo_net muduo_base pthread)