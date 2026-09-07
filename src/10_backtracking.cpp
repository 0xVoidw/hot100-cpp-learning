// =============================================================================
//  10 · 回溯 (Backtracking)
//  Q46 全排列 · Q78 子集 · Q17 电话号码的字母组合 · Q39 组合总和 ·
//  Q22 括号生成 · Q79 单词搜索 · Q131 分割回文串 · Q51 N 皇后
//
//  【回溯 = 在决策树上做 DFS】
//  模板永远是这三行：
//      选择 -> 递归下一层 -> 撤销选择
//  写之前先回答三个问题：
//    1. 决策树的「一层」是什么？（选一个元素 / 放一行皇后 / 加一个括号）
//    2. 何时算到达路径终点、可以收集答案？（长度够了 / 和等于 target / 无更多可选）
//    3. 剪枝条件是什么？（越界、重复、数量不合法、和已经超过 target）
//  start 参数的三种形态，正是「排列 / 组合 / 子集」的区别：
//    · 组合、子集：下一层传 i+1（不回头，天然无序，避免 {1,2} 与 {2,1} 重复）
//    · 可重复使用的组合（Q39）：下一层传 i（允许再选自己）
//    · 排列：不用 start，每层从 0 扫，靠 used[] 排除已在路径里的元素
// =============================================================================
#include <algorithm>
#include <string>
#include <unordered_set>
#include <vector>

#include "test.h"

using namespace std;

// -----------------------------------------------------------------------------
/*
 * Q46. 全排列  Medium
 * https://leetcode.cn/problems/permutations/
 *
 * 关键点：used[] 标记「这个元素是否已在路径中」；递归到底（path.size()==n）时收集。
 *        撤销顺序必须与选择顺序严格对称：pop 之后要把 used 改回 false。
 * 复杂度：时间 O(n·n!)，空间 O(n)（不计输出）—— n!=120 时已经不便宜，别乱套
 */
namespace lc0046 {
class Solution {
 public:
  vector<vector<int>> permute(vector<int>& nums) {
    vector<vector<int>> ans;
    vector<int> path;
    vector<bool> used(nums.size(), false);
    dfs(nums, used, path, ans);
    return ans;
  }

 private:
  static void dfs(const vector<int>& nums, vector<bool>& used, vector<int>& path,
                  vector<vector<int>>& ans) {
    if ((int)path.size() == (int)nums.size()) {  // 选满了 -> 是一个完整排列
      ans.push_back(path);
      return;
    }
    for (int i = 0; i < (int)nums.size(); ++i) {
      if (used[i]) continue;
      used[i] = true;
      path.push_back(nums[i]);
      dfs(nums, used, path, ans);
      path.pop_back();  // 撤销选择
      used[i] = false;
    }
  }
};
}  // namespace lc0046

TEST(backtracking, q46_permutations) {
  lc0046::Solution s;
  vector<int> a{1, 2, 3};
  CHECK_EQ(s.permute(a),
           vector<vector<int>>({{1, 2, 3}, {1, 3, 2}, {2, 1, 3}, {2, 3, 1}, {3, 1, 2}, {3, 2, 1}}));
  vector<int> b{0, 1};
  CHECK_EQ(s.permute(b), vector<vector<int>>({{0, 1}, {1, 0}}));
  vector<int> c{1};
  CHECK_EQ(s.permute(c), vector<vector<int>>({{1}}));
}

// -----------------------------------------------------------------------------
/*
 * Q78. 子集  Medium
 * https://leetcode.cn/problems/subsets/
 *
 * 思路：每个位置「选 / 不选」，用 start 保证组合内下标递增 -> 不会重复。
 *      与 Q46 的区别：这里每个结点都是答案（不是只在叶子收集），所以进函数就 push。
 * 迭代解法（更好背）：从 {[]} 出发，遇到新元素就把已有答案整体加一份并追加该元素。
 * 复杂度：时间 O(n·2^n)，空间 O(n)
 */
