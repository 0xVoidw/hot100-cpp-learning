// =============================================================================
//  17 · 技巧 (Techs)
//  Q136 只出现一次的数字 · Q169 多数元素 · Q75 颜色分类 ·
//  Q31 下一个排列 · Q287 寻找重复数
//
//  【五个必背技巧】
//  1. 位运算异或：a^a=0、a^0=a、满足交换律结合律
//     -> 「只出现一次」类题目的标准解（Q136）；推广到「出现 3 次」见 Q137，
//        用「统计每一位上 1 的个数 mod 3」或两个掩码状态机。
//  2. 摩尔投票（Boyer-Moore）：维护 candidate + count，相同 +1、不同 -1、归零换人。
//     原理：把「一对不同的元素」互相抵消，出现 > n/2 的元素一定有剩。
//     注意：它只保证「如果存在多数元素，则一定是这个 candidate」；
//     题目没保证存在时，必须再扫一遍验证（Q169 不需要，Q229 的 1/3 版本要）。
//  3. 荷兰国旗三向切分：p0 / i / p2 三个指针把数组切成 <1 / ==1 / >1 三段（即 0 | 1 | 2），一趟排完。
//     这就是快速排序的三路划分，也是「Dutch national flag problem」。
//  4. 下一个排列（Q31）：从右往左找第一个下降位 -> 与右边「刚好比它大」的数交换 ->
//     反转后缀。C++ 有 std::next_permutation，但必须会手写（Q556/Q670 同款）。
//  5. 值域下标当链表用（Q287/Q41/Q442/Q645）：数组元素落在 [1,n] 时，
//     nums[i] 天然构成 i -> nums[i] 的函数图，重复元素 => 图上一定有环 =>
//     用 Floyd 龟兔赛跑找环入口。另一路是「把 nums[x] 对应位置取负」当 visited。
//     两者都是 O(1) 额外空间，且都要求数组可（临时）修改或至少能构造慢指针。
// =============================================================================
#include <algorithm>
#include <numeric>
#include <vector>

#include "test.h"

using namespace std;

// -----------------------------------------------------------------------------
/*
 * Q136. 只出现一次的数字  Easy
 * https://leetcode.cn/problems/single-number/
 *
 * 思路：全部异或。出现两次的互相抵消成 0，剩下的就是那个落单的。
 *      不需要额外空间，O(1) 空间 O(n) 时间，是本题的最优解。
 * 复杂度：时间 O(n)，空间 O(1)
 * 推广：Q137 其余出现 3 次 -> 逐位统计 mod 3；
 *      Q260 有两个落单 -> 全体异或得 x^y，取最低位 1 当分组依据，组内各异或一次。
 */
namespace lc0136 {
class Solution {
 public:
  int singleNumber(vector<int>& nums) {
    int x = 0;
    for (int v : nums) x ^= v;
    return x;
  }
};
}  // namespace lc0136

TEST(tricks, q136_single_number) {
  lc0136::Solution s;
  vector<int> a{2, 2, 1};
  CHECK_EQ(s.singleNumber(a), 1);
  vector<int> b{4, 1, 2, 1, 2};
  CHECK_EQ(s.singleNumber(b), 4);
  vector<int> c{1};
  CHECK_EQ(s.singleNumber(c), 1);
  vector<int> d{-1, -1, -2};  // 负数：异或按位运算，符号无所谓
  CHECK_EQ(s.singleNumber(d), -2);
  vector<int> e{0, 0, 7};
  CHECK_EQ(s.singleNumber(e), 7);
}

// -----------------------------------------------------------------------------
/*
 * Q169. 多数元素  Easy
 * https://leetcode.cn/problems/majority-element/
 *
 * 思路：摩尔投票。count 归零就换候选人 —— 相当于每次消掉一对「不同元素」。
 *      多数元素数量 > n/2，消不完，最后一定剩下它。
 * 复杂度：时间 O(n)，空间 O(1)
 * 对比：哈希统计是 O(n) 空间；排序后取中间元素是 O(n log n)/O(1)（能过但不优雅）。
 * 注意：LeetCode 保证多数元素存在，所以不用再验证；
 *      自己写库时务必补一遍「候选人的真实出现次数」检查。
 * 推广：Q229 出现超过 n/3 的元素 -> 只有两个候选人，最后要分别验证计数。
 */
namespace lc0169 {
class Solution {
 public:
  int majorityElement(vector<int>& nums) {
    int cand = 0, count = 0;
    for (int v : nums) {
      if (count == 0) cand = v;  // 没有候选人就立当前值为候选
      count += (v == cand) ? 1 : -1;
    }
    return cand;
  }
};
}  // namespace lc0169

