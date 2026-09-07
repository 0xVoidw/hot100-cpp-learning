// =============================================================================
//  07 · 链表 (Linked List)
//  Q160 相交链表 · Q206 反转链表 · Q234 回文链表 · Q141 环形链表 ·
//  Q142 环形链表 II · Q21 合并两个有序链表 · Q2 两数相加 ·
//  Q19 删除倒数第 N 个结点 · Q24 两两交换 · Q25 K 个一组翻转 ·
//  Q138 随机链表的复制 · Q148 排序链表 · Q23 合并 K 个升序链表 · Q146 LRU 缓存
//
//  【链表通用心法】
//  1) 哨兵结点 dummy：凡是「头结点可能被删/被换」的题，一律从 dummy 出发，
//     最后 return dummy.next。能消掉 90% 的头结点特判。
//  2) 快慢指针三件套：找中点、找倒数第 k、判环。
//  3) 反转是基本功：prev / cur / nxt 三个指针，先存 nxt 再改链。
//  4) 判环的数学：设 head→入口 = a，入口→相遇点 = b，相遇点→入口 = c，环长 L = b + c。
//     相遇时 slow 走了 a+b，fast 走了 a+b+kL，而 fast = 2·slow ⇒ a+b 是 L 的整数倍，
//     所以 a ≡ c (mod L) —— 一个指针回到 head、两个各走 1 步，必在入口相遇（推导见 Q142）。
//  5) 测试里造过环的链表不好统一释放，本文件部分用例故意让进程退出时回收。
// =============================================================================
#include <algorithm>
#include <functional>
#include <list>
#include <queue>
#include <unordered_map>
#include <vector>

#include "test.h"

using namespace std;

// -----------------------------------------------------------------------------
/*
 * Q160. 相交链表  Easy
 * https://leetcode.cn/problems/intersection-of-two-linked-lists/
 *
 * 思路：双指针各自走完自己的链后切换到对方的头。两人都走了 a+b 与 b+a 步，
 *      路程相等 -> 若相交必在交点相遇；不相交则同时在 nullptr 相遇。
 * 复杂度：时间 O(m+n)，空间 O(1)  —— 比「先算长度差再对齐」的写法更短
 */
namespace lc0160 {
class Solution {
 public:
  ListNode* getIntersectionNode(ListNode* headA, ListNode* headB) {
    if (!headA || !headB) return nullptr;
    ListNode *p = headA, *q = headB;
    while (p != q) {
      p = p ? p->next : headB;  // 走完自己就换到对方头上
      q = q ? q->next : headA;
    }
    return p;  // 交点，或者 nullptr
  }
};
}  // namespace lc0160

TEST(linked_list, q160_intersection) {
  lc0160::Solution s;
  auto abc = buildIntersect({4, 1}, {5, 6, 1}, {8, 4, 5});
  CHECK_EQ(s.getIntersectionNode(abc[0], abc[1]), abc[2]);  // 指针相等，不只是值相等
  auto noTouch = buildIntersect({2, 6, 4}, {1, 5}, {});     // 公共后缀为空 -> 不相交
  CHECK_TRUE(s.getIntersectionNode(noTouch[0], noTouch[1]) == nullptr);
  auto onlyCommon = buildIntersect({}, {}, {9});            // 完全共用一条链
  CHECK_EQ(s.getIntersectionNode(onlyCommon[0], onlyCommon[1]), onlyCommon[2]);
}

// -----------------------------------------------------------------------------
/*
 * Q206. 反转链表  Easy
 * https://leetcode.cn/problems/reverse-linked-list/
 *
 * 迭代：prev/cur/nxt 三指针，是几乎所有复杂链表题（Q25、Q92）的子过程。
 * 递归：先递归反转后段，再把 head->next->next 指回 head，最后断开 head->next。
 * 复杂度：都是时间 O(n)；迭代空间 O(1)，递归 O(n) 栈
 */
namespace lc0206 {
class Solution {
 public:
  static ListNode* reverseIterative(ListNode* head) {
    ListNode* prev = nullptr;
    while (head) {
      ListNode* nxt = head->next;  // 先存，否则改链后丢失后继
      head->next = prev;
      prev = head;
      head = nxt;
    }
    return prev;
  }

