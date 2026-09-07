// =============================================================================
//  测试驱动器。跑全部：./hot100    只跑某组：./hot100 linked_list
//  过滤规则：对 "suite/用例名" 做子串匹配，所以 ./hot100 q31_ 只跑 Q31。
// =============================================================================
#include <cstdio>
#include <string>

#include "test.h"

int main(int argc, char** argv) {
  // 行缓冲：万一某个用例把进程跑崩（越界、爆栈），崩溃前的进度仍然可见。
  // 不然 stdout 重定向到文件时是全缓冲的，屏幕上一行都看不到。
  std::setvbuf(stdout, nullptr, _IOLBF, 0);
  std::string filter = argc > 1 ? argv[1] : "";
  return lc::runAll(filter);
}