namespace lc0078 {
class Solution {
 public:
  static vector<vector<int>> subsets(vector<int>& nums) {
    vector<vector<int>> ans;
    vector<int> path;
    dfs(nums, 0, path, ans);
    return ans;
  }

  static vector<vector<int>> subsetsIterative(vector<int>& nums) {
    vector<vector<int>> ans{{}};  // 空集打底
    for (int x : nums) {
      int sz = (int)ans.size();
      for (int i = 0; i < sz; ++i) {
        ans.push_back(ans[i]);  // 复制已有子集
        ans.back().push_back(x);
      }
    }
    return ans;
  }

 private:
  static void dfs(const vector<int>& nums, int start, vector<int>& path,
                  vector<vector<int>>& ans) {
    ans.push_back(path);  // 每个结点都是一个合法子集
    for (int i = start; i < (int)nums.size(); ++i) {
      path.push_back(nums[i]);
      dfs(nums, i + 1, path, ans);  // i+1：不回头，避免 {1,2} / {2,1} 重复
      path.pop_back();
    }
  }
};
}  // namespace lc0078

TEST(backtracking, q78_subsets) {
  lc0078::Solution s;
  vector<int> a{1, 2, 3};
  vector<vector<int>> want{{}, {1}, {1, 2}, {1, 2, 3}, {1, 3}, {2}, {2, 3}, {3}};
  CHECK_EQ(s.subsets(a), want);
  auto got = s.subsetsIterative(a);  // 迭代法顺序不同 -> 排序后比集合
  vector<vector<int>> w2 = want;
  sort(got.begin(), got.end());
  sort(w2.begin(), w2.end());
  CHECK_EQ(got, w2);
  vector<int> b{};
  CHECK_EQ(s.subsets(b), vector<vector<int>>({{}}));  // 只有空集
}

// -----------------------------------------------------------------------------
/*
 * Q17. 电话号码的字母组合  Medium
 * https://leetcode.cn/problems/letter-combinations-of-a-phone-number/
 *
 * 思路：多叉树，第 i 层就是第 i 个数字的按键表，逐层展开。
 * 易错：输入 "" 必须返回 []（不是 [""]）—— 这是本题唯一的坑。
 * 复杂度：时间 O(4^n · n)（4 来自 7/9 键有 4 个字母），空间 O(n)
 */
namespace lc0017 {
class Solution {
 public:
  vector<string> letterCombinations(string digits) {
    vector<string> ans;
    if (digits.empty()) return ans;
    dfs(digits, 0, "", ans);
    return ans;
  }

 private:
  static const vector<string>& keys() {
    static const vector<string> k{"",    "", "abc", "def", "ghi", "jkl",
                                 "mno", "pqrs", "tuv", "wxyz"};
    return k;
  }

  static void dfs(const string& digits, int idx, const string& path, vector<string>& ans) {
    if (idx == (int)digits.size()) {
      ans.push_back(path);
      return;
    }
    for (char c : keys()[digits[idx] - '0'])  // 当前数字对应的每个字母都分支一次
      dfs(digits, idx + 1, path + c, ans);
  }
};
}  // namespace lc0017

TEST(backtracking, q17_phone_letters) {
  lc0017::Solution s;
  CHECK_EQ(s.letterCombinations("23"),
           vector<string>{"ad", "ae", "af", "bd", "be", "bf", "cd", "ce", "cf"});
  CHECK_EQ(s.letterCombinations(""), vector<string>{});  // 空输入 != {""}
  CHECK_EQ(s.letterCombinations("2"), vector<string>{"a", "b", "c"});
  CHECK_EQ(s.letterCombinations("79").size(), (size_t)16);  // 4×4
}

// -----------------------------------------------------------------------------
/*
 * Q39. 组合总和  Medium
 * https://leetcode.cn/problems/combination-sum/
 *
 * 思路：组合型回溯 + 「同一个元素可重复选」-> 递归时传 i 而不是 i+1。
 *      排序后可以在 sum > target 时直接 break（后续更大），这是最有价值的剪枝。
 * 对比 Q40（每个元素只能用一次且含重复值）：传 i+1，并加
 *      `if (i > start && candidates[i] == candidates[i-1]) continue;` 去重。
 * 复杂度：时间 O(2^target/min)，空间 O(target/min)
 */