  static ListNode* reverseRecursive(ListNode* head) {
    if (!head || !head->next) return head;
    ListNode* newHead = reverseRecursive(head->next);
    head->next->next = head;  // 让后继指回自己
    head->next = nullptr;     // 断开原来的正向边（临时尾结点）
    return newHead;
  }
};
}  // namespace lc0206

TEST(linked_list, q206_reverse_list) {
  lc0206::Solution s;
  vector<int> vals{1, 2, 3, 4, 5};
  ListNode* a = buildList(vals);
  CHECK_EQ(listToVec(s.reverseIterative(a)), vector<int>{5, 4, 3, 2, 1});
  freeList(a);
  ListNode* b = buildList(vals);
  CHECK_EQ(listToVec(s.reverseRecursive(b)), vector<int>{5, 4, 3, 2, 1});
  freeList(b);
  CHECK_TRUE(s.reverseIterative(nullptr) == nullptr);
  ListNode* c = buildList({7});
  CHECK_EQ(listToVec(s.reverseIterative(c)), vector<int>{7});
  freeList(c);
}

// -----------------------------------------------------------------------------
/*
 * Q234. 回文链表  Easy
 * https://leetcode.cn/problems/palindrome-linked-list/
 *
 * 思路：快慢指针找中点 -> 反转后半段 -> 双向比对 -> 还原链表。
 *      O(1) 空间的唯一正解；数组化 O(n)  space 是备用答案。
 *      偶数长度时 slow 落在后半段起点，比较时用 q 走完为止，不必管长度。
 * 复杂度：时间 O(n)，空间 O(1)
 * 加分项：还原链表（不破坏输入），面试官会认为你「有工程意识」。
 */
namespace lc0234 {
class Solution {
 public:
  bool isPalindrome(ListNode* head) {
    if (!head || !head->next) return true;
    ListNode *slow = head, *fast = head;
    while (fast && fast->next) {  // 定位后半段起点
      slow = slow->next;
      fast = fast->next->next;
    }
    auto reverseFrom = [](ListNode* p) {
      ListNode* prev = nullptr;
      while (p) {
        ListNode* n = p->next;
        p->next = prev;
        prev = p;
        p = n;
      }
      return prev;
    };
    ListNode* second = reverseFrom(slow);
    bool ok = true;
    for (ListNode *p = head, *q = second; q; p = p->next, q = q->next)
      if (p->val != q->val) {
        ok = false;
        break;
      }
    reverseFrom(second);  // 还原，恢复入参原貌
    return ok;
  }
};
}  // namespace lc0234

TEST(linked_list, q234_palindrome_list) {
  lc0234::Solution s;
  ListNode* a = buildList({1, 2, 2, 1});
  CHECK_TRUE(s.isPalindrome(a));
  CHECK_EQ(listToVec(a), vector<int>{1, 2, 2, 1});  // 验证已被还原
  freeList(a);
  ListNode* b = buildList({1, 2});
  CHECK_FALSE(s.isPalindrome(b));
  freeList(b);
  ListNode* c = buildList({1, 2, 1});  // 奇数长度
  CHECK_TRUE(s.isPalindrome(c));
  CHECK_EQ(listToVec(c), vector<int>{1, 2, 1});
  freeList(c);
}

// -----------------------------------------------------------------------------
/*
 * Q141 / Q142. 环形链表 I & II  Easy / Medium
 * https://leetcode.cn/problems/linked-list-cycle/
 * https://leetcode.cn/problems/linked-list-cycle-ii/
 *
 * 判环：Floyd 快慢指针，快每次 2 步，有环必相遇。
 * 找入口：设头到入口距离 a，入口到相遇点 b，相遇点回到入口 c。
 *        快走的距离 = 2(a+b) = a+b+k(b+c)  =>  a = (k-1)(b+c) + c
 *        所以「从头走」和「从相遇点走」会在入口相遇。
 * 复杂度：时间 O(n)，空间 O(1)；哈希表法 O(n) 空间，不作为首选。
 */
namespace lc0141 {
class Solution {
 public:
  bool hasCycle(ListNode* head) {
    ListNode *slow = head, *fast = head;
    while (fast && fast->next) {
      slow = slow->next;
      fast = fast->next->next;
      if (slow == fast) return true;
    }
    return false;
  }
};
}  // namespace lc0141