TEST(tricks, q169_majority_element) {
  lc0169::Solution s;
  vector<int> a{3, 2, 3};
  CHECK_EQ(s.majorityElement(a), 3);
  vector<int> b{2, 2, 1, 1, 1, 2, 2};
  CHECK_EQ(s.majorityElement(b), 2);
  vector<int> c{1};
  CHECK_EQ(s.majorityElement(c), 1);
  vector<int> d{6, 6, 6, 7, 7};  // 候选换人后仍是 6
  CHECK_EQ(s.majorityElement(d), 6);
  vector<int> e{2, 2, 3, 3, 2};  // 前缀刚好抵消，考验归零换人
  CHECK_EQ(s.majorityElement(e), 2);
}

// -----------------------------------------------------------------------------
/*
 * Q75. 颜色分类  Medium
 * https://leetcode.cn/problems/sort-colors/
 *
 * 思路：荷兰国旗三向切分。不变式：
 *      [0, p0) 全 0 | [p0, i) 全 1 | [p2+1, n) 全 2 | [i, p2] 待处理
 *      i 扫到 0 -> 与 p0 交换并同时前进（换回来的必然是 1 或已处理的 0）；
 *      i 扫到 2 -> 与 p2 交换但 i 不动（换回来的值还没看过！这是唯一的坑）。
 * 复杂度：时间 O(n) 一趟，空间 O(1)
 * 对比：两趟计数排序（数出 0/1/2 的个数再回填）也能过，但不满足「一趟」的要求；
 *      这就是三路快排的 partition，遇到大量重复元素时比普通快排快得多。
 */
namespace lc0075 {
class Solution {
 public:
  void sortColors(vector<int>& nums) {
    int p0 = 0, i = 0, p2 = (int)nums.size() - 1;
    while (i <= p2) {
      if (nums[i] == 0) {
        swap(nums[i++], nums[p0++]);  // 换回的是 1（或自己），可以放心前进
      } else if (nums[i] == 2) {
        swap(nums[i], nums[p2--]);    // 换回来的还没看过，i 不动，下一轮再看
      } else {
        ++i;  // 1 就在正确区域
      }
    }
  }
};
}  // namespace lc0075

TEST(tricks, q75_sort_colors) {
  lc0075::Solution s;
  vector<int> a{2, 0, 2, 1, 1, 0};
  s.sortColors(a);
  CHECK_EQ(a, vector<int>{0, 0, 1, 1, 2, 2});
  vector<int> b{2, 0, 1};
  s.sortColors(b);
  CHECK_EQ(b, vector<int>{0, 1, 2});
  vector<int> c{0};
  s.sortColors(c);
  CHECK_EQ(c, vector<int>{0});
  vector<int> d{2, 2, 2};
  s.sortColors(d);
  CHECK_EQ(d, vector<int>{2, 2, 2});
  vector<int> e{1, 2, 0, 1, 2, 0, 1};
  s.sortColors(e);
  CHECK_EQ(e, vector<int>{0, 0, 1, 1, 1, 2, 2});
  vector<int> f{2, 1, 1, 0};
  s.sortColors(f);
  CHECK_EQ(f, vector<int>{0, 1, 1, 2});
}

// -----------------------------------------------------------------------------
/*
 * Q31. 下一个排列  Medium
 * https://leetcode.cn/problems/next-permutation/
 *
 * 思路（字典序 next_permutation 的实现）：
 *   1) 从右往左找第一个 a[i] < a[i+1]，称 i 为「变弱位」；找不到说明整体降序 -> 已是最大。
 *   2) 若存在 i，从右往左找第一个 a[j] > a[i]（在降序后缀里等价于「刚好比 a[i] 大」），交换。
 *   3) 反转 i 之后的部分（它本来降序，反转后即升序 = 最小）。
 * 直觉：把尽量靠右的低位抬高一点点，然后让后面尽可能小。
 * 复杂度：时间 O(n)，空间 O(1)
 * 标准库：std::next_permutation 就是这套逻辑；Q556（四位数的下一个更大元素）、Q670 同款。
 */
namespace lc0031 {
class Solution {
 public:
  void nextPermutation(vector<int>& nums) {
    const int n = (int)nums.size();
    if (n == 0) return;  // 空数组时 i = -2，下面 begin() + i + 1 会算出非法迭代器
    int i = n - 2;
    while (i >= 0 && nums[i] >= nums[i + 1]) --i;  // 步骤 1：找下降位
    if (i >= 0) {
      int j = n - 1;
      while (nums[j] <= nums[i]) --j;  // 步骤 2：右边降序段里第一个更大的
      swap(nums[i], nums[j]);
    }
    reverse(nums.begin() + i + 1, nums.end());  // 步骤 3：后缀变成最小排列
  }
};
}  // namespace lc0031

