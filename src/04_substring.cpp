// =============================================================================
//  04 · 子串 / 前缀和 (Substring & Prefix Sum)
//  Q560 和为K的子数组 · Q239 滑动窗口最大值 · Q76 最小覆盖子串
//
//  【两个新工具】
//  1) 前缀和 + 哈希：区间和 = pre[r] - pre[l-1]。要「和为 K 的子数组个数」，
//     就是对每个 r 数有多少个 l 满足 pre[l] = pre[r] - K —— 计数查表 O(n)。
//     注意：有负数时滑动窗口失效（窗口和不单调），只能用前缀和。
//  2) 单调队列 / 单调栈：维护一个「候选最优」的递减(递增)序列，
//     队首始终是当前窗口最值 —— 每个元素进出队各一次，均摊 O(1)。
// =============================================================================
#include <climits>
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

#include "test.h"

using namespace std;

// -----------------------------------------------------------------------------
/*
 * Q560. 和为 K 的子数组  Medium
 * https://leetcode.cn/problems/subarray-sum-equals-k/
 *
 * 思路：遍历到 r 时，需要知道「之前有几个前缀和等于 cur - K」。
 *      cnt[0] = 1 必须有 —— 它代表「从下标 0 开始就恰好等于 K」的那种子数组。
 * 复杂度：时间 O(n)，空间 O(n)
 * 易错：nums 含负数，不能用滑动窗口（右扩不保证和变大）。
 *      计数用 long long：n = 2×10^5 全为 0、k = 0 时答案是 n(n+1)/2 ≈ 2×10^10，
 *      用 int 累加会在返回前就溢出（签名是力扣固定的 int，至少保证中间过程正确）。
 */
namespace lc0560 {
class Solution {
 public:
  int subarraySum(vector<int>& nums, int k) {
    unordered_map<int, int> cnt;  // 前缀和 -> 出现次数
    cnt[0] = 1;
    int cur = 0;
    long long ans = 0;  // 见上方「易错」：答案本身可能超出 int
    for (int x : nums) {
      cur += x;
      auto it = cnt.find(cur - k);
      if (it != cnt.end()) ans += it->second;
      ++cnt[cur];  // 先查后存，保证子数组非空
    }
    return static_cast<int>(ans);
  }
};
}  // namespace lc0560

TEST(substring, q560_subarray_sum_equals_k) {
  lc0560::Solution s;
  vector<int> a{1, 1, 1};
  CHECK_EQ(s.subarraySum(a, 2), 2);
  vector<int> b{1, 2, 3};
  CHECK_EQ(s.subarraySum(b, 3), 2);  // [1,2] 和 [3]
  vector<int> c{1, -1, 0};
  CHECK_EQ(s.subarraySum(c, 0), 3);  // [1,-1] [1,-1,0] [0]，含负数
  vector<int> d{3, 4, 7, 2, -3, 1, 4, 2};
  CHECK_EQ(s.subarraySum(d, 9), 1);  // 只有 [7,2]：-3 把后面的和拉低后不再凑效
  vector<int> e{0, 0, 0};
  CHECK_EQ(s.subarraySum(e, 0), 6);  // 全 0：所有子数组都合法，C(3+1,2)=6
  vector<int> f{1, 2, 1, 2, 1};
  CHECK_EQ(s.subarraySum(f, 3), 4);
}

// -----------------------------------------------------------------------------
/*
 * Q239. 滑动窗口最大值  Hard
 * https://leetcode.cn/problems/sliding-window-maximum/
 *
 * 思路：单调递减双端队列，存「下标」。
 *      - 新元素进来前，把队尾所有 <= 它的弹出（它们再也不可能当答案了）
 *      - 队首下标滑出窗口左边界就 pop_front
 *      - 每个元素最多入队出队各一次 -> 总 O(n)
 * 复杂度：时间 O(n)，空间 O(k)
 */
