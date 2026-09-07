// =============================================================================
//  16 · 多维动态规划 (2D DP)
//  Q62 不同路径 · Q64 最小路径和 · Q5 最长回文子串 ·
//  Q1143 最长公共子序列 · Q72 编辑距离
//
//  【什么时候开二维数组】
//  · 网格型：位置由 (i,j) 决定 -> dp[i][j]（Q62/Q64/Q79 计数版）。
//  · 双串型：状态是「A 的前 i 个 vs B 的前 j 个」-> dp[i][j]（LCS、编辑距离、
//    Q10 正则匹配、Q97 交错字符串）。这类题的通用写法就是三格转移：
//        dp[i][j] = f(dp[i-1][j], dp[i][j-1], dp[i-1][j-1])
//  · 区间型：状态是「子串 s[i..j]」-> dp[i][j]，枚举区间长度 + 断点 k
//    （Q5 回文判定、Q131/Q132 分割、Q312 戳气球）。遍历顺序必须
//    保证 dp[i+1][...] 先算：常见是 i 从大到小、j 从小到大，或按长度枚举。
//  【降维规则】当 dp[i][j] 只依赖第 i-1 行时，可以压成一行；
//    但如果同时还依赖「本行的 dp[i][j-1]」，压一维时要小心更新顺序
//    （从左往右扫正好利用了新值；需要 dp[i-1][j-1] 时要额外存一个变量）。
// =============================================================================
#include <algorithm>
#include <string>
#include <vector>

#include "test.h"

using namespace std;

// -----------------------------------------------------------------------------
/*
 * Q62. 不同路径  Medium
 * https://leetcode.cn/problems/unique-paths/
 *
 * 状态：dp[i][j] = 到 (i,j) 的路径数；只能从上方或左方来 -> 两路相加。
 * 初始化：第一行第一列全是 1（只有一条直路）。
 * 滚动优化：一维数组 dp[j] += dp[j-1]（dp[j] 旧值 = 上方，dp[j-1] 新值 = 左方）。
 * 数学解：C(m+n-2, m-1)，注意中间乘法会溢出 int，要用 long long 逐步除。
 * 复杂度：时间 O(mn)，空间 O(n)（滚动后）
 * 变体：Q63 有障碍物 -> 障碍格 dp=0；Q64 换成求最小和。
 */
namespace lc0062 {
class Solution {
 public:
  int uniquePaths(int m, int n) {
    vector<int> dp(n, 1);  // 第 0 行
    for (int i = 1; i < m; ++i)
      for (int j = 1; j < n; ++j) dp[j] += dp[j - 1];
    return dp[n - 1];
  }

  // 二维版更易读，面试先写这个再问「能不能优化空间」
  static int uniquePaths2D(int m, int n) {
    vector<vector<int>> dp(m, vector<int>(n, 1));
    for (int i = 1; i < m; ++i)
      for (int j = 1; j < n; ++j) dp[i][j] = dp[i - 1][j] + dp[i][j - 1];
    return dp[m - 1][n - 1];
  }
};
}  // namespace lc0062

TEST(dp_multidim, q62_unique_paths) {
  lc0062::Solution s;
  CHECK_EQ(s.uniquePaths(3, 7), 28);
  CHECK_EQ(s.uniquePaths(3, 2), 3);
  CHECK_EQ(s.uniquePaths(1, 1), 1);
  CHECK_EQ(s.uniquePaths(7, 3), 28);
  CHECK_EQ(s.uniquePaths(10, 10), 48620);
  CHECK_EQ(s.uniquePaths2D(10, 10), 48620);
  CHECK_EQ(s.uniquePaths(1, 5), 1);  // 单行只有一条路
}

// -----------------------------------------------------------------------------
/*
 * Q64. 最小路径和  Medium
 * https://leetcode.cn/problems/minimum-path-sum/
 *
 * 状态：dp[i][j] = 到 (i,j) 的最小路径和 = grid[i][j] + min(上, 左)。
 * 边界：第一行只能从左来，第一列只能从上来 —— 忘记这两行初始化是新手最常见错误。
 * 技巧：可以直接在 grid 上原地改（LeetCode 允许，本地练习时注意它改的是入参）。
 * 复杂度：时间 O(mn)，空间 O(n)
 * 变体：Q174 地下城游戏（要从右下角反向 DP，因为「血量」不能为 0，正着推不满足
 *      最优子结构 —— 很好的「为什么方向要反」例子）；三角形最小路径和 Q120。
 */
namespace lc0064 {
class Solution {
 public:
  int minPathSum(vector<vector<int>>& grid) {
    if (grid.empty() || grid[0].empty()) return 0;  // 防 grid[0].size() 越界
    const int R = (int)grid.size(), C = (int)grid[0].size();
    vector<int> dp(C);
    for (int i = 0; i < R; ++i)
      for (int j = 0; j < C; ++j) {
        if (i == 0 && j == 0)
          dp[j] = grid[0][0];
        else if (i == 0)
          dp[j] = dp[j - 1] + grid[i][j];  // 第一行：只有左边
        else if (j == 0)
          dp[j] = dp[j] + grid[i][j];      // 第一列：dp[j] 旧值即上一行
        else
          dp[j] = min(dp[j], dp[j - 1]) + grid[i][j];
      }
    return dp[C - 1];
  }
};
}  // namespace lc0064