namespace lc0142 {
class Solution {
 public:
  ListNode* detectCycle(ListNode* head) {
    ListNode *slow = head, *fast = head;
    while (fast && fast->next) {
      slow = slow->next;
      fast = fast->next->next;
      if (slow == fast) {           // 相遇，开始在环里「对表」
        slow = head;
        while (slow != fast) {
          slow = slow->next;
          fast = fast->next;        // 这一步两人都走 1 步
        }
        return slow;                // 两指针同步停在入口；无环的情况由下面的 return nullptr 处理
      }
    }
    return nullptr;
  }
};
}  // namespace lc0142

TEST(linked_list, q141_142_cycle) {
  lc0141::Solution s1;
  lc0142::Solution s2;
  ListNode* a = buildList({3, 2, 0, -4});
  ListNode* entry = makeCycle(a, 1);  // 尾 -> 第 1 个结点(值 2)
  CHECK_TRUE(s1.hasCycle(a));
  CHECK_EQ(s2.detectCycle(a), entry);  // 必须是指针相同
  freeListWithCycle(a, entry);         // 先拆环再释放

  ListNode* b = buildList({1, 2});
  CHECK_FALSE(s1.hasCycle(b));
  CHECK_TRUE(s2.detectCycle(b) == nullptr);
  freeList(b);

  ListNode* c = buildList({1});
  ListNode* selfEntry = makeCycle(c, 0);  // 自环
  CHECK_TRUE(s1.hasCycle(c));
  CHECK_EQ(s2.detectCycle(c), selfEntry);
  freeListWithCycle(c, selfEntry);
}

// -----------------------------------------------------------------------------
/*
 * Q21. 合并两个有序链表  Easy
 * https://leetcode.cn/problems/merge-two-sorted-lists/
 *
 * 思路：哨兵 + 每次挑较小的头结点接上。归并排序的 merge 步骤、Q23 的基础件。
 * 复杂度：时间 O(m+n)，空间 O(1)
 */
namespace lc0021 {
class Solution {
 public:
  ListNode* mergeTwoLists(ListNode* l1, ListNode* l2) {
    ListNode dummy(0);
    ListNode* tail = &dummy;
    while (l1 && l2) {
      if (l1->val <= l2->val) {
        tail->next = l1;
        l1 = l1->next;
      } else {
        tail->next = l2;
        l2 = l2->next;
      }
      tail = tail->next;
    }
    tail->next = l1 ? l1 : l2;  // 剩下的整段直接接上
    return dummy.next;
  }
};
}  // namespace lc0021

TEST(linked_list, q21_merge_two_sorted) {
  lc0021::Solution s;
  ListNode* a = buildList({1, 2, 4});
  ListNode* b = buildList({1, 3, 4});
  CHECK_EQ(listToVec(s.mergeTwoLists(a, b)), vector<int>{1, 1, 2, 3, 4, 4});
  ListNode* c = buildList({});
  ListNode* d = buildList({});
  CHECK_TRUE(s.mergeTwoLists(c, d) == nullptr);
  ListNode* e = buildList({5});
  CHECK_EQ(listToVec(s.mergeTwoLists(nullptr, e)), vector<int>{5});
}

// -----------------------------------------------------------------------------
/*
 * Q2. 两数相加  Medium
 * https://leetcode.cn/problems/add-two-numbers/
 *
 * 思路：竖式加法。链表已经是低位在前，正好顺序遍历。
 *      循环条件用 `carry || (l1 || l2)`，否则 [9,9] + [1] 会丢掉最高位的进位。
 * 复杂度：时间 O(max(m,n))，空间 O(1)（输出不计）
 * 对比：Q445 逆序/正序之分就在于要不要先反转或借助栈。
 */
namespace lc0002 {
class Solution {
 public:
  ListNode* addTwoNumbers(ListNode* l1, ListNode* l2) {
    ListNode dummy(0);
    ListNode* tail = &dummy;
    int carry = 0;
    while (l1 || l2 || carry) {
      int sum = carry + (l1 ? l1->val : 0) + (l2 ? l2->val : 0);
      carry = sum / 10;
      tail->next = new ListNode(sum % 10);
      tail = tail->next;
      if (l1) l1 = l1->next;
      if (l2) l2 = l2->next;
    }
    return dummy.next;
  }
};
}  // namespace lc0002

