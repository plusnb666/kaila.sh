#include <command_handler.hpp>
#include <type/type.hpp>

#include <iostream>

// 程序入口：初始化输出缓冲与 PATH 索引，然后进入 Shell 的 REPL 循环。
int main(int argc, char* argv[])
{
  // 让标准输出在每次写入后自动刷新，便于交互式终端即时显示。
  std::cout << std::unitbuf;
  // 错误输出同样启用自动刷新，避免错误信息滞后。
  std::cerr << std::unitbuf;

  // 预解析环境变量 PATH，将可执行目录缓存到内存，后续查找外部命令更快。
  shell::common::setup_paths();

  // 进入 Read-Eval-Print Loop（读取命令 -> 执行 -> 打印提示符）。
  shell::repl();
}