TEST(dp_multidim, q64_minimum_path_sum) {
  lc0064::Solution s;
  vector<vector<int>> a{{1, 3, 1}, {1, 5, 1}, {4, 2, 1}};
  CHECK_EQ(s.minPathSum(a), 7);  // 1->3->1->1->1
  vector<vector<int>> b{{1, 2, 3}, {4, 5, 6}};
  CHECK_EQ(s.minPathSum(b), 12);
  vector<vector<int>> c{{5}};
  CHECK_EQ(s.minPathSum(c), 5);
  vector<vector<int>> d{{1, 2, 3}};  // 单行
  CHECK_EQ(s.minPathSum(d), 6);
  vector<vector<int>> e{{1}, {2}, {3}};  // 单列
  CHECK_EQ(s.minPathSum(e), 6);
}

// -----------------------------------------------------------------------------
/*
 * Q5. 最长回文子串  Medium
 * https://leetcode.cn/problems/longest-palindromic-substring/
 *
 * 解法 A（中心扩展，推荐）：枚举 2n-1 个中心（n 个字符中心 + n-1 个间隙中心），
 *      向两边扩。时间 O(n^2)、空间 O(1)，最好写。
 *      「奇偶两类中心」是本题唯一容易漏的点。
 * 解法 B（区间 DP）：dp[i][j] = s[i..j] 是否回文 = s[i]==s[j] && dp[i+1][j-1]。
 *      遍历顺序：i 从大到小（因为依赖 i+1）。Q131 预处理回文表用的就是它。
 * 解法 C（Manacher）：O(n)，利用「已知回文的对称性」跳过重复扩展，面试基本不要求手写。
 * 注意：答案不唯一时（"babad" 的 "aba"/"bab"）只要长度对即可，别在测试里写死。
 * 对比：Q647 是「回文子串个数」（同样中心扩展，累加次数）；
 *      Q680 是「最多删一个字符」（双指针 + 失败时各试一侧）。
 */
namespace lc0005 {
class Solution {
 public:
  string longestPalindrome(const string& s) {
    if (s.size() < 2) return s;
    int bestStart = 0, bestLen = 1;
    for (int c = 0; c < (int)s.size(); ++c) {
      // 两类中心都要试：奇数（以字符为中心）、偶数（以间隙为中心）
      for (int offset : {0, 1}) {
        int i = c, j = c + offset;
        while (i >= 0 && j < (int)s.size() && s[i] == s[j]) {
          --i;
          ++j;
        }
        int len = j - i - 1;  // 退出时 i、j 都多走了一步
        if (len > bestLen) {
          bestLen = len;
          bestStart = i + 1;
        }
      }
    }
    return s.substr(bestStart, bestLen);
  }
};
}  // namespace lc0005

TEST(dp_multidim, q5_longest_palindromic_substring) {
  lc0005::Solution s;
  string r1 = s.longestPalindrome("babad");
  CHECK_TRUE(r1 == "aba" || r1 == "bab");  // 两个都是正确答案
  CHECK_EQ(s.longestPalindrome("cbbd"), string("bb"));
  CHECK_EQ(s.longestPalindrome("a"), string("a"));
  CHECK_EQ(s.longestPalindrome("ac"), string("a"));  // 长度 1，取先出现者
  CHECK_EQ(s.longestPalindrome("aacabdkacaa"), string("aca"));
  CHECK_EQ(s.longestPalindrome("ccc").size(), (size_t)3);  // 全同
  CHECK_EQ(s.longestPalindrome("").size(), (size_t)0);
}

// -----------------------------------------------------------------------------
/*
 * Q1143. 最长公共子序列  Medium
 * https://leetcode.cn/problems/longest-common-subsequence/
 *
 * 状态：dp[i][j] = text1 前 i 个 与 text2 前 j 个 的 LCS 长度。
 * 转移：相等 -> dp[i-1][j-1] + 1（这个字符必进 LCS）；
 *      不等 -> max(dp[i-1][j], dp[i][j-1])（各退一步）。
 * 「子序列 vs 子串」：子串必须连续，状态要改成「以 i/j 结尾」并取 max（Q718）；
 *      子序列可以不连续，所以 dp[i][j] 直接是答案。
 * 复杂度：时间 O(mn)，空间 O(mn)（求长度可滚动成 O(n)，但要输出方案就得留整表）
 * 输出方案：从 (m,n) 反向走，遇到「相等且来自对角线」就记录该字符。
 */
