// =============================================================================
//  02 · 双指针 (Two Pointers)
//  Q283 移动零 · Q11 盛最多水的容器 · Q15 三数之和 · Q42 接雨水
//
//  【套路】
//  1) 对撞指针：有序 / 能判断「往哪边走更优」时，l 与 r 相向而行，O(n)。
//  2) 快慢指针：原地搬运元素（去重、移动零）或找环。
//  3) 双指针去重：排序后在同一层跳过重复值。以 Q15 为例：
//     外层 `if (i > 0 && nums[i] == nums[i - 1]) continue;`
//     内层 `while (l < r && nums[l] == nums[l + 1]) ++l;`
//     这是所有「返回所有不重复组合」题的通用手法。
// =============================================================================
#include <algorithm>
#include <vector>

#include "test.h"

using namespace std;

// -----------------------------------------------------------------------------
/*
 * Q283. 移动零  Easy
 * https://leetcode.cn/problems/move-zeroes/
 *
 * 思路：快慢指针。慢指针 k 始终指向「下一个非零元素该放的位置」，
 *      快指针扫完全程后，把 [k, n) 全部填 0。等价于 stable_partition。
 * 复杂度：时间 O(n)，空间 O(1)，最少写操作次数
 */
namespace lc0283 {
class Solution {
 public:
  void moveZeroes(vector<int>& nums) {
    int k = 0;
    for (int x : nums)
      if (x != 0) nums[k++] = x;
    while (k < (int)nums.size()) nums[k++] = 0;
  }
};
}  // namespace lc0283

TEST(two_pointers, q283_move_zeroes) {
  lc0283::Solution s;
  vector<int> a{0, 1, 0, 3, 12};
  s.moveZeroes(a);
  CHECK_EQ(a, vector<int>{1, 3, 12, 0, 0});
  vector<int> b{0, 0, 0};
  s.moveZeroes(b);
  CHECK_EQ(b, vector<int>{0, 0, 0});
  vector<int> c{1, 2, 3};
  s.moveZeroes(c);
  CHECK_EQ(c, vector<int>{1, 2, 3});
}

// -----------------------------------------------------------------------------
/*
 * Q11. 盛最多水的容器  Medium
 * https://leetcode.cn/problems/container-with-most-water/
 *
 * 思路：从最宽的两端开始。面积 = 宽 × min(高)，宽度只会变小，
 *      所以想变大必须换掉「较矮的那根」——移动高的一端必然不更优。
 * 复杂度：时间 O(n)，空间 O(1)
 * 反例：移动较高的一端，min 不会变大而宽度变小，面积必降。
 */
namespace lc0011 {
class Solution {
 public:
  int maxArea(vector<int>& height) {
    int l = 0, r = (int)height.size() - 1, ans = 0;
    while (l < r) {
      ans = max(ans, (r - l) * min(height[l], height[r]));
      if (height[l] < height[r])
        ++l;
      else
        --r;
    }
    return ans;
  }
};
}  // namespace lc0011

TEST(two_pointers, q11_container_with_most_water) {
  lc0011::Solution s;
  vector<int> a{1, 8, 6, 2, 5, 4, 8, 3, 7};
  CHECK_EQ(s.maxArea(a), 49);
  vector<int> b{1, 1};
  CHECK_EQ(s.maxArea(b), 1);
  vector<int> c{4, 3, 2, 1, 4};
  CHECK_EQ(s.maxArea(c), 16);
}

// -----------------------------------------------------------------------------
/*
 * Q15. 三数之和  Medium
 * https://leetcode.cn/problems/3sum/
 *
 * 思路：排序 + 固定一个数 + 对撞指针找剩下两个 = -a。
 *      三处去重缺一不可：
 *        a 层：i>0 && nums[i]==nums[i-1] -> continue
 *        b/c 层：命中后 while 跳过左右重复值
 *      剪枝：nums[i] > 0 时后面不可能再凑出 0。
 * 复杂度：时间 O(n^2)，空间 O(log n)（排序栈）
 */
