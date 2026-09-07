// =============================================================================
//  15 · 动态规划 (Dynamic Programming) —— 一维
//  Q70 爬楼梯 · Q118 杨辉三角 · Q198 打家劫舍 · Q279 完全平方数 ·
//  Q322 零钱兑换 · Q139 单词拆分 · Q300 最长递增子序列 · Q152 乘积最大子数组 ·
//  Q416 分割等和子集 · Q32 最长有效括号
//
//  【写 DP 的固定五问】
//  1. 状态怎么定义？（dp[i] 表示「以 i 结尾」还是「前 i 个」？这一步错了后面全错）
//  2. 转移方程是什么？（dp[i] 由哪些更小的状态算出 —— 就是「最后一步有几种走法」）
//  3. 边界/初始化？（dp[0] 表示空；「凑不出」一般设成 +INF/-1 之类的哨兵）
//  4. 遍历顺序？（依赖前一个 -> 正序；0-1 背包用一维时必须逆序，见 Q416）
//  5. 返回值在哪？（不一定在 dp[n]，Q300/Q152 要取 max(dp[i])）
//
//  【三类一维 DP 的识别信号】
//  · 线性递推（爬楼梯、打家劫舍）：只依赖前 1~2 项 -> 两个变量滚动，空间 O(1)。
//  · 背包（零钱、完全平方、等和子集）：dp[容量]，
//      完全背包（可重复选）内层正序；0-1 背包（每个只能用一次）内层逆序。
//  · 「以 i 结尾」型（LIS、最大子数组积）：dp[i] 只表示「必须选 i」的最优，
//      答案要全局取 max —— 这类最容易只返回 dp[n-1] 而写错。
// =============================================================================
#include <algorithm>
#include <climits>
#include <numeric>
#include <string>
#include <unordered_set>
#include <vector>

#include "test.h"

using namespace std;

// -----------------------------------------------------------------------------
/*
 * Q70. 爬楼梯  Easy
 * https://leetcode.cn/problems/climbing-stairs/
 *
 * 状态：dp[i] = 爬到第 i 阶的方法数。最后一步要么跨 1 阶要么跨 2 阶 -> dp[i]=dp[i-1]+dp[i-2]。
 * 初始化：dp[0]=1（站在地面，「什么都不做」也算一种走法，这样 dp[2]=2 才自然成立）。
 * 复杂度：时间 O(n)，空间 O(1)（滚动变量）
 * 一句话记忆：它是斐波那契，不是组合数求和（虽然数值相等，见 Q746 带成本的变体）。
 */
namespace lc0070 {
class Solution {
 public:
  int climbStairs(int n) {
    int prev = 1, cur = 1;  // dp[0], dp[1]
    for (int i = 2; i <= n; ++i) {
      int next = prev + cur;
      prev = cur;
      cur = next;
    }
    return cur;
  }
};
}  // namespace lc0070

TEST(dp, q70_climbing_stairs) {
  lc0070::Solution s;
  CHECK_EQ(s.climbStairs(1), 1);
  CHECK_EQ(s.climbStairs(2), 2);
  CHECK_EQ(s.climbStairs(3), 3);
  CHECK_EQ(s.climbStairs(10), 89);
  CHECK_EQ(s.climbStairs(45), 1836311903);  // 数据范围上限，恰好不爆 int
}

// -----------------------------------------------------------------------------
/*
 * Q118. 杨辉三角  Easy
 * https://leetcode.cn/problems/pascals-triangle/
 *
 * 状态：tri[i][j] = tri[i-1][j-1] + tri[i-1][j]，边界每行首尾为 1。
 * 复杂度：时间 O(n^2)，空间 O(n^2)（输出本身）
 * 变体：Q119 只返回第 rowIndex 行 -> 逆序原地滚动成一行，空间 O(k)；
 *      另一个视角：第 n 行第 k 个数就是组合数 C(n, k)。
 */