namespace lc0039 {
class Solution {
 public:
  vector<vector<int>> combinationSum(vector<int>& candidates, int target) {
    sort(candidates.begin(), candidates.end());  // 为了 break 剪枝
    vector<vector<int>> ans;
    vector<int> path;
    dfs(candidates, 0, target, 0, path, ans);
    return ans;
  }

 private:
  static void dfs(const vector<int>& c, int start, int target, int sum, vector<int>& path,
                  vector<vector<int>>& ans) {
    if (sum == target) {
      ans.push_back(path);
      return;
    }
    for (int i = start; i < (int)c.size(); ++i) {
      if (sum + c[i] > target) break;  // 已排序 -> 后面只会更大，整层剪掉
      path.push_back(c[i]);
      dfs(c, i, target, sum + c[i], path, ans);  // 注意是 i，不是 i+1
      path.pop_back();
    }
  }
};
}  // namespace lc0039

TEST(backtracking, q39_combination_sum) {
  lc0039::Solution s;
  vector<int> a{2, 3, 6, 7};
  CHECK_EQ(s.combinationSum(a, 7), vector<vector<int>>({{2, 2, 3}, {7}}));
  vector<int> b{2, 3, 5};
  CHECK_EQ(s.combinationSum(b, 8), vector<vector<int>>({{2, 2, 2, 2}, {2, 3, 3}, {3, 5}}));
  vector<int> c{2};
  CHECK_EQ(s.combinationSum(c, 1), vector<vector<int>>{});  // 凑不出来
}

// -----------------------------------------------------------------------------
/*
 * Q22. 括号生成  Medium
 * https://leetcode.cn/problems/generate-parentheses/
 *
 * 思路：任意前缀里「右括号数 <= 左括号数」才合法，所以只有两个约束：
 *      open < n 才能加 '('，close < open 才能加 ')'。走到长度 2n 收集。
 *      这样生成出来的每个串都必然合法，无需事后校验 —— 好的状态定义让剪枝消失。
 * 复杂度：答案数是卡特兰数 C(2n,n)/(n+1)，时间 O(4^n / sqrt(n))
 */
namespace lc0022 {
class Solution {
 public:
  vector<string> generateParenthesis(int n) {
    vector<string> ans;
    string path;
    dfs(n, 0, 0, path, ans);
    return ans;
  }

 private:
  static void dfs(int n, int open, int close, string& path, vector<string>& ans) {
    if ((int)path.size() == 2 * n) {
      ans.push_back(path);
      return;
    }
    if (open < n) {
      path.push_back('(');
      dfs(n, open + 1, close, path, ans);
      path.pop_back();
    }
    if (close < open) {  // 还没配对的左括号数 > 0 才能放右括号
      path.push_back(')');
      dfs(n, open, close + 1, path, ans);
      path.pop_back();
    }
  }
};
}  // namespace lc0022

TEST(backtracking, q22_generate_parentheses) {
  lc0022::Solution s;
  CHECK_EQ(s.generateParenthesis(3),
           vector<string>{"((()))", "(()())", "(())()", "()(())", "()()()"});
  CHECK_EQ(s.generateParenthesis(1), vector<string>{"()"});
  CHECK_EQ(s.generateParenthesis(2).size(), (size_t)2);
  CHECK_EQ(s.generateParenthesis(4).size(), (size_t)14);  // 卡特兰数
}

// -----------------------------------------------------------------------------
/*
 * Q79. 单词搜索  Medium
 * https://leetcode.cn/problems/word-search/
 *
 * 思路：网格 DFS + 回溯。用「临时把格子改成一个非字母占位符」代替 visited 数组，省空间；
 *      但必须还原，否则会破坏入参（多次调用/多次分支都会踩这个坑）。
 * 剪枝：先统计词频，若某个字符在 board 里不够多，直接返回 false（高频加分项，见下方代码）。
 * 复杂度：时间 O(mn·3^L)（每步最多 3 个新方向），空间 O(L)；词频预检 O(mn + L)
 */
