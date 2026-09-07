// =============================================================================
//  09 · 图论 (Graph)
//  Q200 岛屿数量 · Q994 腐烂的橘子 · Q207 课程表 · Q208 实现 Trie（前缀树）
//
//  【网格就是图】
//  m×n 的网格里每个格子是结点，上下左右（有时含对角）是边。于是：
//    · 求连通块个数   -> DFS/BFS 把访问过的「沉掉」（Q200）
//    · 多源最短步数   -> 把所有源点同时入队做 BFS（Q994、Q127、Q130）
//    · 判有没有环     -> 拓扑排序：能出完 n 个点就无环（Q207）
//  注意 DFS 递归深度 = 连通块大小，300×300 全 1 的网格会到 9 万层，
//  真实工程里应改 BFS 或手写栈；面试口头说明这点即可。
// =============================================================================
#include <array>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "test.h"

using namespace std;

// -----------------------------------------------------------------------------
/*
 * Q200. 岛屿数量  Medium
 * https://leetcode.cn/problems/number-of-islands/
 *
 * 思路：扫到 '1' 就 ans++ 并把整座岛 DFS 沉成 '0'（洪水填充）。
 *      沉岛 =  visited 标记，避免同一座岛被重复计数。
 * 复杂度：时间 O(mn)，空间 O(mn)（最坏递归栈）
 * 延伸：不允许改原数组就另开 visited；要「动态加陆地后实时岛屿数」用并查集（Q305）。
 */
namespace lc0200 {
class Solution {
 public:
  int numIslands(vector<vector<char>>& grid) {
    int ans = 0;
    for (int i = 0; i < (int)grid.size(); ++i)
      for (int j = 0; j < (int)grid[0].size(); ++j)
        if (grid[i][j] == '1') {
          ++ans;
          sink(grid, i, j);
        }
    return ans;
  }

 private:
  static void sink(vector<vector<char>>& g, int i, int j) {
    if (i < 0 || j < 0 || i >= (int)g.size() || j >= (int)g[0].size() || g[i][j] != '1') return;
    g[i][j] = '0';  // 先标记再展开，防止来回递归
    sink(g, i + 1, j);
    sink(g, i - 1, j);
    sink(g, i, j + 1);
    sink(g, i, j - 1);
  }
};
}  // namespace lc0200

TEST(graph, q200_number_of_islands) {
  lc0200::Solution s;
  vector<vector<char>> a{{'1', '1', '1', '1', '0'},
                         {'1', '1', '0', '1', '0'},
                         {'1', '1', '0', '0', '0'},
                         {'0', '0', '0', '0', '0'}};
  CHECK_EQ(s.numIslands(a), 1);
  vector<vector<char>> b{{'1', '1', '0', '0', '0'},
                         {'1', '1', '0', '0', '0'},
                         {'0', '0', '1', '0', '0'},
                         {'0', '0', '0', '1', '1'}};
  CHECK_EQ(s.numIslands(b), 3);
  vector<vector<char>> c{{'0'}};
  CHECK_EQ(s.numIslands(c), 0);
}

// -----------------------------------------------------------------------------
/*
 * Q994. 腐烂的橘子  Medium
 * https://leetcode.cn/problems/rotting-oranges/
 *
 * 思路：多源 BFS。所有初始烂橘子同时入队，一层一层扩散 = 一分钟。
 *      用 fresh 计数收尾：结束时还有新鲜橘子 => 返回 -1。
 * 复杂度：时间 O(mn)，空间 O(mn)
 * 易错：① 一开始就没有新鲜橘子要返回 0（不是 -1）；
 *      ② 不多算最后一层的关键在循环条件里的 `&& fresh > 0`：新鲜橘子一烂完就退出，
 *        不会再进入下一层去 `++minutes`（只判 `!q.empty()` 的话，队尾那批烂橘子会再白扩一次）；
 *      ③ 空网格要提前返回，否则下面 `grid[0].size()` 就是越界。
 */
namespace lc0994 {
class Solution {
 public:
  int orangesRotting(vector<vector<int>>& grid) {
    if (grid.empty() || grid[0].empty()) return 0;  // 题面保证 m,n ≥ 1，这里防本地越界
    const int R = (int)grid.size(), C = (int)grid[0].size();
    queue<pair<int, int>> q;
    int fresh = 0;
    for (int i = 0; i < R; ++i)
      for (int j = 0; j < C; ++j) {
        if (grid[i][j] == 2)
          q.push({i, j});
        else if (grid[i][j] == 1)
          ++fresh;
      }
    if (fresh == 0) return 0;
    const int d[5] = {1, 0, -1, 0, 1};  // 用长度 5 的数组表示 4 个方向
    int minutes = 0;
    while (!q.empty() && fresh > 0) {
      ++minutes;
      int sz = (int)q.size();
      while (sz--) {
        auto [x, y] = q.front();
        q.pop();
        for (int k = 0; k < 4; ++k) {
          int nx = x + d[k], ny = y + d[k + 1];
          if (nx >= 0 && ny >= 0 && nx < R && ny < C && grid[nx][ny] == 1) {
            grid[nx][ny] = 2;
            --fresh;
            q.push({nx, ny});
          }
        }
      }
    }
    return fresh == 0 ? minutes : -1;
  }
};
}  // namespace lc0994

