#include <string>
#include <iostream>

namespace shell::echo
{
    // 执行 echo 内建命令：按 token 顺序原样输出，再追加换行。
    // 这里不主动插入空格，是否有空格由上游分词阶段提供的 token 决定。
    int execute(const std::vector<std::string>& tokens)
    {
        for (const std::string& token : tokens)
        {
            std::printf("%s", token.c_str());
        }
        std::printf("\n");
        return 0;
    }
}