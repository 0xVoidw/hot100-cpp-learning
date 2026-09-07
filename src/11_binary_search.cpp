// =============================================================================
//  11 · 二分查找 (Binary Search)
//  Q35 搜索插入位置 · Q74 搜索二维矩阵 · Q34 查找元素的第一个和最后一个位置 ·
//  Q33 搜索旋转排序数组 · Q153 寻找旋转排序数组中的最小值 ·
//  Q4 寻找两个正序数组的中位数
//
//  【把二分写成两个模板，别每次现推】
//  A. 精确查找（找 target）：while (lo <= hi)，命中即返回，最后 lo 是插入位置。
//  B. 边界查找（找「第一个 >= x」，即 lower_bound）：while (lo < hi)，
//     条件成立就 hi = mid，否则 lo = mid + 1；返回 lo。
//  三条铁律：
//    1. mid = lo + (hi - lo) / 2 —— 防 (lo+hi) 溢出，这是面试硬要求。
//    2. 想清楚区间语义：[lo, hi] 闭区间配 hi = mid - 1；[lo, hi) 半开区间配 hi = mid。
//       混用是死循环 / 越界的根源。
//    3. 二分的本质不是「数组有序」，而是「存在一个 predicate，使序列呈 FFFTTT」。
//       所以旋转数组（Q33/Q153）、答案二分（Q875  koko 吃香蕉）都能二分值域。
//  实盘建议：优先用 std::lower_bound / upper_bound，少写错；但必须会手写。
// =============================================================================
#include <algorithm>
#include <climits>
#include <vector>

#include "test.h"

using namespace std;

// -----------------------------------------------------------------------------
/*
 * Q35. 搜索插入位置  Easy
 * https://leetcode.cn/problems/search-insert-position/
 *
 * 思路：就是 lower_bound —— 第一个 >= target 的下标。
 *      循环不变式：答案始终在 [lo, hi] 内；`while (lo <= hi)` 退出时 lo == hi + 1，
 *      而 lo 就是插入点（模板 A 的这个出口条件背下来：闭区间 -> lo 落在第一个更大的位置）。
 * 复杂度：时间 O(log n)，空间 O(1)
 */
namespace lc0035 {
class Solution {
 public:
  static int searchInsert(const vector<int>& nums, int target) {
    int lo = 0, hi = (int)nums.size() - 1;
    while (lo <= hi) {
      int mid = lo + (hi - lo) / 2;
      if (nums[mid] == target)
        return mid;
      else if (nums[mid] < target)
        lo = mid + 1;
      else
        hi = mid - 1;
    }
    return lo;  // 能走到这里说明 target 不存在，lo 就是第一个 > target 的下标 = 插入位
  }
};
}  // namespace lc0035

TEST(binary_search, q35_search_insert) {
  lc0035::Solution s;
  vector<int> a{1, 3, 5, 6};
  CHECK_EQ(s.searchInsert(a, 5), 2);
  CHECK_EQ(s.searchInsert(a, 2), 1);
  CHECK_EQ(s.searchInsert(a, 7), 4);  // 比所有都大
  CHECK_EQ(s.searchInsert(a, 0), 0);  // 比所有都小
  vector<int> b{1};
  CHECK_EQ(s.searchInsert(b, 1), 0);
  CHECK_EQ(s.searchInsert(std::vector<int>{}, 3), 0);  // 空数组
}

// -----------------------------------------------------------------------------
/*
 * Q74. 搜索二维矩阵  Medium
 * https://leetcode.cn/problems/search-a-2d-matrix/
 *
 * 前提（与 Q240 的区别）：每行递增，且「下一行首元素 > 上一行末元素」，
 *      所以整块矩阵拉平后是一个有序数组 -> 直接在 [0, m*n) 上二分，不用写二维逻辑。
 * 映射：mid -> matrix[mid / n][mid % n]
 * 复杂度：时间 O(log(mn))，空间 O(1)
 */
namespace lc0074 {
class Solution {
 public:
  bool searchMatrix(vector<vector<int>>& matrix, int target) {
    if (matrix.empty() || matrix[0].empty()) return false;  // 防 matrix[0] 越界
    const int R = (int)matrix.size(), C = (int)matrix[0].size();
    int lo = 0, hi = R * C - 1;
    while (lo <= hi) {
      int mid = lo + (hi - lo) / 2;
      int v = matrix[mid / C][mid % C];
      if (v == target)
        return true;
      else if (v < target)
        lo = mid + 1;
      else
        hi = mid - 1;
    }
    return false;
  }
};
}  // namespace lc0074