TEST(graph, q994_rotting_oranges) {
  lc0994::Solution s;
  vector<vector<int>> a{{2, 1, 1}, {1, 1, 0}, {0, 1, 1}};
  CHECK_EQ(s.orangesRotting(a), 4);
  vector<vector<int>> b{{2, 1, 1}, {0, 1, 1}, {1, 0, 1}};  // 有个橘子永远烂不到
  CHECK_EQ(s.orangesRotting(b), -1);
  vector<vector<int>> c{{0, 2}};  // 没有新鲜橘子
  CHECK_EQ(s.orangesRotting(c), 0);
  vector<vector<int>> d{{0, 0}};  // 一个橘子都没有
  CHECK_EQ(s.orangesRotting(d), 0);
  vector<vector<int>> e{};  // 空网格：不提前返回就是 grid[0] 越界
  CHECK_EQ(s.orangesRotting(e), 0);
}

// -----------------------------------------------------------------------------
/*
 * Q207. 课程表  Medium
 * https://leetcode.cn/problems/course-schedule/
 *
 * 思路：拓扑排序（Kahn）。先修关系建边 pre->cur，统计入度；
 *      每次取出入度为 0 的课。若能取完 n 门 -> 无环，否则存在环。
 *      「环」就是有门课程间接要求自己先修完自己。
 * 复杂度：时间 O(n+E)，空间 O(n+E)
 * 另解：DFS 三色标记（白/灰/黑），遇到灰色结点即成环 —— Q210 输出方案时更顺手。
 *      求「具体选课顺序」见 Q210，只要把出队顺序记下来。
 */
namespace lc0207 {
class Solution {
 public:
  bool canFinish(int numCourses, vector<vector<int>>& prerequisites) {
    vector<vector<int>> g(numCourses);
    vector<int> indeg(numCourses, 0);
    for (const auto& e : prerequisites) {  // e = [课程, 先修]
      g[e[1]].push_back(e[0]);
      ++indeg[e[0]];
    }
    queue<int> q;
    for (int i = 0; i < numCourses; ++i)
      if (indeg[i] == 0) q.push(i);
    int done = 0;
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      ++done;
      for (int v : g[u])
        if (--indeg[v] == 0) q.push(v);
    }
    return done == numCourses;
  }
};
}  // namespace lc0207

TEST(graph, q207_course_schedule) {
  lc0207::Solution s;
  vector<vector<int>> a{{1, 0}};
  CHECK_TRUE(s.canFinish(2, a));
  vector<vector<int>> b{{1, 0}, {0, 1}};  // 0<->1 互为先修
  CHECK_FALSE(s.canFinish(2, b));
  vector<vector<int>> c{{1, 0}, {2, 1}, {3, 2}, {2, 3}};  // 2-3-2 小环
  CHECK_FALSE(s.canFinish(4, c));
  vector<vector<int>> d{};
  CHECK_TRUE(s.canFinish(1, d));  // 没有先修关系
}

// -----------------------------------------------------------------------------
/*
 * Q208. 实现 Trie（前缀树）  Medium
 * https://leetcode.cn/problems/implement-trie-prefix-tree/
 *
 * 结构：每个结点存 26 个孩子指针 + 一个「结束标志」。
 *      search 走到底且 isEnd；startsWith 只要走到底 —— 两者的区别就是 end 标志。
 * 复杂度：三种操作都是 O(词长)；空间 O(所有前缀结点数)
 * 取舍：数组孩子表查询最快但费空间（26 指针/结点）；
 *      字符集大或稀疏时用 unordered_map 或「排序后二分」的孩子列表。
 * 应用：单词搜索 Q211/Q212、敏感词过滤、IP 路由最长前缀匹配。
 */
namespace lc0208 {
class Trie {
 public:
  Trie() = default;
  ~Trie() {
    for (Trie* c : kids_) delete c;
  }
  Trie(const Trie&) = delete;
  Trie& operator=(const Trie&) = delete;

  void insert(const string& word) {
    Trie* cur = this;
    for (char ch : word) {
      int i = ch - 'a';
      if (!cur->kids_[i]) cur->kids_[i] = new Trie();
      cur = cur->kids_[i];
    }
    cur->isEnd_ = true;
  }

  bool search(const string& word) const {
    const Trie* n = walk(word);
    return n && n->isEnd_;
  }

  bool startsWith(const string& prefix) const { return walk(prefix) != nullptr; }

 private:
  std::array<Trie*, 26> kids_{};  // {} -> 全部初始化为 nullptr
  bool isEnd_ = false;

  Trie* walk(const string& s) {
    Trie* cur = this;
    for (char ch : s) {
      int i = ch - 'a';
      if (!cur->kids_[i]) return nullptr;
      cur = cur->kids_[i];
    }
    return cur;
  }
  const Trie* walk(const string& s) const {
    const Trie* cur = this;
    for (char ch : s) {
      int i = ch - 'a';
      if (!cur->kids_[i]) return nullptr;
      cur = cur->kids_[i];
    }
    return cur;
  }
};
}  // namespace lc0208

TEST(graph, q208_implement_trie) {
  lc0208::Trie t;
  t.insert("apple");
  CHECK_TRUE(t.search("apple"));
  CHECK_FALSE(t.search("app"));         // 前缀存在，但不是完整词
  CHECK_TRUE(t.startsWith("app"));
  t.insert("app");
  CHECK_TRUE(t.search("app"));          // 插入后才成为完整词
  CHECK_FALSE(t.startsWith("ban"));
  CHECK_FALSE(t.search(""));            // 空串不是任何已插入的词
  CHECK_TRUE(t.startsWith(""));         // 但空前缀恒成立
  t.insert("banana");
  CHECK_TRUE(t.search("banana"));
  CHECK_FALSE(t.search("ban"));
}
