// =============================================================================
//  08 · 二叉树 (Binary Tree)
//  Q94 中序遍历 · Q104 最大深度 · Q226 翻转二叉树 · Q101 对称二叉树 ·
//  Q543 二叉树的直径 · Q102 层序遍历 · Q108 有序数组转 BST · Q98 验证 BST ·
//  Q230 BST 第 K 小 · Q199 二叉树的右视图 · Q114 展开为链表 ·
//  Q105 前序+中序构造 · Q437 路径总和 III · Q236 最近公共祖先 ·
//  Q124 二叉树中的最大路径和
//
//  【递归的两种姿势 —— 想清楚这个，二叉树题就通了】
//  A. 「自顶向下」：参数里带上下文（路径和、上下界、前缀和表），
//     像前序遍历，进入结点时先处理携带的信息。例：Q113 路径总和、Q437。
//  B. 「自底向上」：返回值里带信息（高度、链贡献、最大贡献），
//     像后序遍历，用左右子树的返回值算出本结点的答案。例：Q104、Q543、Q124。
//  另外两条主线：
//     · BFS + 分层 for 循环 = 所有「按层」问题（Q102 层序、Q199 右视图、Q103 锯齿）。
//     · BST 的中序遍历严格递增 —— Q98 / Q230 / Q538 / Q700 都靠这一句话。
// =============================================================================
#include <algorithm>
#include <climits>
#include <queue>
#include <stack>
#include <unordered_map>
#include <vector>

#include "test.h"

using namespace std;

// -----------------------------------------------------------------------------
/*
 * Q94. 二叉树的中序遍历  Easy
 * https://leetcode.cn/problems/binary-tree-inorder-traversal/
 *
 * 迭代模板：一路向左压栈 -> 弹出访问 -> 转向右子树。
 *          这个模板稍加改造就是 Q98 验证 BST、Q230 第 K 小、Q173 迭代器。
 * Morris 遍历：利用「前驱结点的右指针为空」临时建线索，可做到 O(1) 空间。
 * 复杂度：时间 O(n)，空间 O(h)
 */
namespace lc0094 {
class Solution {
 public:
  static vector<int> inorderTraversal(TreeNode* root) {
    vector<int> ans;
    stack<TreeNode*> st;
    TreeNode* cur = root;
    while (cur || !st.empty()) {
      while (cur) {  // 一路向左
        st.push(cur);
        cur = cur->left;
      }
      cur = st.top();
      st.pop();
      ans.push_back(cur->val);  // 左根右：弹出时就是「根」
      cur = cur->right;
    }
    return ans;
  }

  static void inorderRec(TreeNode* root, vector<int>& out) {
    if (!root) return;
    inorderRec(root->left, out);
    out.push_back(root->val);
    inorderRec(root->right, out);
  }
};
}  // namespace lc0094

TEST(binary_tree, q94_inorder_traversal) {
  lc0094::Solution s;
  TreeNode* a = buildTree({1, nullopt, 2, 3});  // 3 是 2 的左孩子（层序序列化规则）
  CHECK_EQ(s.inorderTraversal(a), vector<int>{1, 3, 2});  // 官方示例输出就是 [1,3,2]
  vector<int> rec;
  s.inorderRec(a, rec);
  CHECK_EQ(rec, vector<int>{1, 3, 2});  // 递归版与迭代版结果必须一致
  freeTree(a);
  CHECK_EQ(s.inorderTraversal(nullptr), vector<int>{});
  TreeNode* b = buildTree({2, 1, 3});
  CHECK_EQ(s.inorderTraversal(b), vector<int>{1, 2, 3});
  freeTree(b);
}

// -----------------------------------------------------------------------------
/*
 * Q104. 二叉树的最大深度  Easy
 * https://leetcode.cn/problems/maximum-depth-of-binary-tree/
 *
 * 思路：自底向上。depth = 1 + max(左深, 右深)，空树为 0。
 * 复杂度：时间 O(n)，空间 O(h)（最坏退化成链表时 O(n)）
 */
namespace lc0104 {
class Solution {
 public:
  int maxDepth(TreeNode* root) {
    if (!root) return 0;
    return 1 + max(maxDepth(root->left), maxDepth(root->right));
  }
};
}  // namespace lc0104