namespace lc01143 {
class Solution {
 public:
  int longestCommonSubsequence(const string& a, const string& b) {
    const int m = (int)a.size(), n = (int)b.size();
    vector<vector<int>> dp(m + 1, vector<int>(n + 1, 0));  // 空串那一行/列是天然边界
    for (int i = 1; i <= m; ++i)
      for (int j = 1; j <= n; ++j)
        dp[i][j] = (a[i - 1] == b[j - 1]) ? dp[i - 1][j - 1] + 1
                                          : max(dp[i - 1][j], dp[i][j - 1]);
    return dp[m][n];
  }
};
}  // namespace lc01143

TEST(dp_multidim, q1143_lcs) {
  lc01143::Solution s;
  CHECK_EQ(s.longestCommonSubsequence("abcde", "ace"), 3);
  CHECK_EQ(s.longestCommonSubsequence("abc", "abc"), 3);
  CHECK_EQ(s.longestCommonSubsequence("abc", "def"), 0);
  CHECK_EQ(s.longestCommonSubsequence("bsbininm", "jmjkbkjkv"), 1);
  CHECK_EQ(s.longestCommonSubsequence("", "a"), 0);
  CHECK_EQ(s.longestCommonSubsequence("oxcpqrsvwf", "shmtulqrypy"), 2);
}

// -----------------------------------------------------------------------------
/*
 * Q72. 编辑距离  Medium
 * https://leetcode.cn/problems/edit-distance/
 *
 * 状态：dp[i][j] = word1 前 i 个变成 word2 前 j 个的最少操作数。
 * 转移：字符相同 -> 直接继承对角线 dp[i-1][j-1]（不需要操作）；
 *      不同 -> 三种操作取最小 +1：
 *        dp[i-1][j-1] + 1  替换
 *        dp[i-1][j]   + 1  删除 word1 的最后一个
 *        dp[i][j-1]   + 1  插入（等价于删 word2 的一个）
 * 边界：dp[i][0] = i（全删）、dp[0][j] = j（全插）。这一行最容易忘。
 * 复杂度：时间 O(mn)，空间 O(mn)；滚动成一行（minDistanceRolling）后空 O(n)，但要多存一个
 *      「左上角旧值」，更新顺序错一位就全错 —— 本题最值钱的一个坑。
 * 家族：Q10 正则匹配、Q97 交错字符串、Q583 只能删除（= LCS 的变体）。
 */
namespace lc0072 {
class Solution {
 public:
  int minDistance(const string& a, const string& b) {
    const int m = (int)a.size(), n = (int)b.size();
    vector<vector<int>> dp(m + 1, vector<int>(n + 1, 0));
    for (int i = 0; i <= m; ++i) dp[i][0] = i;
    for (int j = 0; j <= n; ++j) dp[0][j] = j;
    for (int i = 1; i <= m; ++i)
      for (int j = 1; j <= n; ++j)
        dp[i][j] = (a[i - 1] == b[j - 1])
                       ? dp[i - 1][j - 1]
                       : 1 + min({dp[i - 1][j - 1], dp[i - 1][j], dp[i][j - 1]});
    return dp[m][n];
  }

  // 一维滚动版：注意 prev 保存的是「上一行左上角」的旧值
  static int minDistanceRolling(const string& a, const string& b) {
    const int m = (int)a.size(), n = (int)b.size();
    vector<int> dp(n + 1);
    for (int j = 0; j <= n; ++j) dp[j] = j;
    for (int i = 1; i <= m; ++i) {
      int diag = dp[0];  // dp[i-1][j-1]
      dp[0] = i;
      for (int j = 1; j <= n; ++j) {
        int old = dp[j];  // 这就是下一轮的 dp[i-1][j-1]
        dp[j] = (a[i - 1] == b[j - 1])
                    ? diag
                    : 1 + min({diag, dp[j], dp[j - 1]});  // 替换 / 删 / 插
        diag = old;
      }
    }
    return dp[n];
  }
};
}  // namespace lc0072

TEST(dp_multidim, q72_edit_distance) {
  lc0072::Solution s;
  CHECK_EQ(s.minDistance("horse", "ros"), 3);
  CHECK_EQ(s.minDistanceRolling("horse", "ros"), 3);
  CHECK_EQ(s.minDistance("intention", "execution"), 5);
  CHECK_EQ(s.minDistanceRolling("intention", "execution"), 5);
  CHECK_EQ(s.minDistance("", "abc"), 3);
  CHECK_EQ(s.minDistance("abc", ""), 3);
  CHECK_EQ(s.minDistance("", ""), 0);
  CHECK_EQ(s.minDistance("abc", "abc"), 0);
  CHECK_EQ(s.minDistance("ab", "ba"), 2);
  CHECK_EQ(s.minDistance("foo", "bar"), 3);  // 三位全替换
  CHECK_EQ(s.minDistance("abb", "a"), 2);    // 删两个
  // 二维版与一维滚动版必须在任意输入上一致（互相检验实现正确性）
  vector<pair<string, string>> cases{{"horse", "ros"}, {"abcab", "cba"},  {"", "aaaa"},
                                      {"a", "pqrs"},   {"intention", "execution"}, {"ab", "ba"}};
  for (const auto& c : cases) CHECK_EQ(s.minDistance(c.first, c.second),
                                       s.minDistanceRolling(c.first, c.second));
}
