// =============================================================================
//  06 · 矩阵 (Matrix)
//  Q73 矩阵置零 · Q54 螺旋矩阵 · Q48 旋转图像 · Q240 搜索二维矩阵 II
//
//  【套路】
//  1) 边界收缩法：用 top/bottom/left/right 四个变量描述尚未走完的矩形，
//     每走完一条边就收缩一次 —— 螺旋类题目的万能模板，比「方向数组 + 转向」少想很多。
//  2) 借用第一行/第一列当标记位，把 O(m+n) 额外空间压成 O(1)；
//     但要单独记住「第一行/第一列本身是否要置零」，否则会互相污染。
//  3) 阶梯查找：矩阵行列双有序时，从右上角出发，比目标小就下移、大就左移，
//     O(m+n)。左下角同理。绝不能用普通二分（整体不是单调的）。
// =============================================================================
#include <algorithm>
#include <vector>

#include "test.h"

using namespace std;

// -----------------------------------------------------------------------------
/*
 * Q73. 矩阵置零  Medium
 * https://leetcode.cn/problems/set-matrix-zeroes/
 *
 * 思路：用第 0 行 / 第 0 列作为「这一行/列要不要清零」的标记数组。
 *      先单独记录第 0 行、第 0 列本身是否含 0，最后再处理它们。
 *      顺序不能反：标记位写在最后处理，否则会被自己新写的 0 污染。
 * 复杂度：时间 O(mn)，空间 O(1)
 */
namespace lc0073 {
class Solution {
 public:
  void setZeroes(vector<vector<int>>& matrix) {
    if (matrix.empty() || matrix[0].empty()) return;  // 题面保证 m,n ≥ 1，本地防越界
    const int R = (int)matrix.size(), C = (int)matrix[0].size();
    bool zeroRow = false, zeroCol = false;
    for (int j = 0; j < C; ++j)
      if (matrix[0][j] == 0) zeroRow = true;
    for (int i = 0; i < R; ++i)
      if (matrix[i][0] == 0) zeroCol = true;
    for (int i = 1; i < R; ++i)
      for (int j = 1; j < C; ++j)
        if (matrix[i][j] == 0) matrix[i][0] = matrix[0][j] = 0;  // 打标记
    for (int i = 1; i < R; ++i)
      for (int j = 1; j < C; ++j)
        if (matrix[i][0] == 0 || matrix[0][j] == 0) matrix[i][j] = 0;
    if (zeroRow)
      for (int j = 0; j < C; ++j) matrix[0][j] = 0;
    if (zeroCol)
      for (int i = 0; i < R; ++i) matrix[i][0] = 0;
  }
};
}  // namespace lc0073

TEST(matrix, q73_set_matrix_zeroes) {
  lc0073::Solution s;
  vector<vector<int>> a{{1, 1, 1}, {1, 0, 1}, {1, 1, 1}};
  s.setZeroes(a);
  CHECK_EQ(a, vector<vector<int>>({{1, 0, 1}, {0, 0, 0}, {1, 0, 1}}));
  vector<vector<int>> b{{0, 1, 2, 0}, {3, 4, 5, 2}, {1, 3, 1, 5}};
  s.setZeroes(b);
  CHECK_EQ(b, vector<vector<int>>({{0, 0, 0, 0}, {0, 4, 5, 0}, {0, 3, 1, 0}}));
  vector<vector<int>> c{{1}};
  s.setZeroes(c);
  CHECK_EQ(c, vector<vector<int>>({{1}}));
}

// -----------------------------------------------------------------------------
/*
 * Q54. 螺旋矩阵  Medium
 * https://leetcode.cn/problems/spiral-matrix/
 *
 * 思路：四边界收缩。右->下->左->上 各走一遍，每遍结束后收缩对应边界，
 *      并且「左/上」两遍走之前要再判一次区间是否还有效（只剩一行/一列时会重复走）。
 * 复杂度：时间 O(mn)，空间 O(1)（不算输出）
 */
namespace lc0054 {
class Solution {
 public:
  vector<int> spiralOrder(vector<vector<int>>& matrix) {
    vector<int> ans;
    if (matrix.empty() || matrix[0].empty()) return ans;  // 不然下面的 matrix[0].size() 就是越界
    int top = 0, bottom = (int)matrix.size() - 1;
    int left = 0, right = (int)matrix[0].size() - 1;
    while (top <= bottom && left <= right) {
      for (int j = left; j <= right; ++j) ans.push_back(matrix[top][j]);  // →
      ++top;
      for (int i = top; i <= bottom; ++i) ans.push_back(matrix[i][right]);  // ↓
      --right;
      if (top <= bottom) {  // 关键判空：只剩一行时不能再往回走
        for (int j = right; j >= left; --j) ans.push_back(matrix[bottom][j]);  // ←
        --bottom;
      }
      if (left <= right) {  // 只剩一列时同理
        for (int i = bottom; i >= top; --i) ans.push_back(matrix[i][left]);  // ↑
        ++left;
      }
    }
    return ans;
  }
};
}  // namespace lc0054