namespace lc0118 {
class Solution {
 public:
  vector<vector<int>> generate(int numRows) {
    vector<vector<int>> tri(numRows);
    for (int i = 0; i < numRows; ++i) {
      tri[i].assign(i + 1, 1);  // 每行首尾都是 1
      for (int j = 1; j < i; ++j) tri[i][j] = tri[i - 1][j - 1] + tri[i - 1][j];
    }
    return tri;
  }
};
}  // namespace lc0118

TEST(dp, q118_pascals_triangle) {
  lc0118::Solution s;
  CHECK_EQ(s.generate(5),
           vector<vector<int>>({{1}, {1, 1}, {1, 2, 1}, {1, 3, 3, 1}, {1, 4, 6, 4, 1}}));
  CHECK_EQ(s.generate(1), vector<vector<int>>({{1}}));
  CHECK_EQ(s.generate(3)[2], vector<int>({1, 2, 1}));
}

// -----------------------------------------------------------------------------
/*
 * Q198. 打家劫舍  Medium
 * https://leetcode.cn/problems/house-robber/
 *
 * 状态：dp[i] = 偷到第 i 间房为止（可以偷 i）的最大金额。
 * 转移：dp[i] = max(dp[i-1], dp[i-2] + nums[i])  —— 要么不偷 i，要么偷 i（则 i-1 必不偷）。
 * 因为只依赖前两项，滚动成两个变量即可 O(1) 空间。
 * 家族：Q213 环形（拆成「不偷首」和「不偷尾」两次线性 DP 取 max）、
 *      Q337 树形 DP（每个结点返回「偷/不偷」两个值，见 08 文件的 Q124 思路）。
 * 复杂度：时间 O(n)，空间 O(1)
 */
namespace lc0198 {
class Solution {
 public:
  int rob(vector<int>& nums) {
    int prev2 = 0, prev1 = 0;  // dp[i-2], dp[i-1]
    for (int x : nums) {
      int cur = max(prev1, prev2 + x);
      prev2 = prev1;
      prev1 = cur;
    }
    return prev1;
  }
};
}  // namespace lc0198

TEST(dp, q198_house_robber) {
  lc0198::Solution s;
  vector<int> a{1, 2, 3, 1};
  CHECK_EQ(s.rob(a), 4);  // 1+3
  vector<int> b{2, 7, 9, 3, 1};
  CHECK_EQ(s.rob(b), 12);  // 2+9+1
  vector<int> c{2, 1};
  CHECK_EQ(s.rob(c), 2);
  vector<int> d{5, 1, 1, 5};  // 下标 0 与 3 不相邻，线性版可以同时偷（换成环形 Q213 才互斥）
  CHECK_EQ(s.rob(d), 10);
  vector<int> e{0};
  CHECK_EQ(s.rob(e), 0);
}

// -----------------------------------------------------------------------------
/*
 * Q279. 完全平方数  Medium
 * https://leetcode.cn/problems/perfect-squares/
 *
 * 视角：这是「物品为 1,4,9,... 且可无限重复」的完全背包，求恰好装满的最少个数。
 * 转移：dp[i] = 1 + min(dp[i - j*j])，j*j <= i。
 * 复杂度：时间 O(n·sqrt(n))，空间 O(n)
 * 数学解：拉格朗日四平方定理 -> 答案只可能是 1/2/3/4；
 *        当 n = 4^a·(8b+7) 时答案为 4（这个判定的 O(log n) 解法常被当作加分项）。
 */
namespace lc0279 {
class Solution {
 public:
  int numSquares(int n) {
    vector<int> dp(n + 1, INT_MAX / 2);  // 除以 2 防止后面 +1 溢出
    dp[0] = 0;
    for (int i = 1; i <= n; ++i)
      for (int j = 1; j * j <= i; ++j) dp[i] = min(dp[i], dp[i - j * j] + 1);
    return dp[n];
  }
};
}  // namespace lc0279

TEST(dp, q279_perfect_squares) {
  lc0279::Solution s;
  CHECK_EQ(s.numSquares(12), 3);  // 4+4+4
  CHECK_EQ(s.numSquares(13), 2);  // 4+9
  CHECK_EQ(s.numSquares(1), 1);
  CHECK_EQ(s.numSquares(7), 4);   // 4+1+1+1（7 = 8·0+7 型）
  CHECK_EQ(s.numSquares(43), 3);  // 25+9+9
  CHECK_EQ(s.numSquares(28), 4);  // 28 = 4·7 -> 需要 4 个
}

