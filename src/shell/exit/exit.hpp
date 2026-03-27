#include <string>
#include <iostream>

namespace shell::exit
{
    // 执行 exit 内建命令。
    // 返回 -1 作为协议值，通知 REPL 跳出主循环并结束程序。
    int execute(const std::vector<std::string>&)
    {
        return -1;
    }
}