namespace lc0079 {
class Solution {
 public:
  bool exist(vector<vector<char>>& board, string word) {
    if (board.empty() || board[0].empty()) return false;  // 空盘搜不出任何东西（题面保证 m,n ≥ 1）
    // 剪枝：词频不够就根本不用搜（面试里这一句常常是真正的加分点）
    int need[26] = {};
    for (char c : word) ++need[(unsigned char)c - 'A'];
    for (const auto& row : board)
      for (char c : row)
        if (c >= 'A' && c <= 'Z') --need[(unsigned char)c - 'A'];
    for (int i = 0; i < 26; ++i)
      if (need[i] > 0) return false;
    const int R = (int)board.size(), C = (int)board[0].size();
    for (int i = 0; i < R; ++i)
      for (int j = 0; j < C; ++j)
        if (dfs(board, word, i, j, 0, R, C)) return true;
    return false;
  }

 private:
  static bool dfs(vector<vector<char>>& b, const string& w, int i, int j, int k, int R, int C) {
    if (k == (int)w.size()) return true;             // 整个词都匹配上了
    if (i < 0 || j < 0 || i >= R || j >= C) return false;
    if (b[i][j] != w[k]) return false;
    char saved = b[i][j];
    b[i][j] = '#';  // 占位：同一格不能在一条路径里用两次
    bool found = dfs(b, w, i + 1, j, k + 1, R, C) || dfs(b, w, i - 1, j, k + 1, R, C) ||
                 dfs(b, w, i, j + 1, k + 1, R, C) || dfs(b, w, i, j - 1, k + 1, R, C);
    b[i][j] = saved;  // 回溯还原 —— 漏了这行，本格被永久标成 '#'，其他分支再用它就得到假阴性
    return found;
  }
};
}  // namespace lc0079

TEST(backtracking, q79_word_search) {
  lc0079::Solution s;
  vector<vector<char>> b{{'A', 'B', 'C', 'E'}, {'S', 'F', 'C', 'S'}, {'A', 'D', 'E', 'E'}};
  CHECK_TRUE(s.exist(b, "ABCCED"));
  CHECK_TRUE(s.exist(b, "SEE"));
  CHECK_FALSE(s.exist(b, "ABCB"));   // 盘上只有一个 B，而且已经被走过 —— 同一格不能在一条路径里用两次
  CHECK_FALSE(s.exist(b, "ABCEZ"));  // 词里有 board 上不存在字符
  CHECK_TRUE(s.exist(b, "A"));
  CHECK_FALSE(s.exist(b, "AAAA"));  // 盘上只有 2 个 A，词频剪枝直接拒掉
  vector<vector<char>> emptyBoard;
  CHECK_FALSE(s.exist(emptyBoard, "A"));  // 空盘：不守卫就是 board[0] 越界
}

// -----------------------------------------------------------------------------
/*
 * Q131. 分割回文串  Medium
 * https://leetcode.cn/problems/palindrome-partitioning/
 *
 * 思路：把「回文切分」看成回溯：每一层枚举下一段的结束位置，是回文才递归下去。
 * 优化：isPal[i][j] 用区间 DP 预处理（Q132 是它的「只计数」版本），
 *      把每次 O(n) 的回文判定降为 O(1)。
 * 复杂度：预处理回文表 O(n^2) 时间 / O(n^2) 空间；输出规模最坏 O(n·2^n)
 */
namespace lc0131 {
class Solution {
 public:
  vector<vector<string>> partition(const string& s) {
    const int n = (int)s.size();
    vector<vector<bool>> pal(n, vector<bool>(n, false));
    for (int i = n - 1; i >= 0; --i)  // 区间 DP：先算短区间
      for (int j = i; j < n; ++j)
        pal[i][j] = s[i] == s[j] && (j - i < 2 || pal[i + 1][j - 1]);
    vector<vector<string>> ans;
    vector<string> path;
    dfs(s, 0, pal, path, ans);
    return ans;
  }