// -----------------------------------------------------------------------------
/*
 * Q322. 零钱兑换  Medium
 * https://leetcode.cn/problems/coin-change/
 *
 * 状态：dp[a] = 凑出金额 a 所需的最少硬币数；凑不出记为 -1（这里用 BIG 哨兵）。
 * 转移：完全背包 —— 枚举最后一枚硬币 c：dp[a] = min(dp[a-c]+1)。
 * 为什么不能用贪心：coins=[1,3,4], amount=6，贪心 4+1+1=3 枚，最优 3+3=2 枚。
 * 易错：dp[0]=0；结果判 dp[amount] 是否仍为哨兵；内层正序（硬币可重复使用）。
 * 复杂度：时间 O(amount·|coins|)，空间 O(amount)
 * 变体：Q518 求「组合数」-> 外层硬币、内层金额，累加；Q377 排列数 -> 内外层互换。
 */
namespace lc0322 {
class Solution {
 public:
  int coinChange(vector<int>& coins, int amount) {
    const int BIG = amount + 1;  // 比任何合法解都大，等价于无穷
    vector<int> dp(amount + 1, BIG);
    dp[0] = 0;
    for (int a = 1; a <= amount; ++a)
      for (int c : coins)
        if (c <= a) dp[a] = min(dp[a], dp[a - c] + 1);
    return dp[amount] == BIG ? -1 : dp[amount];
  }
};
}  // namespace lc0322

TEST(dp, q322_coin_change) {
  lc0322::Solution s;
  vector<int> a{1, 2, 5};
  CHECK_EQ(s.coinChange(a, 11), 3);  // 5+5+1
  vector<int> b{2};
  CHECK_EQ(s.coinChange(b, 3), -1);  // 凑不出
  vector<int> c{1};
  CHECK_EQ(s.coinChange(c, 0), 0);
  vector<int> d{1, 3, 4};  // 反例：贪心会给出 3，正确答案是 2
  CHECK_EQ(s.coinChange(d, 6), 2);
  vector<int> e{2, 5};
  CHECK_EQ(s.coinChange(e, 5), 1);   // 恰好一枚
  CHECK_EQ(s.coinChange(e, 4), 2);
  CHECK_EQ(s.coinChange(e, 3), -1);
}

// -----------------------------------------------------------------------------
/*
 * Q139. 单词拆分  Medium
 * https://leetcode.cn/problems/word-break/
 *
 * 状态：dp[i] = s[0..i) 能否被拆成词典里的词。
 * 转移：dp[i] = OR over j<i of (dp[j] && wordSet.count(s[j..i)))。
 *      把「能不能拆」当成布尔背包：词典是物品，位置是容量。
 * 优化：只枚举长度不超过词典最长词长的 j，复杂度从 O(n^2 · 匹配) 降到 O(n·L·匹配)。
 * 复杂度：时间 O(n·L²)（L = 最长词长；内层 substr 拼接 + 哈希本身就要 O(L)），空间 O(n)
 *      想真拿到 O(n·L)：把词典装进 Trie（Q208）沿字符下走，戒掉 substr。
 * 变体：Q140 要输出所有句子 -> 先跑本 DP 判可达，再从 n 反向 DFS 拼答案。
 */
namespace lc0139 {
class Solution {
 public:
  bool wordBreak(const string& s, vector<string>& wordDict) {
    unordered_set<string> dict(wordDict.begin(), wordDict.end());
    size_t maxLen = 0;
    for (const string& w : dict) maxLen = max(maxLen, w.size());
    const int n = (int)s.size();
    vector<bool> dp(n + 1, false);
    dp[0] = true;  // 空串可拆
    for (int i = 1; i <= n; ++i)
      for (int j = max(0, i - (int)maxLen); j < i; ++j)  // 只回看 maxLen 个位置
        if (dp[j] && dict.count(s.substr(j, i - j))) {
          dp[i] = true;
          break;
        }
    return dp[n];
  }
};
}  // namespace lc0139