TEST(binary_tree, q104_max_depth) {
  lc0104::Solution s;
  TreeNode* a = buildTree({3, 9, 20, nullopt, nullopt, 15, 7});
  CHECK_EQ(s.maxDepth(a), 3);
  freeTree(a);
  TreeNode* b = buildTree({1, nullopt, 2});  // 右斜链
  CHECK_EQ(s.maxDepth(b), 2);
  freeTree(b);
  CHECK_EQ(s.maxDepth(nullptr), 0);
}

// -----------------------------------------------------------------------------
/*
 * Q226. 翻转二叉树  Easy
 * https://leetcode.cn/problems/invert-binary-tree/
 *
 * 思路：每个结点交换左右孩子，递归下去。前序或后序都行（不像 Q101 需要成对比较）。
 * 复杂度：时间 O(n)，空间 O(h)
 */
namespace lc0226 {
class Solution {
 public:
  TreeNode* invertTree(TreeNode* root) {
    if (!root) return nullptr;
    std::swap(root->left, root->right);
    invertTree(root->left);
    invertTree(root->right);
    return root;
  }
};
}  // namespace lc0226

TEST(binary_tree, q226_invert_tree) {
  lc0226::Solution s;
  TreeNode* a = buildTree({4, 2, 7, 1, 3, 6, 9});
  TreeNode* inv = s.invertTree(a);
  CHECK_EQ(treeToVec(inv), OptVec{4, 7, 2, 9, 6, 3, 1});
  freeTree(inv);
  TreeNode* b = buildTree({2, 1, 3});
  CHECK_EQ(treeToVec(s.invertTree(b)), OptVec{2, 3, 1});
  freeTree(b);
}

// -----------------------------------------------------------------------------
/*
 * Q101. 对称二叉树  Easy
 * https://leetcode.cn/problems/symmetric-tree/
 *
 * 思路：对称不是「左右子树结构相同」，而是「互为镜像」：
 *      左.左 vs 右.右 与 左.右 vs 右.左 同时成立。
 *      所以必须写一个接收「两个结点」的递归函数 —— 这是镜像题的通用范式。
 * 复杂度：时间 O(n)，空间 O(h)
 */
namespace lc0101 {
class Solution {
 public:
  bool isSymmetric(TreeNode* root) { return !root || mirror(root->left, root->right); }

 private:
  static bool mirror(const TreeNode* a, const TreeNode* b) {
    if (!a && !b) return true;
    if (!a || !b) return false;             // 一边空一边不空
    return a->val == b->val &&             // 自身值
           mirror(a->left, b->right) &&    // 外侧配外侧
           mirror(a->right, b->left);      // 内侧配内侧
  }
};
}  // namespace lc0101

TEST(binary_tree, q101_symmetric_tree) {
  lc0101::Solution s;
  TreeNode* a = buildTree({1, 2, 2, 3, 4, 4, 3});
  CHECK_TRUE(s.isSymmetric(a));
  freeTree(a);
  TreeNode* b = buildTree({1, 2, 2, nullopt, 3, nullopt, 4});
  CHECK_FALSE(s.isSymmetric(b));
  freeTree(b);
  CHECK_TRUE(s.isSymmetric(nullptr));
}

// -----------------------------------------------------------------------------
/*
 * Q543. 二叉树的直径  Easy
 * https://leetcode.cn/problems/diameter-of-binary-tree/
 *
 * 思路：直径 = 某个结点的「左深 + 右深」（按边数）。
 *      递归函数只返回「高度」，直径用引用参数在沿途更新 —— 这就是
 *      「一个返回值不够用」时的标准处理方式，Q124 完全同构。
 * 复杂度：时间 O(n)，空间 O(h)
 * 易错：返回高度时是 1+max(l,r)，而更新答案时是 l+r（边数），别写成 l+r+1。
 */
namespace lc0543 {
class Solution {
 public:
  int diameterOfBinaryTree(TreeNode* root) {
    int best = 0;
    height(root, best);
    return best;
  }

 private:
  static int height(TreeNode* n, int& best) {
    if (!n) return 0;
    int l = height(n->left, best);
    int r = height(n->right, best);
    best = max(best, l + r);  // 经过 n 的最长路径（边数）
    return 1 + max(l, r);
  }
};
}  // namespace lc0543

