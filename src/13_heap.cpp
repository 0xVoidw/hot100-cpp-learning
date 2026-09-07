// =============================================================================
//  13 · 堆 (Heap / Priority Queue)
//  Q215 数组中的第 K 个最大元素 · Q347 前 K 个高频元素 · Q295 数据流的中位数
//
//  【三件套】
//  1. 「第 K / 前 K」问题 -> 维护大小为 K 的堆，比全排序 O(n log n) 优：
//       · 求第 K 大：用小顶堆装 K 个最大的（堆顶是这 K 个里最小的 = 答案）
//       · 求前 K 高频：小顶堆按频次淘汰，或桶排序做到 O(n)
//     记忆口诀：第 K 大用小顶堆，第 K 小用大顶堆（堆顶是「守门员」）。
//  2. 「流式 / 动态求中位数、第 K 小」-> 对顶堆：
//       大顶堆装较小的一半，小顶堆装较大的一半，保持两堆 size 差 <= 1，
//       中位数就在两个堆顶之间（Q295、Q480、Q502 都能套）。
//  3. C++ 的 priority_queue 默认是「大顶堆」！
//       小顶堆要显式写：priority_queue<int, vector<int>, greater<int>>
//       自定义比较时注意：comp(a,b)==true 表示 a 的优先级低于 b（跟 sort 相反方向）。
// =============================================================================
#include <functional>
#include <queue>
#include <random>
#include <unordered_map>
#include <vector>

#include "test.h"

using namespace std;

// -----------------------------------------------------------------------------
/*
 * Q215. 数组中的第 K 个最大元素  Medium
 * https://leetcode.cn/problems/kth-largest-element-in-an-array/
 *
 * 解法 A：大小为 K 的小顶堆，O(n log k) / O(k) —— 数据流场景的唯一选择。
 * 解法 B：快速选择（quickselect），平均 O(n) / O(1)：
 *        随机选 pivot，做升序 Lomuto 划分（左边都 <= pivot，右边都 > pivot），
 *        第 K 大就是下标 target = n - k；只看包含 target 的那一半，递归。
 *        随机化是必须的：否则有序输入会让它退化到 O(n^2)。
 * 解法 C：nth_element(nums.end()-k, ...) —— 一行，工程里最该用这个。
 * 复杂度对比：sort 是 O(n log n)，本题 n 可到 10^5，三种都比它快或相当。
 */
namespace lc0215 {
class Solution {
 public:
  // 解法 A：K 个小顶堆
  int findKthLargest(vector<int>& nums, int k) {
    priority_queue<int, vector<int>, greater<int>> heap;  // 小顶堆
    for (int x : nums) {
      heap.push(x);
      if ((int)heap.size() > k) heap.pop();  // 堆顶（最小）被淘汰
    }
    return heap.top();  // 留在堆里的是最大的 k 个，堆顶即第 k 大
  }

  // 解法 B：快速选择（原地，平均线性）
  int findKthLargestQuickSelect(vector<int>& nums, int k) {
    int target = (int)nums.size() - k;  // 转成「升序第 target 个」
    int lo = 0, hi = (int)nums.size() - 1;
    mt19937 rng(1234567);  // 固定种子让测试结果可复现
    while (lo < hi) {
      int p = partition(nums, lo, hi, rng);
      if (p == target)
        return nums[p];
      else if (p < target)
        lo = p + 1;
      else
        hi = p - 1;
    }
    return nums[lo];
  }

 private:
  // Lomuto 划分 + 随机 pivot：返回 pivot 最终下标，左边都 <= nums[p]
  static int partition(vector<int>& a, int lo, int hi, mt19937& rng) {
    uniform_int_distribution<int> dist(lo, hi);
    int r = dist(rng);
    swap(a[r], a[hi]);
    int pivot = a[hi], i = lo;
    for (int j = lo; j < hi; ++j)
      if (a[j] <= pivot) swap(a[i++], a[j]);
    swap(a[i], a[hi]);
    return i;
  }
};
}  // namespace lc0215

TEST(heap, q215_kth_largest_element) {
  lc0215::Solution s;
  vector<int> a{3, 2, 1, 5, 6, 4};
  CHECK_EQ(s.findKthLargest(a, 2), 5);
  vector<int> b{3, 2, 3, 1, 2, 4, 5, 5, 6};
  CHECK_EQ(s.findKthLargest(b, 4), 4);
  vector<int> c{1};
  CHECK_EQ(s.findKthLargest(c, 1), 1);
  vector<int> d{7, 6, 5, 4, 3, 2, 1};  // 已排序（随机化 pivot 才不退化）
  CHECK_EQ(s.findKthLargest(d, 3), 5);
  // 快速选择：注意它会打乱原数组，所以传副本
  vector<int> e{3, 2, 3, 1, 2, 4, 5, 5, 6};
  CHECK_EQ(s.findKthLargestQuickSelect(e, 4), 4);
  vector<int> f{3, 2, 1, 5, 6, 4};
  CHECK_EQ(s.findKthLargestQuickSelect(f, 2), 5);
  vector<int> g{2, 1};
  CHECK_EQ(s.findKthLargestQuickSelect(g, 1), 2);
}

// -----------------------------------------------------------------------------
/*
 * Q347. 前 K 个高频元素  Medium
 * https://leetcode.cn/problems/top-k-frequent-elements/
 *
 * 解法 A：频次 -> 小顶堆（容量 K），O(n log k)。
 * 解法 B：桶排序，O(n)：频次只可能落在 1..n，开 n+1 个桶，从高频桶倒序取 K 个。
 *        —— 「值域有界」时，桶排序几乎总是比堆更快更好写。
 * 输出顺序题目不要求，所以测试里先排序再比较。
 */
