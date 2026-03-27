#include <string>
#include <iostream>
#include <filesystem>

namespace shell::pwd
{
    // 执行 pwd 内建命令：打印当前工作目录的绝对路径。
    int execute(const std::vector<std::string>&)
    {
        std::printf("%s\n", std::filesystem::current_path().string().c_str());
        return 0;
    }
}