TEST(tricks, q31_next_permutation) {
  lc0031::Solution s;
  auto next = [&](vector<int> v) {
    s.nextPermutation(v);
    return v;
  };
  CHECK_EQ(next({1, 2, 3}), vector<int>{1, 3, 2});
  CHECK_EQ(next({3, 2, 1}), vector<int>{1, 2, 3});  // 降序 -> 回到最小
  CHECK_EQ(next({1, 1, 5}), vector<int>{1, 5, 1});
  CHECK_EQ(next({1, 3, 2}), vector<int>{2, 1, 3});
  CHECK_EQ(next({2, 3, 1}), vector<int>{3, 1, 2});
  CHECK_EQ(next({1}), vector<int>{1});
  CHECK_EQ(next({5, 1, 1, 5}), vector<int>{5, 1, 5, 1});
  CHECK_EQ(next({1, 2, 3, 4}), vector<int>{1, 2, 4, 3});
  // 与标准库行为一致（对拍 4! 种排列）
  vector<int> base{1, 2, 3, 4};
  sort(base.begin(), base.end());
  do {
    vector<int> mine = base, std_v = base;
    s.nextPermutation(mine);
    std::next_permutation(std_v.begin(), std_v.end());
    CHECK_EQ(mine, std_v);
  } while (next_permutation(base.begin(), base.end()));
}

// -----------------------------------------------------------------------------
/*
 * Q287. 寻找重复数  Medium
 * https://leetcode.cn/problems/find-the-duplicate-number/
 *
 * 约束：不能修改数组、只能用 O(1) 额外空间 —— 排序/哈希/set 全部出局。
 * 思路 A（Floyd 判圈）：把下标当结点、nums[i] 当「下一条边」，
 *        n+1 个数落在 [1,n]，所以一定有环；重复的数就是环的入口。
 *        快慢指针：slow=nums[slow]，fast=nums[nums[fast]]；
 *        相遇后把一个放回起点，各走一步，再相遇处即入口（Q142 同款）。
 * 思路 B（值域二分）：对值 mid 统计「<= mid 的元素个数」，个数 > mid 说明重复值在左半边。
 *        O(n log n)，不需要修改数组，也很好想。
 * 思路 C（原地取负）：把 nums[x] 位置的数取负当 visited —— 但违反「不修改数组」。
 * 复杂度：A 时间 O(n)、空间 O(1)（最优）；B 时间 O(n log n)、空间 O(1)
 */
namespace lc0287 {
class Solution {
 public:
  static int findDuplicate(vector<int>& nums) {
    int slow = nums[0], fast = nums[nums[0]];
    while (slow != fast) {  // 阶段一：找相遇点
      slow = nums[slow];
      fast = nums[nums[fast]];
    }
    slow = 0;  // 阶段二：一个回起点，同速前进（Q142 的结论）
    while (slow != fast) {
      slow = nums[slow];
      fast = nums[fast];
    }
    return slow;
  }

  // 值域二分版：不依赖「慢指针能到起点」这个性质，思路更直白
  static int findDuplicateBinary(vector<int>& nums) {
    int lo = 1, hi = (int)nums.size() - 1;  // 值域是 [1, n]
    while (lo < hi) {
      int mid = lo + (hi - lo) / 2;
      int cnt = 0;
      for (int x : nums)
        if (x <= mid) ++cnt;  // 统计 <= mid 的元素个数
      if (cnt > mid)
        hi = mid;  // 抽屉原理：重复值在 [lo, mid]
      else
        lo = mid + 1;
    }
    return lo;
  }
};
}  // namespace lc0287

TEST(tricks, q287_find_the_duplicate_number) {
  lc0287::Solution s;
  vector<int> a{1, 3, 4, 2, 2};
  CHECK_EQ(s.findDuplicate(a), 2);
  CHECK_EQ(s.findDuplicateBinary(a), 2);
  vector<int> b{3, 1, 3, 4, 2};
  CHECK_EQ(s.findDuplicate(b), 3);
  CHECK_EQ(s.findDuplicateBinary(b), 3);
  vector<int> c{1, 1};
  CHECK_EQ(s.findDuplicate(c), 1);
  CHECK_EQ(s.findDuplicateBinary(c), 1);
  vector<int> d{2, 2, 2, 2, 2};  // 只有一种数且重复多次
  CHECK_EQ(s.findDuplicate(d), 2);
  CHECK_EQ(s.findDuplicateBinary(d), 2);
  vector<int> e{1, 4, 4, 2, 3};  // 长度为 n+1 时元素必须落在 [1,n]（否则下面的快指针会越界）
  CHECK_EQ(s.findDuplicate(e), 4);
  CHECK_EQ(s.findDuplicateBinary(e), 4);
  vector<int> f{1, 2, 3, 4, 5, 6, 6};  // 6 出现两次
  CHECK_EQ(s.findDuplicate(f), 6);
  CHECK_EQ(s.findDuplicateBinary(f), 6);
  // 两版必须完全一致（Floyd 依赖「从 0 出发进环」，二分版不依赖）
  vector<int> g{4, 1, 8, 8, 7, 5, 2, 6, 3};
  CHECK_EQ(s.findDuplicate(g), s.findDuplicateBinary(g));
}