TEST(binary_tree, q543_diameter) {
  lc0543::Solution s;
  TreeNode* a = buildTree({1, 2, 3, 4, 5});
  CHECK_EQ(s.diameterOfBinaryTree(a), 3);  // 4-2-1-3 或 5-2-1-3
  freeTree(a);
  TreeNode* b = buildTree({1, 2, nullopt, 3, nullopt, 4, nullopt, 5});  // 斜链
  CHECK_EQ(s.diameterOfBinaryTree(b), 4);
  freeTree(b);
  CHECK_EQ(s.diameterOfBinaryTree(nullptr), 0);
}

// -----------------------------------------------------------------------------
/*
 * Q102. 二叉树的层序遍历  Medium
 * https://leetcode.cn/problems/binary-tree-level-order-traversal/
 *
 * 思路：BFS + 「进入循环前先记下 q.size()」，这个 size 就是当前层的结点数，
 *      于是天然把层与层分开。Q199 右视图 / Q103 锯齿 / Q107 自底向上都改这一行。
 * 复杂度：时间 O(n)，空间 O(最宽一层) = O(n)
 */
namespace lc0102 {
class Solution {
 public:
  vector<vector<int>> levelOrder(TreeNode* root) {
    vector<vector<int>> ans;
    if (!root) return ans;
    queue<TreeNode*> q;
    q.push(root);
    while (!q.empty()) {
      int sz = (int)q.size();  // 本层结点数，关键一步
      ans.emplace_back();
      ans.back().reserve(sz);
      for (int i = 0; i < sz; ++i) {
        TreeNode* cur = q.front();
        q.pop();
        ans.back().push_back(cur->val);
        if (cur->left) q.push(cur->left);
        if (cur->right) q.push(cur->right);
      }
    }
    return ans;
  }
};
}  // namespace lc0102

TEST(binary_tree, q102_level_order) {
  lc0102::Solution s;
  TreeNode* a = buildTree({3, 9, 20, nullopt, nullopt, 15, 7});
  CHECK_EQ(s.levelOrder(a), vector<vector<int>>({{3}, {9, 20}, {15, 7}}));
  freeTree(a);
  CHECK_EQ(s.levelOrder(nullptr), vector<vector<int>>{});
  TreeNode* b = buildTree({1});
  CHECK_EQ(s.levelOrder(b), vector<vector<int>>({{1}}));
  freeTree(b);
}

// -----------------------------------------------------------------------------
/*
 * Q108. 将有序数组转换为二叉搜索树  Easy
 * https://leetcode.cn/problems/convert-sorted-array-to-binary-search-tree/
 *
 * 思路：取中间元素做根，左右两半递归建子树 —— 天然高度平衡（AVL 式建法）。
 * 复杂度：时间 O(n)，空间 O(log n)
 * 说明：答案不唯一（偶数长度时选中位数左/右都可以），所以测试断言的是
 *      「性质」而不是「形状」：中序遍历等于原数组 + 判定为合法 BST。
 *      为了这个断言，本题的 TEST 写到了 Q98 后面（需要复用它的 isValidBST）。
 */
namespace lc0108 {
class Solution {
 public:
  TreeNode* sortedArrayToBST(vector<int>& nums) { return build(nums, 0, (int)nums.size() - 1); }

 private:
  static TreeNode* build(const vector<int>& a, int l, int r) {
    if (l > r) return nullptr;
    int mid = l + (r - l) / 2;  // 防溢出写法，面试常问
    TreeNode* root = new TreeNode(a[mid]);
    root->left = build(a, l, mid - 1);
    root->right = build(a, mid + 1, r);
    return root;
  }
};
}  // namespace lc0108

// -----------------------------------------------------------------------------
/*
 * Q98. 验证二叉搜索树  Medium
 * https://leetcode.cn/problems/validate-binary-search-tree/
 *
 * 思路：自顶向下传递开区间 (lo, hi)。走左分支收紧上界为父值，走右分支收紧下界。
 * 致命错误：只比较「结点与左右孩子」—— 那只能保证局部有序。
 *          真正的反例是 [10,5,15,nullopt,nullopt,6,20]：每个结点都比左孩子大、比右孩子小，
 *          但 6 落在 10 的右子树里，就必须 > 10 —— 只有「自上而下传区间」能拒掉它。
 *          （官方给的另一例 [5,1,4,nullopt,nullopt,3,6] 反而简单 —— 4 是 5 的右孩子却比 5 小，
 *          「只比父子」一眼就能拒掉，所以它并不能暴露这个 bug）
 * 用 long long 是因为边界要能表示 INT_MIN-1 / INT_MAX+1。
 * 复杂度：时间 O(n)，空间 O(h)
 */