TEST(binary_search, q74_search_a_2d_matrix) {
  lc0074::Solution s;
  vector<vector<int>> m{{1, 3, 5, 7}, {10, 11, 16, 20}, {23, 30, 34, 60}};
  CHECK_TRUE(s.searchMatrix(m, 3));
  CHECK_FALSE(s.searchMatrix(m, 13));
  CHECK_TRUE(s.searchMatrix(m, 1));    // 第一个元素
  CHECK_TRUE(s.searchMatrix(m, 60));   // 最后一个元素
  vector<vector<int>> one{{5}};
  CHECK_TRUE(s.searchMatrix(one, 5));
  CHECK_FALSE(s.searchMatrix(one, 4));
  vector<vector<int>> none{};  // 空矩阵：不守卫就是 matrix[0] 越界
  CHECK_FALSE(s.searchMatrix(none, 1));
}

// -----------------------------------------------------------------------------
/*
 * Q34. 在排序数组中查找元素的第一个和最后一个位置  Medium
 * https://leetcode.cn/problems/find-first-and-last-position-of-element-in-sorted-array/
 *
 * 思路：两次「找边界」。first = lower_bound(target)，
 *      last = lower_bound(target+1) - 1。找右边界也可以写成 upper_bound。
 *      这就是「target 的取值范围 = [lower_bound(t), upper_bound(t))」。
 * 复杂度：时间 O(log n)，空间 O(1)
 * 提醒：target 可能不存在（要判 *it != target），也可能在末尾（+1 会溢出，
 *      所以手写时找右边界用 upper_bound 语义而不是 target+1）。
 */
namespace lc0034 {
class Solution {
 public:
  // 手写边界版：第一个 >= target 的下标（等于 nums.size() 表示没有）
  static int lowerBound(const vector<int>& nums, int target) {
    int lo = 0, hi = (int)nums.size();  // 半开区间 [lo, hi)
    while (lo < hi) {
      int mid = lo + (hi - lo) / 2;
      if (nums[mid] < target)
        lo = mid + 1;
      else
        hi = mid;  // mid 可能就是答案
    }
    return lo;
  }

  // 第一个 > target 的下标。用 upperBound 而不是 lowerBound(target+1)：
  // 后者在 target == INT_MAX 时会整型溢出（这是本题最阴的坑）。
  static int upperBound(const vector<int>& nums, int target) {
    int lo = 0, hi = (int)nums.size();
    while (lo < hi) {
      int mid = lo + (hi - lo) / 2;
      if (nums[mid] <= target)
        lo = mid + 1;
      else
        hi = mid;
    }
    return lo;
  }

  vector<int> searchRange(const vector<int>& nums, int target) {
    int first = lowerBound(nums, target);
    if (first == (int)nums.size() || nums[first] != target) return {-1, -1};
    return {first, upperBound(nums, target) - 1};
  }

  // STL 版：一行搞定，写业务代码时优先用它
  static vector<int> searchRangeSTL(const vector<int>& nums, int target) {
    auto lo = std::lower_bound(nums.begin(), nums.end(), target);
    if (lo == nums.end() || *lo != target) return {-1, -1};
    auto hi = std::upper_bound(lo, nums.end(), target);
    return {(int)(lo - nums.begin()), (int)(hi - nums.begin()) - 1};
  }
};
}  // namespace lc0034

TEST(binary_search, q34_find_first_last) {
  lc0034::Solution s;
  vector<int> a{5, 7, 7, 8, 8, 10};
  CHECK_EQ(s.searchRange(a, 8), vector<int>{3, 4});
  CHECK_EQ(s.searchRange(a, 6), vector<int>{-1, -1});
  CHECK_EQ(s.searchRange(std::vector<int>{}, 0), vector<int>{-1, -1});
  vector<int> b{1};
  CHECK_EQ(s.searchRange(b, 1), vector<int>{0, 0});
  vector<int> c{2, 2};  // 全部相同
  CHECK_EQ(s.searchRange(c, 2), vector<int>{0, 1});
  // 手写版与 STL 版必须完全一致
  vector<int> d{1, 1, 1, 2, 3, 3, 4, 4, 4, 4};
  for (int t = 0; t <= 5; ++t) CHECK_EQ(s.searchRange(d, t), s.searchRangeSTL(d, t));
  vector<int> e{INT_MAX - 1, INT_MAX, INT_MAX};  // 验证不会因 target+1 溢出
  CHECK_EQ(s.searchRange(e, INT_MAX), vector<int>{1, 2});
  vector<int> f{INT_MIN, INT_MIN};
  CHECK_EQ(s.searchRange(f, INT_MIN), vector<int>{0, 1});
}

