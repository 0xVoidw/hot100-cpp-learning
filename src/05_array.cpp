// =============================================================================
//  05 · 普通数组 (Array)
//  Q53 最大子数组和 · Q56 合并区间 · Q189 轮转数组 · Q238 除自身以外数组的乘积 ·
//  Q41 缺失的第一个正数
//
//  【本组的思维母题】
//  · 一维 DP 入门：Q53 的「以 i 结尾的最大和」是最大子数组问题的原型。
//  · 排序 + 扫描：区间题几乎都是「按左端点排序后线性合并」。
//  · 原地 O(1) 空间：数组下标本身就是哈希表（Q41 索引标记、Q189 三次反转）。
// =============================================================================
#include <algorithm>
#include <climits>
#include <cstdlib>
#include <numeric>
#include <vector>

#include "test.h"

using namespace std;

// -----------------------------------------------------------------------------
/*
 * Q53. 最大子数组和  Medium
 * https://leetcode.cn/problems/maximum-subarray/
 *
 * 思路：Kadane。定义 f = 「以当前元素结尾」的最大子数组和，
 *      f = max(f + x, x)：要么接在前面的段后面，要么另起炉灶。
 *      答案是所有 f 的最大值，不是最后的 f —— 这是最常见的错法。
 * 复杂度：时间 O(n)，空间 O(1)
 * 延伸：分治写法 O(n log n)，顺便能构造出「跨中点」的信息，面试常追问。
 */
namespace lc0053 {
class Solution {
 public:
  int maxSubArray(vector<int>& nums) {
    if (nums.empty()) return 0;  // 力扣保证 n ≥ 1，这里只是不让本地调用碰到 nums[0] 越界
    int f = nums[0], ans = nums[0];
    for (size_t i = 1; i < nums.size(); ++i) {
      f = max(f + nums[i], nums[i]);
      ans = max(ans, f);
    }
    return ans;
  }
};
}  // namespace lc0053

TEST(array, q53_maximum_subarray) {
  lc0053::Solution s;
  vector<int> a{-2, 1, -3, 4, -1, 2, 1, -5, 4};
  CHECK_EQ(s.maxSubArray(a), 6);  // [4,-1,2,1]
  vector<int> b{1};
  CHECK_EQ(s.maxSubArray(b), 1);
  vector<int> c{-1};  // 全负数：答案必须是最大的那个负数，不能是 0
  CHECK_EQ(s.maxSubArray(c), -1);
  vector<int> d{5, 4, -1, 7, 8};
  CHECK_EQ(s.maxSubArray(d), 23);
  vector<int> e{};
  CHECK_EQ(s.maxSubArray(e), 0);  // 空输入不越界（行为不在题面里，仅保证健壮）
}

// -----------------------------------------------------------------------------
/*
 * Q56. 合并区间  Medium
 * https://leetcode.cn/problems/merge-intervals/
 *
 * 思路：按左端点排序后，只需要和「结果里最后一个区间」比：
 *      新区间左端 > 上一个的右端 -> 开新段；否则取 max 扩右端。
 * 复杂度：时间 O(n log n)，空间 O(log n)
 * 易错：合并时必须用 max(r, itv[1])，因为存在 [1,10],[2,3] 这种被包含的情况。
 */
namespace lc0056 {
class Solution {
 public:
  vector<vector<int>> merge(vector<vector<int>>& intervals) {
    sort(intervals.begin(), intervals.end(),
         [](const vector<int>& a, const vector<int>& b) { return a[0] < b[0]; });
    vector<vector<int>> ans;
    for (const auto& itv : intervals) {
      if (!ans.empty() && itv[0] <= ans.back()[1])
        ans.back()[1] = max(ans.back()[1], itv[1]);
      else
        ans.push_back(itv);
    }
    return ans;
  }
};
}  // namespace lc0056

TEST(array, q56_merge_intervals) {
  lc0056::Solution s;
  vector<vector<int>> a{{1, 3}, {2, 6}, {8, 10}, {15, 18}};
  CHECK_EQ(s.merge(a), vector<vector<int>>({{1, 6}, {8, 10}, {15, 18}}));
  vector<vector<int>> b{{1, 4}, {4, 5}};  // 端点相接也要合并
  CHECK_EQ(s.merge(b), vector<vector<int>>({{1, 5}}));
  vector<vector<int>> c{{1, 10}, {2, 3}};  // 包含关系，考验 max
  CHECK_EQ(s.merge(c), vector<vector<int>>({{1, 10}}));
}

