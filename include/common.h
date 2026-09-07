// =============================================================================
//  common.h —— LeetCode 数据结构与调试辅助
//
//  这里定义的 ListNode / TreeNode / Node 与 LeetCode 官方题解模板完全一致，
//  把 Solution 类复制进 LeetCode 提交框时，不需要任何改动。
//
//  本文件里的 buildList / buildTree 等函数**只是测试脚手架**，提交时删掉即可。
// =============================================================================
#pragma once

#include <cstdint>
#include <optional>
#include <queue>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// -----------------------------------------------------------------------------
// 1. LeetCode 标准节点定义
// -----------------------------------------------------------------------------

// 单链表节点（Q2 / Q19 / Q21 / Q23 / Q24 / Q25 / ...）
struct ListNode {
  int val;
  ListNode* next;
  ListNode(int x) : val(x), next(nullptr) {}
};

// 二叉树节点（Q94 / Q101 / Q102 / Q104 / Q105 / ...）
struct TreeNode {
  int val;
  TreeNode* left;
  TreeNode* right;
  TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
};

// 带随机指针的链表节点（Q138 随机链表的复制）
struct Node {
  int val;
  Node* next;
  Node* random;
  Node(int _val) : val(_val), next(nullptr), random(nullptr) {}
};

// -----------------------------------------------------------------------------
// 2. 链表：构造 / 导出 / 释放 / 造环
// -----------------------------------------------------------------------------

inline ListNode* buildList(const std::vector<int>& vals) {
  ListNode dummy(0);
  ListNode* tail = &dummy;
  for (int v : vals) {
    tail->next = new ListNode(v);
    tail = tail->next;
  }
  return dummy.next;
}

// 导出成 vector，尾部自动去掉哨兵；遇到环会安全停止
inline std::vector<int> listToVec(ListNode* head) {
  std::vector<int> out;
  int guard = 0;  // 防环：链表带环时也不会死循环
  while (head && guard++ < 1000) {
    out.push_back(head->val);
    head = head->next;
  }
  return out;
}

inline void freeList(ListNode* head) {
  while (head) {
    ListNode* nxt = head->next;
    delete head;
    head = nxt;
  }
}

// 把链表尾接到第 pos 个结点（0 起）上，pos < 0 表示不成环。返回入环点。
inline ListNode* makeCycle(ListNode* head, int pos) {
  if (!head || pos < 0) return nullptr;
  ListNode* tail = head;
  while (tail->next) tail = tail->next;
  ListNode* entry = head;
  for (int i = 0; i < pos && entry; ++i) entry = entry->next;
  tail->next = entry;
  return entry;
}

// 先断开「环内回边」（从环内任一结点走一圈找到 next == entry 的那个），再整条释放。
// 注意不能从 head 扫：entry 往往同时被「前驱结点」和「环尾」指向，会误拆错的边。
inline void freeListWithCycle(ListNode* head, ListNode* entry) {
  if (entry) {
    ListNode* p = entry;
    while (p->next && p->next != entry) p = p->next;
    p->next = nullptr;
  }
  freeList(head);
}

// 构造两条相交链表：返回 {a, b, intersectNode}；common 为公共后缀
inline std::vector<ListNode*> buildIntersect(const std::vector<int>& prefixA,
                                             const std::vector<int>& prefixB,
                                             const std::vector<int>& common) {
  ListNode* tail = buildList(common);
  ListNode* a = buildList(prefixA);
  ListNode* b = buildList(prefixB);
  if (a) { ListNode* t = a; while (t->next) t = t->next; t->next = tail; } else a = tail;
  if (b) { ListNode* t = b; while (t->next) t = t->next; t->next = tail; } else b = tail;
  return {a, b, tail};
}

// -----------------------------------------------------------------------------
// 3. 二叉树：层序构造 / 层序导出 / 释放
//    用 std::nullopt 表示空结点，例如 {1, 2, 3, nullopt, nullopt, 4, 5}
// -----------------------------------------------------------------------------

using OptVec = std::vector<std::optional<int>>;

inline OptVec none(int n) { return OptVec(n, std::nullopt); }

inline TreeNode* buildTree(const OptVec& vals) {
  if (vals.empty() || !vals[0].has_value()) return nullptr;
  TreeNode* root = new TreeNode(*vals[0]);
  std::queue<TreeNode*> q{{root}};
  size_t i = 1;
  while (i < vals.size() && !q.empty()) {
    TreeNode* cur = q.front();
    q.pop();
    if (i < vals.size()) {  // left
      if (vals[i].has_value()) {
        cur->left = new TreeNode(*vals[i]);
        q.push(cur->left);
      }
      ++i;
    }
    if (i < vals.size()) {  // right
      if (vals[i].has_value()) {
        cur->right = new TreeNode(*vals[i]);
        q.push(cur->right);
      }
      ++i;
    }
  }
  return root;
}

// 层序导出，尾部多余的空结点会被裁掉，便于和字面量比较
inline OptVec treeToVec(TreeNode* root) {
  OptVec out;
  if (!root) return out;
  std::queue<TreeNode*> q{{root}};
  while (!q.empty()) {
    TreeNode* cur = q.front();
    q.pop();
    if (cur) {
      out.push_back(cur->val);
      q.push(cur->left);
      q.push(cur->right);
    } else {
      out.push_back(std::nullopt);
    }
  }
  while (!out.empty() && !out.back().has_value()) out.pop_back();
  return out;
}

inline void freeTree(TreeNode* root) {
  if (!root) return;
  freeTree(root->left);
  freeTree(root->right);
  delete root;
}

// 按值查找结点（Q236 最近公共祖先、Q114 等用例需要拿到具体指针）
inline TreeNode* findNode(TreeNode* root, int val) {
  if (!root) return nullptr;
  if (root->val == val) return root;
  TreeNode* l = findNode(root->left, val);
  return l ? l : findNode(root->right, val);
}

// -----------------------------------------------------------------------------
// 4. 通用结果字符串化：给断言框架输出可读的 diff
//    注意：所有重载必须先声明再定义，否则模板内的递归调用找不到 vector 版本。
// -----------------------------------------------------------------------------

namespace lc {

template <class T>
std::string toStr(const T& v);
template <class T>
std::string toStr(const std::vector<T>& v);
template <class T>
std::string toStr(const std::optional<T>& v);
template <class A, class B>
std::string toStr(const std::pair<A, B>& p);
std::string toStr(bool v);
std::string toStr(const std::string& v);
std::string toStr(const char* v);

template <class T>
std::string toStr(const T& v) {
  std::ostringstream os;
  os << v;
  return os.str();
}

template <class T>
std::string toStr(const std::vector<T>& v) {
  std::string out = "[";
  for (size_t i = 0; i < v.size(); ++i) {
    if (i) out += ", ";
    out += toStr(v[i]);
  }
  return out + "]";
}

template <class T>
std::string toStr(const std::optional<T>& v) {
  return v.has_value() ? toStr(*v) : std::string("null");
}

template <class A, class B>
std::string toStr(const std::pair<A, B>& p) {
  return "(" + toStr(p.first) + ", " + toStr(p.second) + ")";
}

inline std::string toStr(bool v) { return v ? "true" : "false"; }
inline std::string toStr(const std::string& v) { return "\"" + v + "\""; }
inline std::string toStr(const char* v) { return std::string("\"") + v + "\""; }

}  // namespace lc