TEST(linked_list, q2_add_two_numbers) {
  lc0002::Solution s;
  ListNode* a = buildList({2, 4, 3});
  ListNode* b = buildList({5, 6, 4});
  CHECK_EQ(listToVec(s.addTwoNumbers(a, b)), vector<int>{7, 0, 8});  // 342+465=807
  ListNode* c = buildList({9, 9, 9, 9, 9, 9, 9});
  ListNode* d = buildList({9, 9, 9, 9});
  CHECK_EQ(listToVec(s.addTwoNumbers(c, d)), vector<int>{8, 9, 9, 9, 0, 0, 0, 1});
  ListNode* e = buildList({0});
  ListNode* f = buildList({0});
  CHECK_EQ(listToVec(s.addTwoNumbers(e, f)), vector<int>{0});
}

// -----------------------------------------------------------------------------
/*
 * Q19. 删除链表的倒数第 N 个结点  Medium
 * https://leetcode.cn/problems/remove-nth-node-from-end-of-list/
 *
 * 思路：fast 先走 n 步，然后两人同速前进；fast 到末尾时 slow 正好停在待删点前驱。
 *      有哨兵才能统一处理「删头结点」（倒数第 n 个正好是 head）。
 * 复杂度：时间 O(n)，空间 O(1)，只扫描一趟
 */
namespace lc0019 {
class Solution {
 public:
  ListNode* removeNthFromEnd(ListNode* head, int n) {
    ListNode dummy(0);
    dummy.next = head;
    ListNode* fast = &dummy;
    for (int i = 0; i < n; ++i) fast = fast->next;  // 先走 n 步
    ListNode* slow = &dummy;
    while (fast->next) {
      slow = slow->next;
      fast = fast->next;
    }
    ListNode* victim = slow->next;
    slow->next = victim->next;
    delete victim;  // 题目语义是删除，测试里就不能让它泄漏
    return dummy.next;
  }
};
}  // namespace lc0019

TEST(linked_list, q19_remove_nth_from_end) {
  lc0019::Solution s;
  ListNode* a = buildList({1, 2, 3, 4, 5});
  CHECK_EQ(listToVec(s.removeNthFromEnd(a, 2)), vector<int>{1, 2, 3, 5});
  ListNode* b = buildList({1});
  CHECK_TRUE(s.removeNthFromEnd(b, 1) == nullptr);  // 删到空
  ListNode* c = buildList({1, 2});
  CHECK_EQ(listToVec(s.removeNthFromEnd(c, 2)), vector<int>{2});  // 删头
}

// -----------------------------------------------------------------------------
/*
 * Q24. 两两交换链表中的节点  Medium
 * https://leetcode.cn/problems/swap-nodes-in-pairs/
 *
 * 思路：每轮关注 4 个指针：prev、first、second、third。
 *      改链顺序：prev->next=second; second->next=first; first->next=third。
 *      交换后 prev 要前进两步（移动到 first 的位置），它就是下一组的 prev。
 * 复杂度：时间 O(n)，空间 O(1)。递归写法更短但栈空间 O(n)。
 */
namespace lc0024 {
class Solution {
 public:
  ListNode* swapPairs(ListNode* head) {
    ListNode dummy(0);
    dummy.next = head;
    ListNode* prev = &dummy;
    while (prev->next && prev->next->next) {
      ListNode* first = prev->next;
      ListNode* second = first->next;
      first->next = second->next;
      second->next = first;
      prev->next = second;
      prev = first;  // 交换后 first 已是本组尾部
    }
    return dummy.next;
  }
};
}  // namespace lc0024

TEST(linked_list, q24_swap_pairs) {
  lc0024::Solution s;
  ListNode* a = buildList({1, 2, 3, 4});
  CHECK_EQ(listToVec(s.swapPairs(a)), vector<int>{2, 1, 4, 3});
  ListNode* b = buildList({1, 2, 3});  // 奇数个，落单的不换
  CHECK_EQ(listToVec(s.swapPairs(b)), vector<int>{2, 1, 3});
  CHECK_TRUE(s.swapPairs(nullptr) == nullptr);
  ListNode* c = buildList({7});
  CHECK_EQ(listToVec(s.swapPairs(c)), vector<int>{7});
}