namespace lc0347 {
class Solution {
 public:
  vector<int> topKFrequent(vector<int>& nums, int k) {
    unordered_map<int, int> cnt;
    for (int x : nums) ++cnt[x];
    // 小顶堆按频次淘汰：堆里始终保留频次最高的 k 个
    auto cmp = [](const pair<int, int>& a, const pair<int, int>& b) {
      return a.second > b.second;  // 注意方向：> 让小顶堆
    };
    priority_queue<pair<int, int>, vector<pair<int, int>>, decltype(cmp)> heap(cmp);
    for (auto& [num, c] : cnt) {
      heap.push({num, c});
      if ((int)heap.size() > k) heap.pop();
    }
    vector<int> ans;
    while (!heap.empty()) {
      ans.push_back(heap.top().first);
      heap.pop();
    }
    return ans;
  }

  vector<int> topKFrequentBucket(vector<int>& nums, int k) {
    unordered_map<int, int> cnt;
    for (int x : nums) ++cnt[x];
    vector<vector<int>> bucket(nums.size() + 1);  // 下标 = 出现次数
    for (auto& [num, c] : cnt) bucket[c].push_back(num);
    vector<int> ans;
    for (int f = (int)bucket.size() - 1; f >= 0 && (int)ans.size() < k; --f)
      for (int num : bucket[f]) {
        ans.push_back(num);
        if ((int)ans.size() == k) break;
      }
    return ans;
  }
};
}  // namespace lc0347

TEST(heap, q347_top_k_frequent) {
  lc0347::Solution s;
  auto sorted = [](vector<int> v) {
    sort(v.begin(), v.end());
    return v;
  };
  vector<int> a{1, 1, 1, 2, 2, 3};
  CHECK_EQ(sorted(s.topKFrequent(a, 2)), sorted(vector<int>{1, 2}));
  CHECK_EQ(sorted(s.topKFrequentBucket(a, 2)), sorted(vector<int>{1, 2}));
  vector<int> b{1};
  CHECK_EQ(s.topKFrequent(b, 1), vector<int>{1});
  vector<int> c{-1, -1};
  CHECK_EQ(s.topKFrequent(c, 1), vector<int>{-1});
  vector<int> d{1, 2};  // 频次相同，任取一个；k=2 时都要
  CHECK_EQ(sorted(s.topKFrequent(d, 2)), sorted(vector<int>{1, 2}));
  CHECK_EQ(sorted(s.topKFrequentBucket(d, 2)), sorted(vector<int>{1, 2}));
  vector<int> e{4, 4, 4, 3, 3, 2, 1, 1, 1, 1};
  CHECK_EQ(sorted(s.topKFrequentBucket(e, 2)), sorted(vector<int>{1, 4}));
}

// -----------------------------------------------------------------------------
/*
 * Q295. 数据流的中位数  Hard
 * https://leetcode.cn/problems/find-median-from-data-stream/
 *
 * 思路：对顶堆。lo 是大顶堆（较小的一半），hi 是小顶堆（较大的一半），
 *      维持不变式：lo.size() == hi.size() 或 lo.size() == hi.size() + 1。
 *      中位数：总数奇数 -> lo 堆顶；偶数 -> 两个堆顶平均。
 * 为什么不用排序？插入是 O(n)；堆让 addNum O(log n)、findMedian O(1)。
 * 复杂度：addNum O(log n)，findMedian O(1)，空间 O(n)
 * 延伸：Q480 滑动窗口中位数要能删除任意元素 -> 懒删除（延迟删除）+ 两个堆。
 */
namespace lc0295 {
class MedianFinder {
 public:
  void addNum(int num) {
    lo.push(num);        // 先进大顶堆，再把它的「最大值」交给右半 —— 保证左半全部 <= 右半
    hi.push(lo.top());   // 把大顶堆最大值挪到右半，维持 lo <= hi
    lo.pop();
    if (hi.size() > lo.size()) {  // 保持 lo 不少于 hi
      lo.push(hi.top());
      hi.pop();
    }
  }

  double findMedian() const {
    if (lo.size() > hi.size()) return lo.top();
    return (static_cast<double>(lo.top()) + hi.top()) / 2.0;
  }

 private:
  priority_queue<int> lo;                                 // 大顶堆：较小的一半
  priority_queue<int, vector<int>, greater<int>> hi;      // 小顶堆：较大的一半
};
}  // namespace lc0295

TEST(heap, q295_median_finder) {
  lc0295::MedianFinder mf;
  mf.addNum(1);
  CHECK_NEAR(mf.findMedian(), 1.0, 1e-9);
  mf.addNum(2);
  CHECK_NEAR(mf.findMedian(), 1.5, 1e-9);
  mf.addNum(3);
  CHECK_NEAR(mf.findMedian(), 2.0, 1e-9);
  mf.addNum(4);
  CHECK_NEAR(mf.findMedian(), 2.5, 1e-9);
  mf.addNum(5);
  CHECK_NEAR(mf.findMedian(), 3.0, 1e-9);
  lc0295::MedianFinder m2;
  m2.addNum(-1);
  m2.addNum(-2);
  CHECK_NEAR(m2.findMedian(), -1.5, 1e-9);  // 负数
  m2.addNum(-3);
  CHECK_NEAR(m2.findMedian(), -2.0, 1e-9);
}
