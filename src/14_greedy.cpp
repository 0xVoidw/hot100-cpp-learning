// =============================================================================
//  14 · 贪心 (Greedy)
//  Q121 买卖股票的最佳时机 · Q55 跳跃游戏 · Q45 跳跃游戏 II · Q763 划分字母区间
//
//  【贪心能不能用，看两件事】
//  1. 贪心选择性质：局部最优选择不会影响后续的可解性（能证明「exchange argument」：
//     把任一最优解的第一步换成我的贪心选择，解不会变差）。
//  2. 最优子结构：剩下的子问题的最优解 + 我的选择 = 全局最优。
//  不满足就必须上 DP。经典反例：零钱兑换用「每次取最大面额」是错的
//      （coins=[1,3,4], amount=6：贪心得 4+1+1 三张，最优是 3+3 两张 —— 见 Q322）。
//  跳跃类题目的统一视角：把「能到达的最远处」当成贪心状态，
//     能到 i 才能从 i 出发（Q55 判可达、Q45 数步数、Q1306/Q1345 同理）。
// =============================================================================
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

#include "test.h"

using namespace std;

// -----------------------------------------------------------------------------
/*
 * Q121. 买卖股票的最佳时机  Easy
 * https://leetcode.cn/problems/best-time-to-buy-and-sell-stock/
 *
 * 思路：只许买一次卖一次（先买后卖）。边扫边记「历史最低价」，
 *      用当天价格减它取最大 —— 等价于对每个卖出日算最优买入日。
 * 复杂度：时间 O(n)，空间 O(1)
 * 家族：Q122 无限次交易 = 累加所有上涨段；Q123 最多 2 笔 / Q188 最多 k 笔 = 状态机 DP；
 *      Q309 含冷冻期、Q714 含手续费 = 在同一套状态机上改转移。
 */
namespace lc0121 {
class Solution {
 public:
  int maxProfit(vector<int>& prices) {
    if (prices.empty()) return 0;  // 防 prices[0] 越界（题面保证 n >= 1）
    int ans = 0;
    int low = prices[0];  // 到目前为止的最低买入价
    for (int p : prices) {
      ans = max(ans, p - low);  // 今天卖
      low = min(low, p);        // 更新「今天及之前」最便宜的买点
    }
    return ans;  // 全程下跌时保持 0（不交易）
  }
};
}  // namespace lc0121

TEST(greedy, q121_best_time_to_buy_and_sell) {
  lc0121::Solution s;
  vector<int> a{7, 1, 5, 3, 6, 4};
  CHECK_EQ(s.maxProfit(a), 5);  // 第 2 天 1 买、第 5 天 6 卖
  vector<int> b{7, 6, 4, 3, 1};
  CHECK_EQ(s.maxProfit(b), 0);  // 一直跌就不做
  vector<int> c{1, 2};
  CHECK_EQ(s.maxProfit(c), 1);
  vector<int> d{2, 4, 1};  // 低点出现在高点之后，不能倒着卖
  CHECK_EQ(s.maxProfit(d), 2);
  vector<int> e{2, 1, 4};  // 卖出日可以比买入日晚很多
  CHECK_EQ(s.maxProfit(e), 3);
  vector<int> f{5};
  CHECK_EQ(s.maxProfit(f), 0);
}

// -----------------------------------------------------------------------------
/*
 * Q55. 跳跃游戏  Medium
 * https://leetcode.cn/problems/jump-game/
 *
 * 思路：贪心维护「最远能到达的下标」reach。遍历到 i 时，如果 i > reach 说明
 *      这一步都迈不出去（中间有跳不过去的坑）-> 失败；否则用 i+nums[i] 更新 reach。
 *      注意不需要真的模拟每一步跳多远，这就是它比 DP 快在哪。
 * 复杂度：时间 O(n)，空间 O(1)
 */
namespace lc0055 {
class Solution {
 public:
  bool canJump(vector<int>& nums) {
    int reach = 0;
    for (int i = 0; i < (int)nums.size(); ++i) {
      if (i > reach) return false;  // 当前位置不可达
      reach = max(reach, i + nums[i]);
      if (reach >= (int)nums.size() - 1) return true;  // 提前收工
    }
    return true;
  }
};
}  // namespace lc0055

