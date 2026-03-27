#include <string>
#include <iostream>
#include <filesystem>
#include <common/common.hpp>

namespace shell::cd
{
    // 执行 cd 内建命令。
    // tokens[0] 视为目标路径：支持 "~"（HOME）与普通目录切换。
    int execute(const std::vector<std::string>& tokens)
    {
        // cd ~：切换到用户主目录。
        if (tokens[0] == "~")
        {
            std::filesystem::current_path(common::HOME);
            return 0;
        }

        // 目标存在且是目录：执行目录切换。
        if (std::filesystem::exists(tokens[0]) && std::filesystem::is_directory(tokens[0]))
        {
            std::filesystem::current_path(tokens[0]);
            return 0;
        }
        else
        {
            // 与常见 shell 一致：目录不存在时输出错误信息。
            std::printf("cd: %s: No such file or directory\n", tokens[0].c_str());
            return 1;
        }
    }
}