// -----------------------------------------------------------------------------
/*
 * Q33. 搜索旋转排序数组  Medium
 * https://leetcode.cn/problems/search-in-rotated-sorted-array/
 *
 * 核心判断：mid 把数组分成两半，「至少有一半是有序的」。
 *   · nums[lo] <= nums[mid] -> 左半有序：target 落在 [nums[lo], nums[mid]) 就去左边，否则右边。
 *   · 否则右半有序：target 落在 (nums[mid], nums[hi]] 就去右边，否则左边。
 * 用 nums[lo] <= nums[mid] 而不是 < ，是为了处理 lo == mid（区间长度 2）。
 * 复杂度：时间 O(log n)，空间 O(1)
 * 陷阱：本题元素互不相同；若有重复就退化成 Q81，需要 nums[lo]==nums[mid] 时 lo++ 慢慢缩。
 */
namespace lc0033 {
class Solution {
 public:
  int search(vector<int>& nums, int target) {
    int lo = 0, hi = (int)nums.size() - 1;
    while (lo <= hi) {
      int mid = lo + (hi - lo) / 2;
      if (nums[mid] == target) return mid;
      if (nums[lo] <= nums[mid]) {  // 左半 [lo, mid] 有序
        if (nums[lo] <= target && target < nums[mid])
          hi = mid - 1;
        else
          lo = mid + 1;
      } else {                      // 右半 [mid, hi] 有序
        if (nums[mid] < target && target <= nums[hi])
          lo = mid + 1;
        else
          hi = mid - 1;
      }
    }
    return -1;
  }
};
}  // namespace lc0033

TEST(binary_search, q33_search_rotated) {
  lc0033::Solution s;
  vector<int> a{4, 5, 6, 7, 0, 1, 2};
  CHECK_EQ(s.search(a, 0), 4);
  CHECK_EQ(s.search(a, 4), 0);
  CHECK_EQ(s.search(a, 3), -1);
  CHECK_EQ(s.search(a, 2), 6);   // 最小值在末尾附近
  CHECK_EQ(s.search(a, 7), 3);   // 最大值
  vector<int> b{1};
  CHECK_EQ(s.search(b, 0), -1);
  vector<int> c{3, 1};
  CHECK_EQ(s.search(c, 1), 1);   // 两元素旋转，考验 lo==mid
  CHECK_EQ(s.search(c, 3), 0);
  vector<int> d{1, 2, 3, 4, 5};  // 没旋转
  CHECK_EQ(s.search(d, 5), 4);
}

// -----------------------------------------------------------------------------
/*
 * Q153. 寻找旋转排序数组中的最小值  Medium
 * https://leetcode.cn/problems/find-minimum-in-rotated-sorted-array/
 *
 * 思路：和「右端点」比。nums[mid] > nums[hi] 说明最小值一定在 mid 右边 -> lo = mid+1；
 *      否则最小值在 [lo, mid] 里 -> hi = mid（mid 仍可能是答案，不能 -1）。
 *      为什么和右端点比而不是左端点？和左端点比无法区分「未旋转」与「已旋转」。
 * 复杂度：时间 O(log n)，空间 O(1)；收缩条件 lo<hi 保证退出时 lo==hi 即最小值下标
 */
namespace lc0153 {
class Solution {
 public:
  int findMin(vector<int>& nums) {
    int lo = 0, hi = (int)nums.size() - 1;
    while (lo < hi) {
      int mid = lo + (hi - lo) / 2;
      if (nums[mid] > nums[hi])
        lo = mid + 1;
      else
        hi = mid;
    }
    return nums[lo];
  }
};
}  // namespace lc0153