// -----------------------------------------------------------------------------
/*
 * Q25. K 个一组翻转链表  Hard
 * https://leetcode.cn/problems/reverse-nodes-in-k-group/
 *
 * 思路：先探路确认剩余够 k 个（不够就保留原序），再反转这一组。
 *      反转时把 prev 初值设为 groupNext，反转完自然接上了后段。
 *      groupPrev 只在本组完成后前移一次，移到「本组原来的尾结点」。
 * 复杂度：时间 O(n)，空间 O(1)
 * 易错：反转后本组的头尾互换，记 `tail = 原头` 才能正确接下一组。
 */
namespace lc0025 {
class Solution {
 public:
  ListNode* reverseKGroup(ListNode* head, int k) {
    ListNode dummy(0);
    dummy.next = head;
    ListNode* groupPrev = &dummy;
    while (true) {
      ListNode* kth = groupPrev;
      for (int i = 0; i < k && kth; ++i) kth = kth->next;
      if (!kth) break;                       // 不足 k 个，保持原序
      ListNode* groupNext = kth->next;
      ListNode* prev = groupNext;            // 反转时组尾直接接上后段
      ListNode* cur = groupPrev->next;
      while (cur != groupNext) {
        ListNode* nxt = cur->next;
        cur->next = prev;
        prev = cur;
        cur = nxt;
      }
      ListNode* groupTail = groupPrev->next;  // 旧的组头，现在是组尾
      groupPrev->next = kth;                  // 新的组头接到前一段
      groupPrev = groupTail;
    }
    return dummy.next;
  }
};
}  // namespace lc0025

TEST(linked_list, q25_reverse_k_group) {
  lc0025::Solution s;
  ListNode* a = buildList({1, 2, 3, 4, 5});
  CHECK_EQ(listToVec(s.reverseKGroup(a, 2)), vector<int>{2, 1, 4, 3, 5});
  ListNode* b = buildList({1, 2, 3, 4, 5});
  CHECK_EQ(listToVec(s.reverseKGroup(b, 3)), vector<int>{3, 2, 1, 4, 5});
  ListNode* c = buildList({1, 2, 3, 4, 5});
  CHECK_EQ(listToVec(s.reverseKGroup(c, 1)), vector<int>{1, 2, 3, 4, 5});
  ListNode* d = buildList({1, 2, 3});
  CHECK_EQ(listToVec(s.reverseKGroup(d, 4)), vector<int>{1, 2, 3});  // k 超过长度
}

// -----------------------------------------------------------------------------
/*
 * Q138. 随机链表的复制  Medium
 * https://leetcode.cn/problems/copy-list-with-random-pointer/
 *
 * 解法一（本文件采用）：两趟 + 哈希表 old->new。好懂、好讲。
 * 解法二 O(1) 空间：先把 copy 插在 原结点 后面 A->A'->B->B'，
 *      则 A'->random = A->random->next；最后再拆链。
 * 复杂度：解法一 时间 O(n)，空间 O(n)
 */
namespace lc0138 {
class Solution {
 public:
  Node* copyRandomList(Node* head) {
    unordered_map<Node*, Node*> mp;
    for (Node* p = head; p; p = p->next) mp[p] = new Node(p->val);  // 只复制值
    for (Node* p = head; p; p = p->next) {
      mp[p]->next = p->next ? mp[p->next] : nullptr;
      mp[p]->random = p->random ? mp[p->random] : nullptr;  // 指针映射过去
    }
    return head ? mp[head] : nullptr;
  }
};

// —— 测试脚手架：按下标数组构造随机链表 ——
inline Node* buildRandList(const vector<int>& vals, const vector<int>& randomIdx) {
  vector<Node*> nodes;
  for (size_t i = 0; i < vals.size(); ++i) nodes.push_back(new Node(vals[i]));
  for (size_t i = 0; i < nodes.size(); ++i) {
    if (i + 1 < nodes.size()) nodes[i]->next = nodes[i + 1];
    nodes[i]->random = randomIdx[i] < 0 ? nullptr : nodes[randomIdx[i]];
  }
  return nodes.empty() ? nullptr : nodes[0];
}
inline vector<int> randRandomIdx(Node* head, const vector<Node*>& orig) {
  vector<int> out;
  for (Node* p = head; p; p = p->next) {
    int idx = -1;
    for (size_t i = 0; i < orig.size(); ++i)
      if (orig[i] == p->random) idx = (int)i;
    out.push_back(idx);
  }
  return out;
}
inline vector<Node*> collect(Node* head) {
  vector<Node*> v;
  for (Node* p = head; p; p = p->next) v.push_back(p);
  return v;
}
// random 不影响 next 链，只顺着 next 释放即可（不会 double free）
inline void freeRandList(Node* head) {
  while (head) {
    Node* n = head->next;
    delete head;
    head = n;
  }
}
}  // namespace lc0138