TEST(dp, q139_word_break) {
  lc0139::Solution s;
  vector<string> a{"leet", "code"};
  CHECK_TRUE(s.wordBreak("leetcode", a));
  vector<string> b{"apple", "pen"};
  CHECK_TRUE(s.wordBreak("applepenapple", b));
  vector<string> c{"cats", "dog", "sand", "and", "cat"};
  CHECK_FALSE(s.wordBreak("catsandog", c));
  vector<string> d{"car", "ca", "rs"};
  CHECK_TRUE(s.wordBreak("cars", d));  // ca + rs（不是 car）
  vector<string> e{"a"};
  CHECK_TRUE(s.wordBreak("aaaa", e));
  CHECK_TRUE(s.wordBreak("", e));  // 空串：什么都不拆就已满足（跟 dp[0]=true 一致）
}

// -----------------------------------------------------------------------------
/*
 * Q300. 最长递增子序列  Medium
 * https://leetcode.cn/problems/longest-increasing-subsequence/
 *
 * 解法 A（O(n^2)）：dp[i] = 以 nums[i] 结尾的 LIS 长度，dp[i]=max(dp[j]+1) for j<i 且
 *      nums[j]<nums[i]。注意答案是 max(dp[i])，不是 dp[n-1]。
 * 解法 B（O(n log n)）：tails[k] = 长度为 k+1 的递增子序列的「最小结尾」。
 *      它本身不是 LIS，但长度就是 LIS 长度。用 lower_bound 找替换位置：
 *      能替换说明存在同样长度但结尾更小的子序列 —— 结尾越小越有潜力延长。
 *      严格递增用 lower_bound；若求「非降」子序列改用 upper_bound。
 * 复杂度：A 时间 O(n^2)；B 时间 O(n log n)，两者空间 O(n)
 * 变体：Q673 最长递增子序列的个数；Q354 俄罗斯套娃（排序后转 LIS）。
 */
namespace lc0300 {
class Solution {
 public:
  static int lengthOfLIS(vector<int>& nums) {
    vector<int> tails;  // tails[k]：长度 k+1 的递增子序列的最小末尾
    for (int x : nums) {
      auto it = lower_bound(tails.begin(), tails.end(), x);  // 第一个 >= x
      if (it == tails.end())
        tails.push_back(x);  // 能延长
      else
        *it = x;  // 替换成更小的结尾
    }
    return (int)tails.size();
  }

  static int lengthOfLISDP(vector<int>& nums) {
    const int n = (int)nums.size();
    vector<int> dp(n, 1);
    int best = n ? 1 : 0;
    for (int i = 1; i < n; ++i) {
      for (int j = 0; j < i; ++j)
        if (nums[j] < nums[i]) dp[i] = max(dp[i], dp[j] + 1);
      best = max(best, dp[i]);
    }
    return best;
  }
};
}  // namespace lc0300

TEST(dp, q300_longest_increasing_subsequence) {
  lc0300::Solution s;
  vector<int> a{10, 9, 2, 5, 3, 7, 101, 18};
  CHECK_EQ(s.lengthOfLIS(a), 4);
  CHECK_EQ(s.lengthOfLISDP(a), 4);  // 两种解法必须一致
  vector<int> b{0, 1, 0, 3, 2, 3};
  CHECK_EQ(s.lengthOfLIS(b), 4);
  CHECK_EQ(s.lengthOfLISDP(b), 4);
  vector<int> c{7, 7, 7, 7, 7};  // 严格递增 -> 长度 1
  CHECK_EQ(s.lengthOfLIS(c), 1);
  CHECK_EQ(s.lengthOfLISDP(c), 1);
  vector<int> d{4, 10, 4, 3, 8, 9};
  CHECK_EQ(s.lengthOfLIS(d), 3);
  CHECK_EQ(s.lengthOfLISDP(d), 3);
  vector<int> e{1};
  CHECK_EQ(s.lengthOfLIS(e), 1);
}