namespace lc0098 {
class Solution {
 public:
  bool isValidBST(TreeNode* root) { return valid(root, LLONG_MIN, LLONG_MAX); }

 private:
  static bool valid(TreeNode* n, long long lo, long long hi) {
    if (!n) return true;
    if (n->val <= lo || n->val >= hi) return false;  // 严格小于/大于
    return valid(n->left, lo, n->val) && valid(n->right, n->val, hi);
  }
};
}  // namespace lc0098

TEST(binary_tree, q98_validate_bst) {
  lc0098::Solution s;
  TreeNode* a = buildTree({2, 1, 3});
  CHECK_TRUE(s.isValidBST(a));
  freeTree(a);
  TreeNode* b = buildTree({5, 1, 4, nullopt, nullopt, 3, 6});  // 4 是 5 的右孩子却比 5 小
  CHECK_FALSE(s.isValidBST(b));
  freeTree(b);
  TreeNode* c = buildTree({1, nullopt, 1});  // 相等也不允许
  CHECK_FALSE(s.isValidBST(c));
  freeTree(c);
  // 局部都合法、全局不合法：只比较父子结点的写法会在这一条上返回 true
  TreeNode* d = buildTree({10, 5, 15, nullopt, nullopt, 6, 20});
  CHECK_FALSE(s.isValidBST(d));
  freeTree(d);
  TreeNode* e = buildTree({INT_MIN});
  CHECK_TRUE(s.isValidBST(e));  // 极值：用 int 边界会挂
  freeTree(e);
}

// Q108 的用例：放在这里是因为要用上面的 isValidBST 做「性质断言」
TEST(binary_tree, q108_array_to_bst) {
  lc0108::Solution s;
  vector<int> a{-10, -3, 0, 5, 9};
  TreeNode* t = s.sortedArrayToBST(a);
  CHECK_EQ(lc0094::Solution::inorderTraversal(t), a);  // 中序 == 有序数组
  lc0098::Solution v;                                  // 借用 Q98 交叉验证
  CHECK_TRUE(v.isValidBST(t));
  lc0104::Solution d;
  CHECK_EQ(d.maxDepth(t), 3);  // 5 个结点的平衡树高度 3
  freeTree(t);
}

// -----------------------------------------------------------------------------
/*
 * Q230. 二叉搜索树中第 K 小的元素  Medium
 * https://leetcode.cn/problems/kth-smallest-element-in-a-bst/
 *
 * 思路：BST 中序遍历第 k 个即答案。用 Q94 的迭代模板，数到 k 直接返回，
 *      不用遍历完整棵树 —— 这是它比「中序数组化」强的地方。
 * 复杂度：时间 O(h + k)，空间 O(h)
 * 延伸：若频繁调用且树会变动，应在结点里维护「子树大小」字段（Q776 思路）。
 */
namespace lc0230 {
class Solution {
 public:
  int kthSmallest(TreeNode* root, int k) {
    stack<TreeNode*> st;
    TreeNode* cur = root;
    while (cur || !st.empty()) {
      while (cur) {
        st.push(cur);
        cur = cur->left;
      }
      cur = st.top();
      st.pop();
      if (--k == 0) return cur->val;  // 第 k 个被访问的结点
      cur = cur->right;
    }
    return -1;  // 题目保证 k 合法
  }
};
}  // namespace lc0230

TEST(binary_tree, q230_kth_smallest) {
  lc0230::Solution s;
  TreeNode* a = buildTree({3, 1, 4, nullopt, 2});
  CHECK_EQ(s.kthSmallest(a, 1), 1);
  CHECK_EQ(s.kthSmallest(a, 3), 3);
  freeTree(a);
  TreeNode* b = buildTree({5, 3, 6, 2, 4, nullopt, nullopt, 1});
  CHECK_EQ(s.kthSmallest(b, 3), 3);
  freeTree(b);
}