 private:
  static void dfs(const string& s, int start, const vector<vector<bool>>& pal, vector<string>& path,
                  vector<vector<string>>& ans) {
    if (start == (int)s.size()) {
      ans.push_back(path);
      return;
    }
    for (int end = start; end < (int)s.size(); ++end) {
      if (!pal[start][end]) continue;  // 切出来的这段必须回文
      path.push_back(s.substr(start, end - start + 1));
      dfs(s, end + 1, pal, path, ans);
      path.pop_back();
    }
  }
};
}  // namespace lc0131

TEST(backtracking, q131_palindrome_partition) {
  lc0131::Solution s;
  vector<vector<string>> want{{"a", "a", "b"}, {"aa", "b"}};
  auto got = s.partition("aab");
  sort(got.begin(), got.end());
  sort(want.begin(), want.end());
  CHECK_EQ(got, want);
  auto single = s.partition("a");
  CHECK_EQ(single, vector<vector<string>>({{"a"}}));
  auto all = s.partition("aaa");
  CHECK_EQ(all.size(), (size_t)4);  // a|a|a, a|aa, aa|a, aaa
}

// -----------------------------------------------------------------------------
/*
 * Q51. N 皇后  Hard
 * https://leetcode.cn/problems/n-queens/
 *
 * 思路：一行放一个 -> 决策树只有 n 层，每层枚举列。
 *      冲突检查三件事：同列、同主对角线(i-j)、同副对角线(i+j)。
 *      用三个 bool 数组把检查降到 O(1)：cols[j] / diag1[i-j+n-1] / diag2[i+j]。
 * 复杂度：时间 O(n!)，空间 O(n)
 * 变体：Q52 只要方案数 -> 不必存棋盘；位运算版本可以再快几倍。
 */
namespace lc0051 {
class Solution {
 public:
  vector<vector<string>> solveNQueens(int n) {
    vector<vector<string>> ans;
    if (n <= 0) return ans;  // 否则 2*n-1 = -1 会变成巨大的 vector<bool> 尺寸
    vector<string> board(n, string(n, '.'));
    vector<bool> cols(n, false), diag1(2 * n - 1, false), diag2(2 * n - 1, false);
    dfs(n, 0, board, cols, diag1, diag2, ans);
    return ans;
  }

 private:
  static void dfs(int n, int row, vector<string>& board, vector<bool>& cols, vector<bool>& d1,
                  vector<bool>& d2, vector<vector<string>>& ans) {
    if (row == n) {
      ans.push_back(board);
      return;
    }
    for (int col = 0; col < n; ++col) {
      int k1 = row - col + n - 1;  // 主对角线编号（平移成非负）
      int k2 = row + col;          // 副对角线编号
      if (cols[col] || d1[k1] || d2[k2]) continue;
      board[row][col] = 'Q';
      cols[col] = d1[k1] = d2[k2] = true;
      dfs(n, row + 1, board, cols, d1, d2, ans);
      board[row][col] = '.';  // 撤销
      cols[col] = d1[k1] = d2[k2] = false;
    }
  }
};
}  // namespace lc0051

TEST(backtracking, q51_n_queens) {
  lc0051::Solution s;
  auto g4 = s.solveNQueens(4);
  vector<vector<string>> want4{{"..Q.", "Q...", "...Q", ".Q.."}, {".Q..", "...Q", "Q...", "..Q."}};
  sort(g4.begin(), g4.end());
  sort(want4.begin(), want4.end());
  CHECK_EQ(g4, want4);
  CHECK_EQ(s.solveNQueens(1), vector<vector<string>>({{"Q"}}));
  CHECK_EQ(s.solveNQueens(2).size(), (size_t)0);  // 2、3 皇后无解
  CHECK_EQ(s.solveNQueens(3).size(), (size_t)0);
  CHECK_EQ(s.solveNQueens(8).size(), (size_t)92);
  CHECK_EQ(s.solveNQueens(0).size(), (size_t)0);  // n=0 时 2*n-1 = -1，不守卫会抛 length_error
}