namespace lc0239 {
class Solution {
 public:
  vector<int> maxSlidingWindow(vector<int>& nums, int k) {
    deque<int> dq;  // 下标，对应值单调递减
    vector<int> ans;
    for (int i = 0; i < (int)nums.size(); ++i) {
      while (!dq.empty() && dq.front() <= i - k) dq.pop_front();  // 过期
      while (!dq.empty() && nums[dq.back()] <= nums[i]) dq.pop_back();  // 淘汰劣势
      dq.push_back(i);
      if (i >= k - 1) ans.push_back(nums[dq.front()]);
    }
    return ans;
  }
};
}  // namespace lc0239

TEST(substring, q239_sliding_window_maximum) {
  lc0239::Solution s;
  vector<int> a{1, 3, -1, -3, 5, 3, 6, 7};
  CHECK_EQ(s.maxSlidingWindow(a, 3), vector<int>{3, 3, 5, 5, 6, 7});
  vector<int> b{1};
  CHECK_EQ(s.maxSlidingWindow(b, 1), vector<int>{1});
  vector<int> c{9, 8, 7, 6, 5};  // 严格递减 → 每步都不会淘汰队尾，但「过期弹出」把队长压在 ≤ k
  CHECK_EQ(s.maxSlidingWindow(c, 3), vector<int>{9, 8, 7});
  vector<int> d{1, 1, 1};
  CHECK_EQ(s.maxSlidingWindow(d, 2), vector<int>{1, 1});
}

// -----------------------------------------------------------------------------
/*
 * Q76. 最小覆盖子串  Hard
 * https://leetcode.cn/problems/minimum-window-substring/
 *
 * 思路：变长窗口的标准模板 —— 右扩到「合法」，再左缩到「不合法」，
 *      在每次合法的时刻记录答案。
 *      用一个 required 计数器（还差多少个字符），避免每步比较整个计数数组。
 *      技巧：cnt[c] 表示「c 还缺几个」。--cnt[c] >= 0 说明这次拿到的正是还缺的那个（注意是
 *      `>= 0`，不是 `> 0`：补上最后一个缺口时，自减后恰好是 0）。出窗时用反方向的 `> 0`。
 * 复杂度：时间 O(|s| + |t|)，空间 O(字符集)
 */
namespace lc0076 {
class Solution {
 public:
  string minWindow(string s, string t) {
    if (t.empty() || s.size() < t.size()) return "";  // 空 t 直接返回，不等 `required` 变负后才「碰巧」正确
    int cnt[256] = {};  // 按 unsigned char 全值域开，128 会在高字节上越界
    for (char c : t) ++cnt[(unsigned char)c];
    int required = (int)t.size();  // 还差多少个（含重复）字符
    int bestL = 0, bestLen = INT_MAX, l = 0;
    for (int r = 0; r < (int)s.size(); ++r) {
      if (--cnt[(unsigned char)s[r]] >= 0) --required;  // 这个字符是需要的
      while (required == 0) {                            // 窗口已覆盖 t
        if (r - l + 1 < bestLen) {
          bestLen = r - l + 1;
          bestL = l;
        }
        if (++cnt[(unsigned char)s[l]] > 0) ++required;  // 移出后重新缺了
        ++l;
      }
    }
    return bestLen == INT_MAX ? "" : s.substr(bestL, bestLen);
  }
};
}  // namespace lc0076

TEST(substring, q76_minimum_window_substring) {
  lc0076::Solution s;
  CHECK_EQ(s.minWindow("ADOBECODEBANC", "ABC"), string("BANC"));
  CHECK_EQ(s.minWindow("a", "a"), string("a"));
  CHECK_EQ(s.minWindow("a", "aa"), string(""));   // t 有重复，s 不够
  CHECK_EQ(s.minWindow("aa", "aa"), string("aa"));
  CHECK_EQ(s.minWindow("cabwefgewcwaefgcf", "cae"), string("cwae"));
  CHECK_EQ(s.minWindow("abc", ""), string(""));      // 空 t 显式返回空串
}
