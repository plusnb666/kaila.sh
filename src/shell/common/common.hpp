#pragma once

#include <unordered_set>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>
#include <unistd.h>
#include <algorithm>

namespace shell::common
{
    // 内建命令集合，供 type 命令判断“是否为 shell builtin”。
    static std::unordered_set<std::string> commands {
            "cd",
            "echo",
            "exit",
            "pwd",
            "type"
    };

    // 解析 PATH 后缓存的目录列表。
    static std::vector<std::string> paths;

    // HOME 用于 cd ~ 的目录展开。
    static std::string HOME = std::getenv("HOME");
    // PATH 用于查找外部命令所在目录。
    const std::string PATH = std::getenv("PATH");

    // 启动时解析 PATH（以 ':' 分隔），只保留真实存在的目录。
    void setup_paths()
    {
        std::stringstream ss(PATH);
        std::string current_path;
        while (std::getline(ss, current_path, ':'))
        {
            if (std::filesystem::exists(current_path))
            {
                paths.push_back(current_path);
            }
        }
    }

    // 在缓存的 PATH 目录中查找可执行文件，返回完整路径；找不到返回空字符串。
    std::string get_command_path(const std::string& command)
    {
        for (const std::string& path : paths)
        {
            const std::string filepath = path + "/" + command;
            // 需同时满足：路径存在、是普通文件、且有执行权限。
            if (std::filesystem::exists(filepath)
                && std::filesystem::is_regular_file(filepath)
                && access(filepath.c_str(), X_OK) == 0)
            {
                return filepath;
            }
        }
        return "";
    }

    // 处理单引号字符串：提取成一个整体 token（不解析转义）。
    void tokenize_apostrophe(size_t& idx, const std::string& input, std::vector<std::string>& tokenized_input)
    {
        size_t string_end = input.find(input[idx], idx + 1);
        if (string_end > idx + 1)
        {
            tokenized_input.emplace_back(input.substr(idx + 1, string_end - idx - 1));
        }
        // 跳过闭合引号，继续后续扫描。
        idx = string_end + 1;
    }

    // 处理双引号字符串：支持 \" 与 \\ 的转义语义。
    void tokenize_quote(size_t& idx, const std::string& input, std::vector<std::string>& tokenized_input)
    {
        // 略过开头双引号。
        ++idx;
        size_t string_end = input.find('\"', idx);

        // 在双引号内部逐段处理反斜杠，保留被转义字符的字面值。
        for (size_t string_backslash = input.find('\\', idx);
             string_backslash != std::string::npos && string_backslash < string_end;
             string_backslash = input.find('\\', idx))
        {
            tokenized_input.emplace_back(input.substr(idx, string_backslash - idx));
            idx = string_backslash;
            if (input[idx + 1] == '\"' || input[idx + 1] == '\\')
            {
                tokenized_input.emplace_back(std::string{input[++idx]});
                if (input[idx] == '\"')
                {
                    string_end = input.find('\"', idx + 1);
                }
            }
            else
            {
                tokenized_input.emplace_back(std::string{input[idx]});
            }
            idx++;
        }

        // 追加最后一段普通字符并跳过闭合双引号。
        tokenized_input.emplace_back(input.substr(idx, string_end - idx));
        idx = string_end + 1;
    }

    // 将“整体被引号包裹”的命令名规范化成去引号后的实际字符串。
    std::string tokenize_quoted_command(const std::string& command)
    {
        size_t idx = 0;
        std::vector<std::string> tokens;
        if (command[0] == '\'')
        {
            common::tokenize_apostrophe(idx, command, tokens);
        }
        else
        {
            common::tokenize_quote(idx, command, tokens);
        }
        std::string new_command = "";
        for (const std::string& token : tokens)
        {
            new_command += token;
        }
        return new_command;
    }

    // 将参数字符串拆分为 token 序列。
    // 约定：空格也会以 " " 的形式保留，供 echo 等命令按原样重组输出。
    std::vector<std::string> tokenize_input(const std::string& input)
    {
        size_t idx = 0;
        std::vector<std::string> tokenized_input;
        for (; idx < input.size(); )
        {
           // 单引号片段：整体读取。
           if (input[idx] == '\'')
           {
               tokenize_apostrophe(idx, input, tokenized_input);
           }
           // 双引号片段：支持部分转义规则。
           else if (input[idx] == '\"')
           {
               tokenize_quote(idx, input, tokenized_input);
           }
           // 反斜杠转义：将下一个字符按字面量加入 token。
           else if (input[idx] == '\\')
           {
               tokenized_input.emplace_back(std::string{input[idx + 1]});
               idx += 2;
           }
           // 连续空格折叠为单个 " " token，避免重复记录。
           else if (input[idx] == ' ')
           {
               if (!tokenized_input.empty() && tokenized_input.back() != " ")
               {
                   tokenized_input.emplace_back(" ");
               }
               ++idx;
           }
           else
           {
               // 普通文本：扫描到下一个特殊字符（空格/引号/反斜杠）为止。
               size_t string_space = input.find(' ', idx);
               size_t string_apostrophe = input.find('\'', idx);
               size_t string_quote = input.find('\"', idx);
               size_t string_backslash = input.find('\\', idx);

               // 后续没有任何特殊字符，直接收尾。
               if (string_space == std::string::npos && string_apostrophe == std::string::npos && string_quote == std::string::npos && string_backslash == std::string::npos)
               {
                   tokenized_input.emplace_back(input.substr(idx));
                   break;
               }

               // 取最早出现的特殊字符位置作为当前普通片段终点。
               size_t string_end = (std::min)({string_space, string_apostrophe, string_quote, string_backslash});
               tokenized_input.emplace_back(input.substr(idx, string_end - idx));
               idx = string_end;
           }
        }
        return tokenized_input;
    }

    // 抽取“命令关键字 + 参数 token”。
    // 若整行没有空格，或命令本身以引号开头，则将整行当作关键字处理。
    std::pair<std::string, std::vector<std::string>> extract_keyword_input(const std::string& command)
    {
        size_t first_word_idx = command.find(' ');
        if (first_word_idx == std::string::npos || command[0] == '\'' || command[0] == '\"')
        {
            return {command, {}};
        }
        const std::string keyword = command.substr(0, first_word_idx);
        const std::string input = command.substr(first_word_idx + 1);
        const std::vector<std::string> tokenized_input = tokenize_input(input);
        return {keyword, tokenized_input};
    }
}