// -----------------------------------------------------------------------------
/*
 * Q152. 乘积最大子数组  Medium
 * https://leetcode.cn/problems/maximum-product-subarray/
 *
 * 关键：乘法会把「最小」变成「最大」（负负得正），所以必须同时维护
 *      以 i 结尾的最大积 curMax 和最小积 curMin，遇到负数时两者交换。
 * 易错：初值必须是 nums[0] 而不是 0 或 1：
 *        · 初值 0 -> 全负数组（如 [-2]）会返回 0（错，应为 -2）；
 *        · 初值 1 -> 相当于允许「什么都不选」。
 * 复杂度：时间 O(n)，空间 O(1)
 * 对比：Q53 最大子数组和只需要一个 cur（负了就归零），这题不行 —— 这就是负数的威力。
 */
namespace lc0152 {
class Solution {
 public:
  int maxProduct(vector<int>& nums) {
    if (nums.empty()) return 0;  // 题面保证 n >= 1；nums[0] 不能裸取
    int ans = nums[0], curMax = nums[0], curMin = nums[0];
    for (size_t i = 1; i < nums.size(); ++i) {
      int x = nums[i];
      if (x < 0) swap(curMax, curMin);  // 乘负数会让大小关系翻转
      curMax = max(x, curMax * x);      // 要么从 x 重新开始，要么接上去
      curMin = min(x, curMin * x);
      ans = max(ans, curMax);
    }
    return ans;
  }
};
}  // namespace lc0152

TEST(dp, q152_maximum_product_subarray) {
  lc0152::Solution s;
  vector<int> a{2, 3, -2, 4};
  CHECK_EQ(s.maxProduct(a), 6);
  vector<int> b{-2, 0, -1};
  CHECK_EQ(s.maxProduct(b), 0);
  vector<int> c{-2};  // 单元素负数，考验初值
  CHECK_EQ(s.maxProduct(c), -2);
  vector<int> d{-2, 3, -4};
  CHECK_EQ(s.maxProduct(d), 24);  // 负负得正
  vector<int> e{2, -5, -2, -4, 3};
  CHECK_EQ(s.maxProduct(e), 24);
  vector<int> f{-4, -3, -2};  // 只有前两个负数能配对：(-4)×(-3)=12；三个一起乘变 -24
  CHECK_EQ(s.maxProduct(f), 12);
}

// -----------------------------------------------------------------------------
/*
 * Q416. 分割等和子集  Medium
 * https://leetcode.cn/problems/partition-equal-subset-sum/
 *
 * 转化：能不能选出若干个数，和恰好为 sum/2（sum 为奇数直接 false）-> 0-1 背包判定。
 * 一维 dp[j] = 「和为 j 是否可达」。
 * 【最重要的细节】内层容量必须逆序遍历：
 *      正序会让 dp[j-w] 在本轮刚被更新过，等于同一个数用了两次（变成完全背包）。
 * 剪枝：j 的上界取 min(target, 已扫过元素之和)，避免无意义遍历。
 * 复杂度：时间 O(n·target)，空间 O(target)
 */
namespace lc0416 {
class Solution {
 public:
  bool canPartition(vector<int>& nums) {
    int sum = accumulate(nums.begin(), nums.end(), 0);
    if (sum % 2) return false;
    int target = sum / 2;
    vector<char> dp(target + 1, false);
    dp[0] = true;
    int reach = 0;  // 目前能凑出的最大和
    for (int w : nums) {
      if (w > target) return false;  // 单个数就超过一半，直接失败
      for (int j = min(target, reach + w); j >= w; --j)  // 逆序！
        if (dp[j - w]) dp[j] = true;
      reach += w;
      if (dp[target]) return true;
    }
    return dp[target];
  }
};
}  // namespace lc0416

