include_directories(../lib/public) # 指定头文件目录

file(GLOB source_files "cpp/*.cpp")
# 使用 foreach 循环添加执行目标和链接库
foreach(source_file ${source_files})
    get_filename_component(filename "${source_file}" NAME)
    add_executable(${filename} ${source_file})
    target_link_libraries(${filename} lib)
endforeach(source_file)