TEST(binary_search, q153_find_min_rotated) {
  lc0153::Solution s;
  vector<int> a{3, 4, 5, 1, 2};
  CHECK_EQ(s.findMin(a), 1);
  vector<int> b{4, 5, 6, 7, 0, 1, 2};
  CHECK_EQ(s.findMin(b), 0);
  vector<int> c{11, 13, 15, 17};  // 未旋转
  CHECK_EQ(s.findMin(c), 11);
  vector<int> d{2, 1};
  CHECK_EQ(s.findMin(d), 1);
  vector<int> e{1};
  CHECK_EQ(s.findMin(e), 1);
}

// -----------------------------------------------------------------------------
/*
 * Q4. 寻找两个正序数组的中位数  Hard
 * https://leetcode.cn/problems/median-of-two-sorted-arrays/
 *
 * 思路（划分数法）：中位数把两个数组各自切成「左半 / 右半」，要求
 *      ① 左半元素总数 = 右半（或比右半多 1）
 *      ② A 的左最大 <= B 的右最小，且 B 的左最大 <= A 的右最小
 *    只在较短数组上枚举切点 i（保证 O(log(min(m,n)))），j 由 ① 直接算出。
 *    边界用 ±INF 表示「这一侧没有元素」，可以把四种情况统一成一行 max/min。
 * 复杂度：时间 O(log(min(m,n)))，空间 O(1)
 * 备用解法：求「第 k 小」，每次扔掉两个数组各 k/2 个 -> O(log(m+n))，更好想。
 * 别用：合并后取中位是 O(m+n)，不满足题目要求（虽然能 AC）。
 */
namespace lc0004 {
class Solution {
 public:
  double findMedianSortedArrays(vector<int>& a, vector<int>& b) {
    if (a.size() > b.size()) return findMedianSortedArrays(b, a);  // 让 a 更短
    const int m = (int)a.size(), n = (int)b.size();
    const int half = (m + n + 1) / 2;  // 左半元素总数（奇数时归左半）
    int lo = 0, hi = m;
    while (lo <= hi) {
      int i = lo + (hi - lo) / 2;   // a 贡献 i 个到左半
      int j = half - i;             // b 贡献剩下的
      int aL = (i == 0) ? INT_MIN : a[i - 1];
      int aR = (i == m) ? INT_MAX : a[i];
      int bL = (j == 0) ? INT_MIN : b[j - 1];
      int bR = (j == n) ? INT_MAX : b[j];
      if (aL <= bR && bL <= aR) {  // 切法合法
        if ((m + n) % 2 == 1) return std::max(aL, bL);
        return (static_cast<double>(std::max(aL, bL)) + std::min(aR, bR)) / 2.0;
      }
      if (aL > bR)
        hi = i - 1;  // a 取多了
      else
        lo = i + 1;  // a 取少了
    }
    return 0.0;  // 输入保证有序，理论到不了这里
  }
};
}  // namespace lc0004

TEST(binary_search, q4_median_of_two_sorted_arrays) {
  lc0004::Solution s;
  vector<int> a1{1, 3}, b1{2};
  CHECK_NEAR(s.findMedianSortedArrays(a1, b1), 2.0, 1e-9);
  vector<int> a2{1, 2}, b2{3, 4};
  CHECK_NEAR(s.findMedianSortedArrays(a2, b2), 2.5, 1e-9);
  vector<int> a3{}, b3{1};
  CHECK_NEAR(s.findMedianSortedArrays(a3, b3), 1.0, 1e-9);  // 一个数组为空
  vector<int> a4{2}, b4{};
  CHECK_NEAR(s.findMedianSortedArrays(a4, b4), 2.0, 1e-9);
  vector<int> a5{1, 2, 3, 4, 5}, b5{6, 7, 8};
  CHECK_NEAR(s.findMedianSortedArrays(a5, b5), 4.5, 1e-9);
  vector<int> a6{1, 1}, b6{1, 1};  // 全相等
  CHECK_NEAR(s.findMedianSortedArrays(a6, b6), 1.0, 1e-9);
  vector<int> a7{-5, 3, 6, 12, 15}, b7{-12, -10, -6, -3, 4, 10};
  // 合起来排序后共 11 个，中位数是第 6 个 = 3
  CHECK_NEAR(s.findMedianSortedArrays(a7, b7), 3.0, 1e-9);
}