TEST(dp, q416_partition_equal_subset_sum) {
  lc0416::Solution s;
  vector<int> a{1, 5, 11, 5};
  CHECK_TRUE(s.canPartition(a));   // 11 == 1+5+5
  vector<int> b{1, 2, 3, 5};
  CHECK_FALSE(s.canPartition(b));  // sum=11 奇数
  vector<int> c{1, 1};
  CHECK_TRUE(s.canPartition(c));
  vector<int> d{100};
  CHECK_FALSE(s.canPartition(d));
  vector<int> e{2, 2, 1, 1};
  CHECK_TRUE(s.canPartition(e));
  vector<int> f{3, 3, 3, 3, 4};  // 需要「同一个数只用一次」才能过，逆序写错就真阳性
  CHECK_FALSE(s.canPartition(f));
}

// -----------------------------------------------------------------------------
/*
 * Q32. 最长有效括号  Hard
 * https://leetcode.cn/problems/longest-valid-parentheses/
 *
 * 思路（栈，最好记）：栈底始终存「最后一个破坏合法性的位置」，初始压 -1。
 *      遇 '(' 压下标；遇 ')' 先弹一个：
 *        · 弹完栈空 -> 这个 ')' 成为新的分隔符，压下它的下标；
 *        · 否则栈顶就是当前匹配段的「左边界前一位」，长度 = i - 栈顶。
 * 思路（DP）：dp[i] = 以 i 结尾的最长有效长度。
 *      s[i]==')' 时看 s[i-1]：是 '(' 则 dp[i]=dp[i-2]+2；
 *      是 ')' 则看 s[i-dp[i-1]-1] 是否为 '('，是则再拼上前面的 dp[i-dp[i-1]-2]。
 * 思路（双指针）：左右各扫一遍，open==close 时更新、close>open 时归零 ——
 *      为什么必须扫两遍？因为 "(()" 只有左到右扫会漏掉。
 * 复杂度：三种都是时间 O(n)；栈 O(n)，DP O(n)，双指针 O(1)
 */
namespace lc0032 {
class Solution {
 public:
  static int longestValidParentheses(const string& s) {
    int ans = 0;
    vector<int> st{
        -1};  // 栈底：最后一个「无法匹配」的位置（哨兵）
    for (int i = 0; i < (int)s.size(); ++i) {
      if (s[i] == '(') {
        st.push_back(i);
      } else {
        st.pop_back();
        if (st.empty())
          st.push_back(i);  // 多余右括号，成为新分隔符
        else
          ans = max(ans, i - st.back());
      }
    }
    return ans;
  }

  static int longestValidParenthesesDP(const string& s) {
    const int n = (int)s.size();
    vector<int> dp(n, 0);  // dp[i]：以 s[i] 结尾的最长有效括号长度
    int ans = 0;
    for (int i = 1; i < n; ++i) {
      if (s[i] != ')') continue;
      if (s[i - 1] == '(') {
        dp[i] = (i >= 2 ? dp[i - 2] : 0) + 2;
      } else {
        int j = i - dp[i - 1] - 1;  // 与 s[i] 配对的候选左括号位置
        if (j >= 0 && s[j] == '(') dp[i] = dp[i - 1] + 2 + (j >= 1 ? dp[j - 1] : 0);
      }
      ans = max(ans, dp[i]);
    }
    return ans;
  }
};
}  // namespace lc0032

TEST(dp, q32_longest_valid_parentheses) {
  lc0032::Solution s;
  CHECK_EQ(s.longestValidParentheses(")()())"), 4);
  CHECK_EQ(s.longestValidParenthesesDP(")()())"), 4);
  CHECK_EQ(s.longestValidParentheses(""), 0);
  CHECK_EQ(s.longestValidParentheses("()"), 2);
  CHECK_EQ(s.longestValidParentheses("(()"), 2);
  CHECK_EQ(s.longestValidParentheses("()(()"), 2);
  CHECK_EQ(s.longestValidParentheses("()(())"), 6);
  CHECK_EQ(s.longestValidParenthesesDP("()(())"), 6);
  CHECK_EQ(s.longestValidParentheses("(()(()"), 2);   // 两段被夹住
  CHECK_EQ(s.longestValidParenthesesDP("())((())"), 4);
  CHECK_EQ(s.longestValidParentheses("())((())"), 4);
}
