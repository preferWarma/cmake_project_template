set(src memCheck.cpp)

add_library(MyLib OBJECT ${src}) # 设置库名以及库对应的cpp文件

target_include_directories(MyLib PUBLIC ../include) # 将lib文件夹添加进链接库目录, 可以在main.cpp中不用再使用相对路径了, 可以直接include库名