TEST(matrix, q54_spiral_matrix) {
  lc0054::Solution s;
  vector<vector<int>> a{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
  CHECK_EQ(s.spiralOrder(a), vector<int>{1, 2, 3, 6, 9, 8, 7, 4, 5});
  vector<vector<int>> b{{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}};
  CHECK_EQ(s.spiralOrder(b), vector<int>{1, 2, 3, 4, 8, 12, 11, 10, 9, 5, 6, 7});
  vector<vector<int>> c{{1, 2}, {3, 4}, {5, 6}};  // 竖长
  CHECK_EQ(s.spiralOrder(c), vector<int>{1, 2, 4, 6, 5, 3});
  vector<vector<int>> d{{1, 2, 3}};  // 单行：走到「←」时靠 `top <= bottom` 跳过
  CHECK_EQ(s.spiralOrder(d), vector<int>{1, 2, 3});
  vector<vector<int>> e{{1}, {2}, {3}};  // 单列：靠 `left <= right` 跳过重复的「↑」
  CHECK_EQ(s.spiralOrder(e), vector<int>{1, 2, 3});
  vector<vector<int>> f{};  // 空输入
  CHECK_EQ(s.spiralOrder(f), vector<int>{});
}

// -----------------------------------------------------------------------------
/*
 * Q48. 旋转图像  Medium
 * https://leetcode.cn/problems/rotate-image/
 *
 * 思路：顺时针 90° = 先沿主对角线转置，再把每一行左右翻转。
 *      （逆时针 90° = 转置 + 上下翻转整个矩阵，或先左右翻转再转置）
 * 推导：转置后 matrix[i][j] = 原[j][i]；再行翻转 -> 位置 (i, n-1-j) = 原[j][i]，
 *      即新[i][n-1-j] = 原[j][i]，正是顺时针 90° 的定义。
 * 复杂度：时间 O(n^2)，空间 O(1)
 */
namespace lc0048 {
class Solution {
 public:
  void rotate(vector<vector<int>>& m) {
    const int n = (int)m.size();
    for (int i = 0; i < n; ++i)
      for (int j = i + 1; j < n; ++j)  // j 从 i+1 起，避免交换两次
        std::swap(m[i][j], m[j][i]);
    for (auto& row : m) reverse(row.begin(), row.end());
  }
};
}  // namespace lc0048

TEST(matrix, q48_rotate_image) {
  lc0048::Solution s;
  vector<vector<int>> a{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
  s.rotate(a);
  CHECK_EQ(a, vector<vector<int>>({{7, 4, 1}, {8, 5, 2}, {9, 6, 3}}));
  vector<vector<int>> b{{5, 1}, {9, 2}};
  s.rotate(b);
  CHECK_EQ(b, vector<vector<int>>({{9, 5}, {2, 1}}));
  vector<vector<int>> c{{1}};
  s.rotate(c);
  CHECK_EQ(c, vector<vector<int>>({{1}}));
}

// -----------------------------------------------------------------------------
/*
 * Q240. 搜索二维矩阵 II  Medium
 * https://leetcode.cn/problems/search-a-2d-matrix-ii/
 *
 * 思路：从右上角出发的「阶梯下降」。该位置比 target 大 -> 整列太大，左移；
 *      比 target 小 -> 整行太小，下移。每步排除一行或一列。
 * 复杂度：时间 O(m+n)，空间 O(1)
 * 易错：不能从左上角或右下角出发 —— 那两个方向「大小关系」是不确定的。
 */
namespace lc0240 {
class Solution {
 public:
  bool searchMatrix(vector<vector<int>>& matrix, int target) {
    if (matrix.empty() || matrix[0].empty()) return false;  // 空矩阵不能取 matrix[0]
    int i = 0, j = (int)matrix[0].size() - 1;  // 右上角
    while (i < (int)matrix.size() && j >= 0) {
      if (matrix[i][j] == target)
        return true;
      else if (matrix[i][j] > target)
        --j;  // 这一列都比 target 大
      else
        ++i;  // 这一行都比 target 小
    }
    return false;
  }
};
}  // namespace lc0240

TEST(matrix, q240_search_a_2d_matrix_ii) {
  lc0240::Solution s;
  vector<vector<int>> m{{1, 4, 7, 11, 15},  {2, 5, 8, 12, 19},
                        {3, 6, 9, 16, 22},  {10, 13, 14, 17, 24},
                        {18, 21, 23, 26, 30}};
  CHECK_TRUE(s.searchMatrix(m, 5));
  CHECK_FALSE(s.searchMatrix(m, 20));
  CHECK_TRUE(s.searchMatrix(m, 1));   // 左上角
  CHECK_TRUE(s.searchMatrix(m, 30));  // 右下角
  vector<vector<int>> one{{-5}};
  CHECK_TRUE(s.searchMatrix(one, -5));
  vector<vector<int>> none{};
  CHECK_FALSE(s.searchMatrix(none, 1));  // 空矩阵不越界
  CHECK_FALSE(s.searchMatrix(one, -2));
}
