#pragma once

#include <common/common.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>

namespace shell::type
{

    // 执行 type 内建命令：判断参数是内建命令、外部可执行，还是不存在。
    int execute(const std::vector<std::string>& tokens)
    {
        // 先判定是否为 shell 内建命令。
        if (common::commands.find(tokens[0]) != common::commands.end())
        {
            std::printf("%s is a shell builtin\n", tokens[0].c_str());
            return 0;
        }

        // 再尝试在 PATH 中解析外部程序路径。
        std::string program_path = common::get_command_path(tokens[0]);

        if (!program_path.empty())
        {
            std::printf("%s is %s\n", tokens[0].c_str(), program_path.c_str());
            return 0;
        }
        else
        {
            // 两者都不匹配时，返回 not found。
            std::printf("%s: not found\n", tokens[0].c_str());
            return 1;
        }
    }
}