// -----------------------------------------------------------------------------
/*
 * Q199. 二叉树的右视图  Medium
 * https://leetcode.cn/problems/binary-tree-right-side-view/
 *
 * 思路：层序遍历，每层取最后一个。
 *      也可用 DFS（根->右->左 的顺序，第一次到达某深度时记录），
 *      两种写法都是高频追问点。
 * 复杂度：时间 O(n)，空间 O(宽度)
 */
namespace lc0199 {
class Solution {
 public:
  vector<int> rightSideView(TreeNode* root) {
    vector<int> ans;
    if (!root) return ans;
    queue<TreeNode*> q{{root}};
    while (!q.empty()) {
      int sz = (int)q.size();
      for (int i = 0; i < sz; ++i) {
        TreeNode* cur = q.front();
        q.pop();
        if (i == sz - 1) ans.push_back(cur->val);  // 本层最右
        if (cur->left) q.push(cur->left);
        if (cur->right) q.push(cur->right);
      }
    }
    return ans;
  }
};
}  // namespace lc0199

TEST(binary_tree, q199_right_side_view) {
  lc0199::Solution s;
  TreeNode* a = buildTree({1, 2, 3, nullopt, 5, nullopt, 4});
  CHECK_EQ(s.rightSideView(a), vector<int>{1, 3, 4});
  freeTree(a);
  TreeNode* b = buildTree({1, 2, 3, nullopt, 5});  // 右侧缺，取到 5
  CHECK_EQ(s.rightSideView(b), vector<int>{1, 3, 5});
  freeTree(b);
  CHECK_EQ(s.rightSideView(nullptr), vector<int>{});
}

// -----------------------------------------------------------------------------
/*
 * Q114. 二叉树展开为链表  Medium
 * https://leetcode.cn/problems/flatten-binary-tree-to-linked-list/
 *
 * 思路（原地，O(1) 空间）：对每个有左子树的结点，找到「左子树最右结点」（前驱），
 *      把当前右子树挂到前驱右边，再把左子树整体搬右，左置空，然后下移。
 *      这本质是 Morris 遍历的变形。
 * 另解：反后序（右->左->根）+ 一个 prev 指针，一行一行地接，也很好记。
 * 复杂度：时间 O(n)，空间 O(1)
 */
namespace lc0114 {
class Solution {
 public:
  void flatten(TreeNode* root) {
    TreeNode* cur = root;
    while (cur) {
      if (cur->left) {
        TreeNode* pred = cur->left;
        while (pred->right) pred = pred->right;  // 前驱 = 左子树最右
        pred->right = cur->right;
        cur->right = cur->left;
        cur->left = nullptr;
      }
      cur = cur->right;
    }
  }

  // 测试辅助：沿 right 走一遍取值，同时确认 left 全空
  static vector<int> rightSpine(TreeNode* root, bool& allLeftEmpty) {
    vector<int> out;
    allLeftEmpty = true;
    for (TreeNode* p = root; p; p = p->right) {
      if (p->left) allLeftEmpty = false;
      out.push_back(p->val);
    }
    return out;
  }
};
}  // namespace lc0114

TEST(binary_tree, q114_flatten) {
  lc0114::Solution s;
  TreeNode* a = buildTree({1, 2, 5, 3, 4, nullopt, 6});
  s.flatten(a);
  bool leftEmpty = false;
  CHECK_EQ(s.rightSpine(a, leftEmpty), vector<int>{1, 2, 3, 4, 5, 6});
  CHECK_TRUE(leftEmpty);
  freeTree(a);
  TreeNode* b = buildTree({});
  bool e2 = true;
  CHECK_EQ(s.rightSpine(b, e2), vector<int>{});
  TreeNode* c = buildTree({0});
  s.flatten(c);
  bool e3 = false;
  CHECK_EQ(s.rightSpine(c, e3), vector<int>{0});
  CHECK_TRUE(e3);
  freeTree(c);
}

// -----------------------------------------------------------------------------
/*
 * Q105. 从前序与中序遍历序列构造二叉树  Medium
 * https://leetcode.cn/problems/construct-binary-tree-from-preorder-and-inorder-traversal/
 *
 * 思路：pre[pl] 一定是当前子树的根；在 inorder 里查到它的位置 k，
 *      [inL, k) 是左子树、(k, inR] 是右子树，左子树结点数 = k - inL，
 *      据此切分 preorder 区间递归。用哈希表存「值 -> 中序下标」把查找降到 O(1)。
 * 复杂度：时间 O(n)，空间 O(n)
 * 易错：右子树的 preorder 起点是 pl + (k - inL) + 1，不是 pl + 1。
 *      前提：结点值互不相同（题目保证），否则无法唯一定位。
 * 对照：Q106（中序+后序）对称 —— 后序的根在数组末尾。
 */
