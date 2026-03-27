#pragma once

#include <cd/cd.hpp>
#include <common/common.hpp>
#include <echo/echo.hpp>
#include <exit/exit.hpp>
#include <pwd/pwd.hpp>
#include <type/type.hpp>

#include <unordered_map>
#include <string>
#include <iostream>
#include <functional>
#include <sstream>
#include "cmd/cmd.hpp"

namespace shell
{
    // 内建命令分发表：命令名 -> 对应执行函数。
    // 每个执行函数签名统一为 int(const std::vector<std::string>&)，返回码用于控制 REPL 行为。
    static std::unordered_map<std::string, std::function<int (const std::vector<std::string>&)>> string_to_command {
            {"cd", &cd::execute},
            {"echo", &echo::execute},
            {"exit", &exit::execute},
            {"pwd", &pwd::execute},
            {"type", &type::execute},
    };

    // 执行单条命令。
    // 返回值约定：0=成功，1=失败，-1=请求退出 shell。
    int execute(const std::string& command)
    {
        // 先尝试抽取命令关键字与参数 token。
        auto [keyword, tokenized_input] = common::extract_keyword_input(command);

        // 若命中内建命令，直接走内建处理逻辑。
        if (string_to_command.contains(keyword))
        {
            return string_to_command[keyword](tokenized_input);
        }

        // 兼容被引号包裹的外部命令名（例如 "'ls' -la" 的第一段）。
        if (command[0] == '\'' || command[0] == '\"')
        {
            keyword = common::tokenize_quoted_command(command);
        }

        // 若 PATH 中存在同名可执行文件，则交给系统 shell 执行原始命令字符串。
        if (!common::get_command_path(keyword).empty())
        {
            return std::system(command.c_str());
        }
        else
        {
            // 既不是内建命令，也找不到外部程序，按 shell 习惯返回 command not found。
            std::printf("%s: command not found\n", keyword.c_str());
            return 1;
        }
    }

    // REPL 主循环：持续打印提示符、读取输入、执行命令，直到收到退出信号。
    void repl()
    {
        while (true)
        {
            // 打印经典 shell 提示符。
            std::printf("$ ");
            std::string command;
            // 从标准输入读取一整行命令（保留空格与引号信息）。
            std::getline(std::cin, command);
            int exit_code = execute(command);

            // 约定 -1 表示 exit 命令触发，跳出循环结束程序。
            if (exit_code == -1)
            {
                break;
            }
        }
    }
}