TEST(linked_list, q138_copy_random_list) {
  lc0138::Solution s;
  Node* head = lc0138::buildRandList({7, 13, 11, 10, 1}, {-1, 0, 4, 2, 0});
  Node* copy = s.copyRandomList(head);
  CHECK_EQ(lc0138::collect(copy).size(), (size_t)5);  // 长度一致
  CHECK_EQ(lc0138::randRandomIdx(copy, lc0138::collect(copy)),
           vector<int>{-1, 0, 4, 2, 0});  // random 相对结构一致
  CHECK_TRUE(copy != head);               // 必须是新结点
  CHECK_TRUE(copy->next != head->next);
  Node* empty = s.copyRandomList(nullptr);
  CHECK_TRUE(empty == nullptr);
  lc0138::freeRandList(head);
  lc0138::freeRandList(copy);
}

// -----------------------------------------------------------------------------
/*
 * Q148. 排序链表  Medium
 * https://leetcode.cn/problems/sort-list/
 *
 * 思路：自顶向下归并。快慢指针切两半 -> 递归排序 -> 合并（Q21）。
 *      链表归并的优势：不需要像数组那样开额外临时空间，天然 O(1) 合并。
 * 复杂度：时间 O(n log n)，递归栈 O(log n)
 * 进阶：自底向上按 size=1,2,4... 倍增归并，可做真 O(1) 空间（面试官常追问）。
 *      快排在链表上最坏会退化，堆排序不方便，所以这题的标准答案是归并。
 */
namespace lc0148 {
class Solution {
 public:
  ListNode* sortList(ListNode* head) {
    if (!head || !head->next) return head;
    // 找左半部分的尾：fast 从 head->next 出发，保证偶数长度时 slow 落在前段末尾
    ListNode *slow = head, *fast = head->next;
    while (fast && fast->next) {
      slow = slow->next;
      fast = fast->next->next;
    }
    ListNode* second = slow->next;
    slow->next = nullptr;  // 必须断链，否则两段互相牵连
    return merge(sortList(head), sortList(second));
  }

 private:
  static ListNode* merge(ListNode* a, ListNode* b) {
    ListNode dummy(0);
    ListNode* t = &dummy;
    while (a && b) {
      if (a->val <= b->val) {
        t->next = a;
        a = a->next;
      } else {
        t->next = b;
        b = b->next;
      }
      t = t->next;
    }
    t->next = a ? a : b;
    return dummy.next;
  }
};
}  // namespace lc0148

TEST(linked_list, q148_sort_list) {
  lc0148::Solution s;
  ListNode* a = buildList({4, 2, 1, 3});
  CHECK_EQ(listToVec(s.sortList(a)), vector<int>{1, 2, 3, 4});
  ListNode* b = buildList({-1, 5, 3, 4, 0});
  CHECK_EQ(listToVec(s.sortList(b)), vector<int>{-1, 0, 3, 4, 5});
  CHECK_TRUE(s.sortList(nullptr) == nullptr);
  ListNode* c = buildList({1, 1, 1});  // 全相等，考验归并的稳定性
  CHECK_EQ(listToVec(s.sortList(c)), vector<int>{1, 1, 1});
}

// -----------------------------------------------------------------------------
/*
 * Q23. 合并 K 个升序链表  Hard
 * https://leetcode.cn/problems/merge-k-sorted-lists/
 *
 * 思路：最小堆维护「每条链当前未合并的最小结点」。弹出接答案，把它 next 入堆。
 * 复杂度：时间 O(N log k)，空间 O(k)。
 *      对比「逐个两两合并」是 O(N·k)；对比「分治两两合并」同为 O(N log k) 但无堆开销。
 * C++ 细节：priority_queue 默认是最大堆，比较器写 `a->val > b->val` 才得到最小堆。
 */
