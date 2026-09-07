// =============================================================================
//  01 · 哈希 (Hash)
//  Q1 两数之和 · Q49 字母异位词分组 · Q128 最长连续序列
//
//  【本类题的通用思路】
//  哈希表的本质是「用 O(n) 空间，把一次线性扫描变成 O(1) 查询」。
//  凡是看到「是否存在配对 / 出现过几次 / 属于同一组」，先想哈希。
//  三个高频坑：
//    1) 边查边存 —— 避免同一个元素和自己配对（Q1 的 [3,3]）
//    2) unordered_map 的 key 可以是 string / 数组签名，但不能直接用 vector<int>
//    3) 判断「连续」时要先去重，否则 1,1,1,2 会算错长度（Q128）
// =============================================================================
#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "test.h"

using namespace std;

// -----------------------------------------------------------------------------
/*
 * Q1. 两数之和  Easy
 * https://leetcode.cn/problems/two-sum/
 *
 * 思路：一次遍历，哈希表里存「已经看过的值 -> 下标」。
 *      对当前 nums[i]，先查 target-nums[i] 在不在表里，再把自己塞进去。
 *      顺序「先查后存」保证不会用同一个元素两次（[3,3] target=6 也能过）。
 * 复杂度：时间 O(n)，空间 O(n)
 * 对比：暴力二重循环 O(n^2)；本题要返回下标，直接 sort 会把下标打乱（得额外带下标排序），
 *      所以哈希表一遍扫更顺手。
 */
namespace lc0001 {
class Solution {
 public:
  vector<int> twoSum(vector<int>& nums, int target) {
    unordered_map<int, int> seen;  // 值 -> 下标
    seen.reserve(nums.size() * 2);
    for (int i = 0; i < (int)nums.size(); ++i) {
      auto it = seen.find(target - nums[i]);
      if (it != seen.end()) return {it->second, i};
      seen[nums[i]] = i;
    }
    return {};  // 题目保证有解，这里只是编译器安心
  }
};
}  // namespace lc0001

TEST(hash, q1_two_sum) {
  lc0001::Solution s;
  vector<int> a{2, 7, 11, 15};
  CHECK_EQ(s.twoSum(a, 9), vector<int>{0, 1});
  vector<int> b{3, 2, 4};
  CHECK_EQ(s.twoSum(b, 6), vector<int>{1, 2});
  vector<int> c{3, 3};  // 重复元素：先查后存才不会自配
  CHECK_EQ(s.twoSum(c, 6), vector<int>{0, 1});
  vector<int> d{2, 5, 5, 11};
  CHECK_EQ(s.twoSum(d, 10), vector<int>{1, 2});
}

// -----------------------------------------------------------------------------
/*
 * Q49. 字母异位词分组  Medium
 * https://leetcode.cn/problems/group-anagrams/
 *
 * 思路：异位词排序后是同一个字符串 —— 拿「排序后的串」当分组 key。
 *      进阶写法：用 26 个字母的计数串做 key（每段前缀一个 '#' 分隔），
 *      把每组 O(k log k) 降到 O(k)。分隔符是必需的：没有它，「1个a+11个b」与
 *      「11个a+1个b」都会拼成 "111"，两组不同字母就被归到一组了。
 * 复杂度：排序法 时间 O(n·k log k)；计数法 时间 O(n·k)，空间 O(n·k)
 */
namespace lc0049 {
class Solution {
 public:
  vector<vector<string>> groupAnagrams(vector<string>& strs) {
    unordered_map<string, vector<string>> groups;
    for (const string& w : strs) {
      string key = w;
      sort(key.begin(), key.end());  // "eat" / "tea" -> "aet"
      groups[key].push_back(w);
    }
    vector<vector<string>> ans;
    ans.reserve(groups.size());
    for (auto& kv : groups) ans.push_back(std::move(kv.second));
    return ans;
  }

  // 计数法 key：适合单词很长、组很多的情况
  static string countKey(const string& w) {
    int cnt[26] = {};
    for (char c : w) cnt[c - 'a']++;
    string key;
    for (int i = 0; i < 26; ++i) {
      key += '#';
      key += std::to_string(cnt[i]);
    }
    return key;
  }
};
}  // namespace lc0049

TEST(hash, q49_group_anagrams) {
  lc0049::Solution s;
  vector<string> a{"eat", "tea", "tan", "ate", "nat", "bat"};
  vector<vector<string>> got = s.groupAnagrams(a);
  // 组间顺序无序 —— 排序后再比，这是「结果集合类」题目的通用断言手法
  for (auto& g : got) sort(g.begin(), g.end());
  sort(got.begin(), got.end());
  vector<vector<string>> want{{"ate", "eat", "tea"}, {"bat"}, {"nat", "tan"}};
  CHECK_EQ(got, want);
  // countKey 必须与排序 key 等价
  CHECK_EQ(lc0049::Solution::countKey("abc"), lc0049::Solution::countKey("cab"));
  // 真正考验 '#' 分隔符的一对：计数序列 [1,11] 与 [11,1]，不加分隔都拼成 "111"
  const string xa = "a" + string(11, 'b');
  const string xb = string(11, 'a') + "b";
  CHECK_TRUE(lc0049::Solution::countKey(xa) != lc0049::Solution::countKey(xb));
  vector<string> b{""};
  CHECK_EQ(s.groupAnagrams(b), vector<vector<string>>{{""}});
}

// -----------------------------------------------------------------------------
/*
 * Q128. 最长连续序列  Medium
 * https://leetcode.cn/problems/longest-consecutive-sequence/
 *
 * 思路：全部丢进 unordered_set。只对「一段连续区间的起点」展开向右延伸，
 *      起点判定 = set 里不存在 num-1。这样每个数最多被访问两次 -> O(n)。
 * 复杂度：时间 O(n)，空间 O(n)
 * 关键：不做起点判定的话，[1,2,3,4] 会从 1 和 2 都往右扫，退化成 O(n^2)。
 *      题目明确要求 O(n)，所以「排序后扫描」的 O(n log n) 只能算备用解。
 */
namespace lc0128 {
class Solution {
 public:
  int longestConsecutive(vector<int>& nums) {
    unordered_set<int> pool(nums.begin(), nums.end());  // 顺便去重
    int best = 0;
    for (int x : pool) {
      if (pool.count(x - 1)) continue;  // 不是起点，跳过
      int cur = x, len = 0;
      while (pool.count(cur)) {
        ++len;
        ++cur;  // 向右延伸
      }
      best = max(best, len);
    }
    return best;
  }
};
}  // namespace lc0128

TEST(hash, q128_longest_consecutive) {
  lc0128::Solution s;
  vector<int> a{100, 4, 200, 1, 3, 2};
  CHECK_EQ(s.longestConsecutive(a), 4);  // 1,2,3,4
  vector<int> b{0, 3, 7, 2, 5, 8, 4, 6, 0, 1};
  CHECK_EQ(s.longestConsecutive(b), 9);  // 0..8，重复的 0 不影响
  vector<int> c{};
  CHECK_EQ(s.longestConsecutive(c), 0);
  vector<int> d{1};
  CHECK_EQ(s.longestConsecutive(d), 1);
  vector<int> e{-1, 0, 1};  // 含负数
  CHECK_EQ(s.longestConsecutive(e), 3);
}