namespace lc0105 {
class Solution {
 public:
  TreeNode* buildTree(vector<int>& preorder, vector<int>& inorder) {
    for (int i = 0; i < (int)inorder.size(); ++i) pos_[inorder[i]] = i;
    return build(preorder, 0, (int)preorder.size() - 1, inorder, 0, (int)inorder.size() - 1);
  }

 private:
  unordered_map<int, int> pos_;  // 值 -> 中序下标

  TreeNode* build(const vector<int>& pre, int pl, int pr, const vector<int>& in, int il,
                  int ir) {
    if (pl > pr) return nullptr;
    int rootVal = pre[pl];
    int k = pos_[rootVal];            // 根在中序里的位置
    int leftSize = k - il;            // 左子树结点个数
    TreeNode* root = new TreeNode(rootVal);
    root->left = build(pre, pl + 1, pl + leftSize, in, il, k - 1);
    root->right = build(pre, pl + leftSize + 1, pr, in, k + 1, ir);
    return root;
  }
};
}  // namespace lc0105

TEST(binary_tree, q105_build_from_pre_in) {
  lc0105::Solution s;
  vector<int> pre{3, 9, 20, 15, 7};
  vector<int> in{9, 3, 15, 20, 7};
  TreeNode* t = s.buildTree(pre, in);
  CHECK_EQ(treeToVec(t), OptVec{3, 9, 20, nullopt, nullopt, 15, 7});
  freeTree(t);
  vector<int> pre2{-1};
  vector<int> in2{-1};
  TreeNode* u = s.buildTree(pre2, in2);
  CHECK_EQ(treeToVec(u), OptVec{-1});
  freeTree(u);
}

// -----------------------------------------------------------------------------
/*
 * Q437. 路径总和 III  Medium
 * https://leetcode.cn/problems/path-sum-iii/
 *
 * 思路：路径必须「向下」但起终点任意 -> 等价于树上任意一条根向链上的子段和，
 *      于是套用 Q560：DFS 维护根到当前的前缀和 cur，查有多少祖先的前缀和 = cur-target。
 *      回溯时必须 cnt[cur]--（离开这条分支要把状态还回去），这是树上前缀和的命门。
 * 复杂度：时间 O(n)，空间 O(n)
 * 对比：双重 DFS（每个结点都当一次起点）是 O(n^2)。若结点值全为非负，可以在
 *      「向下累积和已超过 target」时剪枝；但本题允许 -1000 ≤ val ≤ 1000，
 *      往下走不保证和变大，所以必须一路探到底 —— 这才是它需要前缀和的原因。
 */
namespace lc0437 {
class Solution {
 public:
  int pathSum(TreeNode* root, int targetSum) {
    unordered_map<long long, int> cnt;
    cnt[0] = 1;  // 空前缀：从根出发的路径
    return dfs(root, 0, (long long)targetSum, cnt);
  }

 private:
  static int dfs(TreeNode* n, long long cur, long long target,
                 unordered_map<long long, int>& cnt) {
    if (!n) return 0;
    cur += n->val;
    int ans = cnt[cur - target];       // 以 n 为终点、起点是某祖先的路径条数
    ++cnt[cur];
    ans += dfs(n->left, cur, target, cnt);
    ans += dfs(n->right, cur, target, cnt);
    --cnt[cur];                        // 回溯！兄弟分支不该看到这条路径的前缀和
    return ans;
  }
};
}  // namespace lc0437

TEST(binary_tree, q437_path_sum_iii) {
  lc0437::Solution s;
  TreeNode* a = buildTree({10, 5, -3, 3, 2, nullopt, 11, 3, -2, nullopt, 1});
  CHECK_EQ(s.pathSum(a, 8), 3);  // 5->3, 5->2->1, -3->11
  TreeNode* b = buildTree({5, 4, 8, 11, nullopt, 13, 4, 7, 2, nullopt, nullopt, nullopt, 1});
  // 注意路径起点不必是根：5->4->11->2 与 4->11->7 都算，共 2 条
  // （同一棵树用 Q112/113「根到叶」只会有 1 条，这就是两者的区别）
  CHECK_EQ(s.pathSum(b, 22), 2);
  CHECK_EQ(s.pathSum(nullptr, 0), 0);
  freeTree(a);
  freeTree(b);
}