TEST(greedy, q55_jump_game) {
  lc0055::Solution s;
  vector<int> a{2, 3, 1, 1, 4};
  CHECK_TRUE(s.canJump(a));
  vector<int> b{3, 2, 1, 0, 4};  // 0 是死坑
  CHECK_FALSE(s.canJump(b));
  vector<int> c{0};  // 长度 1，站在终点
  CHECK_TRUE(s.canJump(c));
  vector<int> d{1, 0};
  CHECK_TRUE(s.canJump(d));
  vector<int> e{2, 0, 0};  // 恰好跨过 0
  CHECK_TRUE(s.canJump(e));
  vector<int> f{1, 2, 0, 1};
  CHECK_TRUE(s.canJump(f));
}

// -----------------------------------------------------------------------------
/*
 * Q45. 跳跃游戏 II  Medium
 * https://leetcode.cn/problems/jump-game-ii/
 *
 * 思路：把「可达范围」看成 BFS 的分层：
 *      curEnd = 当前这一跳能到的最远处；farthest = 在 [.., curEnd] 里出发能到的最远处。
 *      走到 curEnd 就必须再跳一次（步数 +1），并把边界推进到 farthest。
 *      循环只跑到 n-2：走到终点不用再跳了，这是最常见的 off-by-one。
 * 复杂度：时间 O(n)，空间 O(1)
 * 为什么贪心对：每一跳都尽可能延伸边界，等价于「一层最多覆盖多远」，
 *      而层数就是最少跳数 —— 和 BFS 求最短路同理。
 */
namespace lc0045 {
class Solution {
 public:
  int jump(vector<int>& nums) {
    const int n = (int)nums.size();
    int steps = 0, curEnd = 0, farthest = 0;
    for (int i = 0; i < n - 1; ++i) {  // 不处理最后一个元素
      farthest = max(farthest, i + nums[i]);
      if (i == curEnd) {  // 到边界了，必须跳
        ++steps;
        curEnd = farthest;
        if (curEnd >= n - 1) break;
      }
    }
    return steps;
  }
};
}  // namespace lc0045

TEST(greedy, q45_jump_game_ii) {
  lc0045::Solution s;
  vector<int> a{2, 3, 1, 1, 4};
  CHECK_EQ(s.jump(a), 2);  // 1 -> 4
  vector<int> b{2, 3, 0, 1, 4};
  CHECK_EQ(s.jump(b), 2);
  vector<int> c{1, 2};
  CHECK_EQ(s.jump(c), 1);
  vector<int> d{0};  // 已在终点
  CHECK_EQ(s.jump(d), 0);
  vector<int> e{1, 1, 1, 1};  // 每次只能跳 1
  CHECK_EQ(s.jump(e), 3);
}

// -----------------------------------------------------------------------------
/*
 * Q763. 划分字母区间  Medium
 * https://leetcode.cn/problems/partition-labels/
 *
 * 思路：先记录每个字母的最后出现位置 last[c]。
 *      从左往右扫，维护「当前片段必须延伸到的右端点」end = max(end, last[c])；
 *      一旦 i == end，说明这个片段里的字母都不会再出现，可以切断。
 *      本质是「区间合并」：每个字母给出一个必须在一起的区间 [i, last]，求合并后的长度。
 * 复杂度：时间 O(n + Σ)，空间 O(Σ)
 */
namespace lc0763 {
class Solution {
 public:
  vector<int> partitionLabels(const string& s) {
    vector<int> last(26, -1);
    for (int i = 0; i < (int)s.size(); ++i) last[s[i] - 'a'] = i;
    vector<int> ans;
    int start = 0, end = 0;
    for (int i = 0; i < (int)s.size(); ++i) {
      end = max(end, last[s[i] - 'a']);  // 想把这段留在身后，就得至少延伸到 end
      if (i == end) {
        ans.push_back(i - start + 1);
        start = i + 1;
      }
    }
    return ans;
  }
};
}  // namespace lc0763

TEST(greedy, q763_partition_labels) {
  lc0763::Solution s;
  CHECK_EQ(s.partitionLabels("ababcbacadefegdehijhklij"), vector<int>{9, 7, 8});
  CHECK_EQ(s.partitionLabels("eccbbbbdec"), vector<int>{10});
  CHECK_EQ(s.partitionLabels("abc"), vector<int>{1, 1, 1});
  CHECK_EQ(s.partitionLabels("aa"), vector<int>{2});
  CHECK_EQ(s.partitionLabels("abac"), vector<int>{3, 1});
}