namespace lc0023 {
class Solution {
 public:
  struct Cmp {
    bool operator()(const ListNode* a, const ListNode* b) const {
      return a->val > b->val;  // 注意：> 才是小顶堆
    }
  };

  ListNode* mergeKLists(vector<ListNode*>& lists) {
    priority_queue<ListNode*, vector<ListNode*>, Cmp> pq;
    for (ListNode* l : lists)
      if (l) pq.push(l);
    ListNode dummy(0);
    ListNode* tail = &dummy;
    while (!pq.empty()) {
      ListNode* node = pq.top();
      pq.pop();
      tail->next = node;
      tail = node;
      if (node->next) pq.push(node->next);
    }
    tail->next = nullptr;
    return dummy.next;
  }
};
}  // namespace lc0023

TEST(linked_list, q23_merge_k_sorted) {
  lc0023::Solution s;
  vector<ListNode*> lists{buildList({1, 4, 5}), buildList({1, 3, 4}), buildList({2, 6})};
  CHECK_EQ(listToVec(s.mergeKLists(lists)), vector<int>{1, 1, 2, 3, 4, 4, 5, 6});
  vector<ListNode*> noLists{};
  CHECK_TRUE(s.mergeKLists(noLists) == nullptr);
  vector<ListNode*> onlyNull{nullptr, nullptr};
  CHECK_TRUE(s.mergeKLists(onlyNull) == nullptr);
}

// -----------------------------------------------------------------------------
/*
 * Q146. LRU 缓存  Medium
 * https://leetcode.cn/problems/lru-cache/
 *
 * 数据结构 = 哈希表 + 双向链表：
 *   · 双向链表保存「使用顺序」，表头最新、表尾最旧，淘汰只看表尾。
 *   · 哈希表 key -> 链表迭代器，保证 get 能 O(1) 找到结点。
 *   · splice 是 O(1) 把结点搬到表头的操作，不用 new/delete，也不会让迭代器失效。
 * 复杂度：get / put 均摊 O(1)，空间 O(capacity)
 * 延伸：LFU 需要「频次桶 + 每桶内 LRU」，见 Q460。
 */
namespace lc0146 {
class LRUCache {
 public:
  explicit LRUCache(int capacity) : cap_(capacity) {}

  int get(int key) {
    auto it = pos_.find(key);
    if (it == pos_.end()) return -1;
    order_.splice(order_.begin(), order_, it->second);  // 提到表头
    return it->second->second;
  }

  void put(int key, int value) {
    if (cap_ <= 0) return;  // 题面保证 capacity ≥ 1；否则下面的 order_.back() 会在空表上 UB
    auto it = pos_.find(key);
    if (it != pos_.end()) {
      it->second->second = value;
      order_.splice(order_.begin(), order_, it->second);
      return;
    }
    if ((int)order_.size() == cap_) {  // 先淘汰，再插入
      pos_.erase(order_.back().first);
      order_.pop_back();
    }
    order_.emplace_front(key, value);
    pos_[key] = order_.begin();
  }

 private:
  int cap_;
  std::list<std::pair<int, int>> order_;  // (key, value)，front = 最近使用
  std::unordered_map<int, std::list<std::pair<int, int>>::iterator> pos_;
};
}  // namespace lc0146

TEST(linked_list, q146_lru_cache) {
  lc0146::LRUCache c(2);
  c.put(1, 1);
  c.put(2, 2);
  CHECK_EQ(c.get(1), 1);
  c.put(3, 3);  // 淘汰最久未用的 2
  CHECK_EQ(c.get(2), -1);
  c.put(4, 4);  // 淘汰 1
  CHECK_EQ(c.get(1), -1);
  CHECK_EQ(c.get(3), 3);
  CHECK_EQ(c.get(4), 4);
  // 更新已有 key 不算新增，不触发淘汰
  lc0146::LRUCache d(2);
  d.put(1, 1);
  d.put(2, 2);
  d.put(2, 20);
  CHECK_EQ(d.get(2), 20);
  CHECK_EQ(d.get(1), 1);
}