// -----------------------------------------------------------------------------
/*
 * Q236. 二叉树的最近公共祖先  Medium
 * https://leetcode.cn/problems/lowest-common-ancestor-of-a-binary-tree/
 *
 * 返回值语义（背下来，一类题全通）：
 *   · 本结点为 null 或等于 p/q -> 直接返回本结点
 *   · 左右都返回非空 -> p、q 分居两侧，本结点就是 LCA
 *   · 只有一侧非空 -> 把那一侧的结果原样上传
 * 注意：这题不是 BST！不能靠大小比较剪枝（那是 Q235）。
 * 复杂度：时间 O(n)，空间 O(h)
 */
namespace lc0236 {
class Solution {
 public:
  TreeNode* lowestCommonAncestor(TreeNode* root, TreeNode* p, TreeNode* q) {
    if (!root || root == p || root == q) return root;
    TreeNode* left = lowestCommonAncestor(root->left, p, q);
    TreeNode* right = lowestCommonAncestor(root->right, p, q);
    if (left && right) return root;  // 分叉点
    return left ? left : right;
  }
};
}  // namespace lc0236

TEST(binary_tree, q236_lca) {
  lc0236::Solution s;
  TreeNode* t = buildTree({3, 5, 1, 6, 2, 0, 8, nullopt, nullopt, 7, 4});
  CHECK_EQ(s.lowestCommonAncestor(t, findNode(t, 5), findNode(t, 1)), findNode(t, 3));
  // p 是 q 的祖先时，LCA 就是 p 本身（本题允许这种输入）
  CHECK_EQ(s.lowestCommonAncestor(t, findNode(t, 5), findNode(t, 4)), findNode(t, 5));
  CHECK_EQ(s.lowestCommonAncestor(t, findNode(t, 7), findNode(t, 6)), findNode(t, 5));
  CHECK_EQ(s.lowestCommonAncestor(t, findNode(t, 0), findNode(t, 8)), findNode(t, 1));
  freeTree(t);
}

// -----------------------------------------------------------------------------
/*
 * Q124. 二叉树中的最大路径和  Hard
 * https://leetcode.cn/problems/binary-tree-maximum-path-sum/
 *
 * 区分两个量（本题的难点全在这里）：
 *   · 链贡献 gain(n)：从 n 出发还能往「上」延伸给父亲用，只能二选一，
 *     所以是 n->val + max(左, 右)，并且负贡献要截断为 0。
 *   · 路径答案：在 n 处「拐弯」，左 + n + 右，只能在此刻更新全局答案，无法上传。
 * 全负树：答案必须是最大的那个负数，所以初值是 INT_MIN 而不是 0。
 * 复杂度：时间 O(n)，空间 O(h)
 */
namespace lc0124 {
class Solution {
 public:
  int maxPathSum(TreeNode* root) {
    int best = INT_MIN;
    gain(root, best);
    return best;
  }

 private:
  static int gain(TreeNode* n, int& best) {
    if (!n) return 0;
    int l = max(0, gain(n->left, best));   // 负收益的子树直接不要
    int r = max(0, gain(n->right, best));
    best = max(best, l + r + n->val);      // 在 n 处拐弯的完整路径
    return n->val + max(l, r);             // 只能带一条边上传
  }
};
}  // namespace lc0124

TEST(binary_tree, q124_max_path_sum) {
  lc0124::Solution s;
  TreeNode* a = buildTree({-10, 9, 20, nullopt, nullopt, 15, 7});
  CHECK_EQ(s.maxPathSum(a), 42);  // 15 -> 20 -> 7
  freeTree(a);
  TreeNode* b = buildTree({1, 2, 3});
  CHECK_EQ(s.maxPathSum(b), 6);
  freeTree(b);
  TreeNode* c = buildTree({-3});  // 全负
  CHECK_EQ(s.maxPathSum(c), -3);
  freeTree(c);
  TreeNode* d = buildTree({2, -1});
  CHECK_EQ(s.maxPathSum(d), 2);  // 宁可只取根
  freeTree(d);
}
