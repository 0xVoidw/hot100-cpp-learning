// =============================================================================
//  03 · 滑动窗口 (Sliding Window)
//  Q3 无重复字符的最长子串 · Q438 找到字符串中所有字母异位词
//
//  【窗口三问】写滑动窗口前先答清楚：
//    1. 右边界进来时，窗口被破坏的条件是什么？（更新状态）
//    2. 什么时候必须收缩左边界？（while 条件）
//    3. 答案在收缩前更新还是收缩后更新？（取值时机）
//  定长窗口（Q438）用 if 每步弹出一个；变长窗口有两种写法：
//  while 逐步缩到合法（Q76），或直接把 left 跳到破坏点之后（Q3 的「跳跃式」）。
// =============================================================================
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

#include "test.h"

using namespace std;

// -----------------------------------------------------------------------------
/*
 * Q3. 无重复字符的最长子串  Medium
 * https://leetcode.cn/problems/longest-substring-without-repeating-characters/
 *
 * 思路：last[c] 记录字符 c 上次出现的下标。遇到重复时不必一步步缩左边界，
 *      直接把 l 跳到 last[c]+1（前提 last[c] >= l，否则会倒退）。
 *      这就是「带跳转的滑动窗口」，比逐步收缩更快也更短。
 * 复杂度：时间 O(n)，空间 O(字符集) —— 这里按 256 个字节取值开定长数组。
 *      写成 last[128] 的话，一旦数据里有 ≥128 的字节（比如 UTF-8 中文）就是栈越界，
 *      而编译器与用例都不一定报得出来。
 */
namespace lc0003 {
class Solution {
 public:
  int lengthOfLongestSubstring(string s) {
    int last[256];  // 必须覆盖 unsigned char 的全部值域 [0,255]
    std::fill(std::begin(last), std::end(last), -1);
    int ans = 0, l = 0;
    for (int r = 0; r < (int)s.size(); ++r) {
      int c = (unsigned char)s[r];
      if (last[c] >= l) l = last[c] + 1;  // 只有重复点在窗口内才收缩
      last[c] = r;
      ans = max(ans, r - l + 1);
    }
    return ans;
  }
};
}  // namespace lc0003

TEST(sliding_window, q3_longest_substring_without_repeating) {
  lc0003::Solution s;
  CHECK_EQ(s.lengthOfLongestSubstring("abcabcbb"), 3);
  CHECK_EQ(s.lengthOfLongestSubstring("bbbbb"), 1);
  CHECK_EQ(s.lengthOfLongestSubstring("pwwkew"), 3);  // "wke"，注意不是子序列
  CHECK_EQ(s.lengthOfLongestSubstring(""), 0);
  CHECK_EQ(s.lengthOfLongestSubstring("dvdf"), 3);    // 验证 l 跳跃不回退
  CHECK_EQ(s.lengthOfLongestSubstring("abba"), 2);    // 经典错误写法会给出 3
  // 高字节（≥128）：数组只开 128 时这里就是越界写
  string hi;
  hi += 'a';
  hi += static_cast<char>(0xFF);
  hi += 'b';
  hi += static_cast<char>(0xFE);
  CHECK_EQ(s.lengthOfLongestSubstring(hi), 4);  // 四个字符全不重复
}

// -----------------------------------------------------------------------------
/*
 * Q438. 找到字符串中所有字母异位词  Medium
 * https://leetcode.cn/problems/find-all-anagrams-in-a-string/
 *
 * 思路：定长窗口（长度 = p.size()）。每步右进一个、左出一个，
 *      维护 26 计数数组，和 need 比较即可。
 *      窗口长度固定，所以用「进一个出一个」而不是 while 收缩。
 * 复杂度：时间 O(26·n)，空间 O(1)
 * 优化：用 diff 计数「还有几个字母数量不对」可做 O(n)，思路同 Q76。
 */
namespace lc0438 {
class Solution {
 public:
  vector<int> findAnagrams(string s, string p) {
    vector<int> ans;
    const int n = (int)s.size(), m = (int)p.size();
    if (n < m) return ans;
    vector<int> need(26, 0), win(26, 0);
    for (char c : p) ++need[c - 'a'];
    for (int i = 0; i < n; ++i) {
      ++win[s[i] - 'a'];          // 右边进
      if (i >= m) --win[s[i - m] - 'a'];  // 左边出，维持定长
      if (i >= m - 1 && win == need) ans.push_back(i - m + 1);
    }
    return ans;
  }
};
}  // namespace lc0438

TEST(sliding_window, q438_find_all_anagrams) {
  lc0438::Solution s;
  CHECK_EQ(s.findAnagrams("cbaebabacd", "abc"), vector<int>{0, 6});
  CHECK_EQ(s.findAnagrams("abab", "ab"), vector<int>{0, 1, 2});
  CHECK_EQ(s.findAnagrams("a", "aa"), vector<int>{});   // s 比 p 短
  CHECK_EQ(s.findAnagrams("abcdef", "xyz"), vector<int>{});
}