// -----------------------------------------------------------------------------
/*
 * Q189. 轮转数组  Medium
 * https://leetcode.cn/problems/rotate-array/
 *
 * 思路：三次反转。整体反转把「尾巴」翻到前面，再分别把两段翻回有序。
 *      [1..7] k=3 -> [7 6 5 | 4 3 2 1] -> [5 6 7 | 1 2 3 4]
 * 复杂度：时间 O(n)，空间 O(1)
 * 易错：k 可能大于 n，必须先 k %= n。
 *      另一种「循环替换」写法也是 O(1)，但要按 gcd(n,k) 分环处理。
 */
namespace lc0189 {
class Solution {
 public:
  void rotate(vector<int>& nums, int k) {
    const int n = (int)nums.size();
    if (n == 0) return;
    k %= n;
    reverse(nums.begin(), nums.end());
    reverse(nums.begin(), nums.begin() + k);
    reverse(nums.begin() + k, nums.end());
  }
};
}  // namespace lc0189

TEST(array, q189_rotate_array) {
  lc0189::Solution s;
  vector<int> a{1, 2, 3, 4, 5, 6, 7};
  s.rotate(a, 3);
  CHECK_EQ(a, vector<int>{5, 6, 7, 1, 2, 3, 4});
  vector<int> b{-1, -100, 3, 99};
  s.rotate(b, 2);
  CHECK_EQ(b, vector<int>{3, 99, -1, -100});
  vector<int> c{1, 2};
  s.rotate(c, 3);  // k > n
  CHECK_EQ(c, vector<int>{2, 1});
}

// -----------------------------------------------------------------------------
/*
 * Q238. 除自身以外数组的乘积  Medium
 * https://leetcode.cn/problems/product-of-array-except-self/
 *
 * 思路：答案 = 左侧前缀积 × 右侧后缀积。两趟扫描，各用一个滚动变量。
 *      题目禁止用除法（还要处理 0），前后缀积是唯一正解。
 * 复杂度：时间 O(n)，空间 O(1)（输出数组不计）
 */
namespace lc0238 {
class Solution {
 public:
  vector<int> productExceptSelf(vector<int>& nums) {
    const int n = (int)nums.size();
    vector<int> ans(n, 1);
    int left = 1;
    for (int i = 0; i < n; ++i) {
      ans[i] = left;  // i 左边所有数的积
      left *= nums[i];
    }
    int right = 1;
    for (int i = n - 1; i >= 0; --i) {
      ans[i] *= right;  // 乘上 i 右边所有数的积
      right *= nums[i];
    }
    return ans;
  }
};
}  // namespace lc0238

TEST(array, q238_product_except_self) {
  lc0238::Solution s;
  vector<int> a{1, 2, 3, 4};
  CHECK_EQ(s.productExceptSelf(a), vector<int>{24, 12, 8, 6});
  vector<int> b{-1, 1, 0, -3, 3};
  CHECK_EQ(s.productExceptSelf(b), vector<int>{0, 0, 9, 0, 0});  // 含 0
  vector<int> c{2, 3};
  CHECK_EQ(s.productExceptSelf(c), vector<int>{3, 2});
}

// -----------------------------------------------------------------------------
/*
 * Q41. 缺失的第一个正数  Hard
 * https://leetcode.cn/problems/first-missing-positive/
 *
 * 思路：答案一定落在 [1, n+1]。把数组自身当哈希表：让值 x 去标记下标 x-1。
 *      先把 <=0 的数改成 n+1（避免干扰标记），再用正负号打标，
 *      第一个没被打标的位置 i 就是答案 i+1。
 * 复杂度：时间 O(n)，空间 O(1)  —— 这才是本题作为 Hard 的考点
 * 另解：原地交换 `while (nums[x-1] != x) swap(...)`，写法更短，同样 O(n)。
 */
namespace lc0041 {
class Solution {
 public:
  int firstMissingPositive(vector<int>& nums) {
    const int n = (int)nums.size();
    for (int& x : nums)
      if (x <= 0) x = n + 1;  // 非正数不可能参与标记，换成哨兵
    for (int i = 0; i < n; ++i) {
      int x = std::abs(nums[i]);
      if (x <= n) nums[x - 1] = -std::abs(nums[x - 1]);  // 标记 x 出现过
    }
    for (int i = 0; i < n; ++i)
      if (nums[i] > 0) return i + 1;
    return n + 1;
  }
};
}  // namespace lc0041

TEST(array, q41_first_missing_positive) {
  lc0041::Solution s;
  vector<int> a{3, 4, -1, 1};
  CHECK_EQ(s.firstMissingPositive(a), 2);
  vector<int> b{1, 2, 0};
  CHECK_EQ(s.firstMissingPositive(b), 3);
  vector<int> c{7, 8, 9, 11, 12};  // 一个都没占住 1..n
  CHECK_EQ(s.firstMissingPositive(c), 1);
  vector<int> d{1, 1};  // 重复
  CHECK_EQ(s.firstMissingPositive(d), 2);
}