namespace lc0015 {
class Solution {
 public:
  vector<vector<int>> threeSum(vector<int>& nums) {
    sort(nums.begin(), nums.end());
    const int n = (int)nums.size();
    vector<vector<int>> ans;
    for (int i = 0; i + 2 < n; ++i) {
      if (nums[i] > 0) break;            // 剪枝
      if (i > 0 && nums[i] == nums[i - 1]) continue;  // a 去重
      int l = i + 1, r = n - 1;
      while (l < r) {
        int sum = nums[i] + nums[l] + nums[r];
        if (sum < 0) {
          ++l;
        } else if (sum > 0) {
          --r;
        } else {
          ans.push_back({nums[i], nums[l], nums[r]});
          while (l < r && nums[l] == nums[l + 1]) ++l;  // b 去重
          while (l < r && nums[r] == nums[r - 1]) --r;  // c 去重
          ++l;
          --r;
        }
      }
    }
    return ans;
  }
};
}  // namespace lc0015

TEST(two_pointers, q15_3sum) {
  lc0015::Solution s;
  vector<int> a{-1, 0, 1, 2, -1, -4};
  CHECK_EQ(s.threeSum(a), vector<vector<int>>{{-1, -1, 2}, {-1, 0, 1}});
  vector<int> b{0, 0, 0, 0};
  CHECK_EQ(s.threeSum(b), vector<vector<int>>{{0, 0, 0}});  // 不能重复输出
  vector<int> c{1, 1, -2};
  CHECK_EQ(s.threeSum(c), vector<vector<int>>{{-2, 1, 1}});
  vector<int> d{};
  CHECK_EQ(s.threeSum(d), vector<vector<int>>{});
  vector<int> e{1, 2, 3};  // 排序后最小值已 > 0，触发 `if (nums[i] > 0) break;` 剪枝
  CHECK_EQ(s.threeSum(e), vector<vector<int>>{});
}

// -----------------------------------------------------------------------------
/*
 * Q42. 接雨水  Hard
 * https://leetcode.cn/problems/trapping-rain-water/
 *
 * 核心：位置 i 能接的水 = min(左侧最高, 右侧最高) - height[i]。
 * 双指针为什么对：当 height[l] < height[r] 时，右端存在一个比 height[l]
 *   更高的柱子，所以 leftMax 一定 <= 真实的右侧最高 -> 左侧瓶颈已确定，
 *   可以直接结算 left 这一格，不必知道右侧的完整信息。
 * 另解：单调栈按「凹槽」横向累加，与 Q84 柱状图同源（见 12_stack.cpp）。
 * 复杂度：时间 O(n)，空间 O(1)
 */
namespace lc0042 {
class Solution {
 public:
  int trap(vector<int>& height) {
    int l = 0, r = (int)height.size() - 1;
    int leftMax = 0, rightMax = 0, ans = 0;
    while (l < r) {
      leftMax = max(leftMax, height[l]);
      rightMax = max(rightMax, height[r]);
      if (height[l] < height[r])
        ans += leftMax - height[l++];
      else
        ans += rightMax - height[r--];
    }
    return ans;
  }
};
}  // namespace lc0042

TEST(two_pointers, q42_trapping_rain_water) {
  lc0042::Solution s;
  vector<int> a{0, 1, 0, 2, 1, 0, 1, 3, 2, 1, 2, 1};
  CHECK_EQ(s.trap(a), 6);
  vector<int> b{4, 2, 0, 3, 2, 5};
  CHECK_EQ(s.trap(b), 9);
  vector<int> c{3, 0, 3};
  CHECK_EQ(s.trap(c), 3);
  vector<int> d{1, 2, 3, 4, 5};  // 单调无积水
  CHECK_EQ(s.trap(d), 0);
}
