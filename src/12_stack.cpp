// =============================================================================
//  12 · 栈 (Stack)
//  Q20 有效的括号 · Q155 最小栈 · Q394 字符串解码 · Q739 每日温度 ·
//  Q84 柱状图中最大的矩形
//
//  【单调栈：一类题一个模板】
//  求「左边/右边第一个更大/更小的元素」，全都用单调栈：
//      for each x:
//          while (栈顶破坏了单调性) { 弹出并结算 }
//          此刻栈顶就是 x 要找的那个「第一个更大/更小者」
//          x 入栈
//  栈内维持什么，取决于你要找什么：
//      · 找「右边第一个更大」-> 栈递减（Q739 每日温度）
//      · 找「左右第一个更小」-> 栈递增（Q84 柱状图）
//  为什么是 O(n)：每个下标最多入栈一次、出栈一次。
//  另一个大类：括号匹配 / 嵌套结构 -> 普通栈（Q20、Q394、Q150、Q224）。
//  顺带一提：Q42 接雨水也能用单调栈（按「凹槽」逐层结算），和双指针法互为验证。
// =============================================================================
#include <algorithm>
#include <cctype>
#include <stack>
#include <string>
#include <vector>

#include "test.h"

using namespace std;

// -----------------------------------------------------------------------------
/*
 * Q20. 有效的括号  Easy
 * https://leetcode.cn/problems/valid-parentheses/
 *
 * 思路：遇到左括号入栈；遇到右括号则栈顶必须是配对的左括号。
 * 易错：① 右括号来时栈可能为空（")(" 这种情况）—— 必须判空再 top()；
 *      ② 结束时栈必须为空，否则 "((" 会被误判为合法。
 * 复杂度：时间 O(n)，空间 O(n)
 */
namespace lc0020 {
class Solution {
 public:
  bool isValid(const string& s) {
    stack<char> st;
    for (char c : s) {
      if (c == '(' || c == '[' || c == '{') {
        st.push(c);
      } else {
        if (st.empty()) return false;  // 右括号多余
        char open = st.top();
        st.pop();
        if (c == ')' && open != '(') return false;
        if (c == ']' && open != '[') return false;
        if (c == '}' && open != '{') return false;
      }
    }
    return st.empty();  // 左括号不能剩余
  }
};
}  // namespace lc0020

TEST(stack, q20_valid_parentheses) {
  lc0020::Solution s;
  CHECK_TRUE(s.isValid("()"));
  CHECK_TRUE(s.isValid("()[]{}"));
  CHECK_FALSE(s.isValid("(]"));
  CHECK_FALSE(s.isValid("([)]"));  // 交叉嵌套非法
  CHECK_TRUE(s.isValid("{[]}"));
  CHECK_FALSE(s.isValid("]"));   // 以右括号开头
  CHECK_FALSE(s.isValid("("));   // 结束时栈非空
  CHECK_TRUE(s.isValid(""));
}

// -----------------------------------------------------------------------------
/*
 * Q155. 最小栈  Medium
 * https://leetcode.cn/problems/min-stack/
 *
 * 要求 getMin 也是 O(1)。做法：再开一个「最小值栈」，
 *      每次 push 时把「压入后的当前最小值」同步压进去，pop 时一起弹。
 *      两栈高度始终相同，所以不必比较值是否相等（最小值重复出现也正确）。
 * 空间优化：辅助栈只在 x <= 当前最小值时才压入，pop 时同步弹出。
 * 复杂度：所有操作 O(1)，空间 O(n)
 */
namespace lc0155 {
class MinStack {
 public:
  void push(int val) {
    st_.push(val);
    // 关键：压的是「val 与当前最小值里的较小者」，min_ 栈顶永远是 st_ 全体的最小值
    min_.push(min_.empty() ? val : min(val, min_.top()));
  }

  void pop() {
    st_.pop();
    min_.pop();
  }

  int top() const { return st_.top(); }
  int getMin() const { return min_.top(); }

 private:
  stack<int> st_;
  stack<int> min_;
};
}  // namespace lc0155

TEST(stack, q155_min_stack) {
  lc0155::MinStack st;
  st.push(-2);
  st.push(0);
  st.push(-3);
  CHECK_EQ(st.getMin(), -3);
  st.pop();
  CHECK_EQ(st.top(), 0);
  CHECK_EQ(st.getMin(), -2);
  // 重复最小值：弹掉一个 2 之后，最小值仍是 2
  lc0155::MinStack d;
  d.push(2);
  d.push(2);
  d.push(1);
  d.pop();
  CHECK_EQ(d.getMin(), 2);
  d.pop();
  CHECK_EQ(d.getMin(), 2);
}

// -----------------------------------------------------------------------------
/*
 * Q394. 字符串解码  Medium
 * https://leetcode.cn/problems/decode-string/
 *
 * 思路：嵌套结构 -> 双栈。一个存重复次数，一个存「进入本层之前已拼好的串」。
 *      遇 '[' 把当前 num 和当前 cur 同时压栈并清零（开新层）；
 *      遇 ']' 弹栈：cur = 上层前缀 + 本层内容 × 次数。
 *      数字可能多位（"12[a]"），所以要 num = num*10 + (c-'0') 累积。
 * 另解：递归下降（遇 '[' 就递归）代码更短，栈空间隐式。
 * 复杂度：时间 O(输出长度)，空间 O(嵌套层数 + 输出长度)（栈里存的串本身也是 O(输出)）
 */
namespace lc0394 {
class Solution {
 public:
  string decodeString(const string& s) {
    stack<int> nums;
    stack<string> parts;
    string cur;
    int num = 0;
    for (char c : s) {
      if (isdigit((unsigned char)c)) {
        num = num * 10 + (c - '0');
      } else if (c == '[') {
        nums.push(num);   // 本层要重复几次
        parts.push(cur);  // 保存本层之前的内容
        num = 0;
        cur.clear();      // 开始收集本层内容
      } else if (c == ']') {
        string body = cur;
        int times = nums.top();
        nums.pop();
        cur = parts.top();  // 回到上一层
        parts.pop();
        for (int i = 0; i < times; ++i) cur += body;
      } else {
        cur += c;
      }
    }
    return cur;
  }
};
}  // namespace lc0394

TEST(stack, q394_decode_string) {
  lc0394::Solution s;
  CHECK_EQ(s.decodeString("3[a]2[bc]"), string("aaabcbc"));
  CHECK_EQ(s.decodeString("3[a2[c]]"), string("accaccacc"));  // 嵌套
  CHECK_EQ(s.decodeString("2[abc]3[cd]ef"), string("abcabccdcdcdef"));
  CHECK_EQ(s.decodeString("abc3[cd]xyz"), string("abccdcdcdxyz"));
  CHECK_EQ(s.decodeString("10[a]"), string("aaaaaaaaaa"));  // 多位数字
}

// -----------------------------------------------------------------------------
/*
 * Q739. 每日温度  Medium
 * https://leetcode.cn/problems/daily-temperatures/
 *
 * 思路：单调递减栈存「下标」。当前温度比栈顶高，说明栈顶那天「等到今天升温了」，
 *      弹出并结算 i - 栈顶下标。
 * 这是「右边第一个更大元素」的标准模板，改几行就是：
 *      Q496 下一个更大元素 I / Q503 下一个更大元素 II（循环数组跑两圈）/
 *      Q84（改成求更小并结算面积）
 * 复杂度：时间 O(n)，空间 O(n)
 */
namespace lc0739 {
class Solution {
 public:
  vector<int> dailyTemperatures(vector<int>& temperatures) {
    const int n = (int)temperatures.size();
    vector<int> ans(n, 0);
    stack<int> st;  // 存下标，对应温度单调递减
    for (int i = 0; i < n; ++i) {
      while (!st.empty() && temperatures[i] > temperatures[st.top()]) {
        int j = st.top();
        st.pop();
        ans[j] = i - j;  // 第 j 天等了 i-j 天
      }
      st.push(i);
    }
    return ans;  // 栈里剩下的天然等不到更高的一天，保持 0
  }
};
}  // namespace lc0739

TEST(stack, q739_daily_temperatures) {
  lc0739::Solution s;
  vector<int> a{73, 74, 75, 71, 69, 72, 76, 73};
  CHECK_EQ(s.dailyTemperatures(a), vector<int>{1, 1, 4, 2, 1, 1, 0, 0});
  vector<int> b{30, 40, 50, 60};
  CHECK_EQ(s.dailyTemperatures(b), vector<int>{1, 1, 1, 0});
  vector<int> c{60, 50, 40, 30};  // 递减，全部等不到
  CHECK_EQ(s.dailyTemperatures(c), vector<int>{0, 0, 0, 0});
}

// -----------------------------------------------------------------------------
/*
 * Q84. 柱状图中最大的矩形  Hard
 * https://leetcode.cn/problems/largest-rectangle-in-histogram/
 *
 * 思路：枚举「以某根柱子的高度为准」能撑多宽 —— 即左右第一个更矮的柱子在哪。
 *      单调递增栈一次遍历搞定：当 h[i] 小于栈顶时弹出 t，
 *      弹出后新栈顶 left 就是 t 左边第一个更矮的位置，i 是右边第一个更矮的位置，
 *      于是宽度 = i - left - 1。
 *      首尾各加一根 0 哨兵，免掉「栈空」和「收尾未结算」两种特判。
 * 复杂度：时间 O(n)，空间 O(n)
 * 联动：Q85 最大矩形 = 把每一行当作柱状图的底，逐行调用本函数。
 * 数值范围：高度 <= 10^4、n <= 10^5，面积最大 10^9，int 刚好装得下（2^31-1 ≈ 2.1×10^9）；
 *      范围再大一档（比如高度 10^5）就必须换 long long 了。
 */
namespace lc0084 {
class Solution {
 public:
  int largestRectangleArea(vector<int>& heights) {
    vector<int> h;
    h.reserve(heights.size() + 2);
    h.push_back(0);  // 左哨兵：保证栈里永远有一个更矮的柱子
    for (int x : heights) h.push_back(x);
    h.push_back(0);  // 右哨兵：保证结束时全部结算完
    stack<int> st;   // 单调递增栈（存下标）
    int best = 0;
    for (int i = 0; i < (int)h.size(); ++i) {
      while (!st.empty() && h[i] < h[st.top()]) {
        int height = h[st.top()];
        st.pop();
        int left = st.top();  // 左边第一个更矮的
        best = max(best, height * (i - left - 1));
      }
      st.push(i);
    }
    return best;
  }
};
}  // namespace lc0084

TEST(stack, q84_largest_rectangle_in_histogram) {
  lc0084::Solution s;
  vector<int> a{2, 1, 5, 6, 2, 3};
  CHECK_EQ(s.largestRectangleArea(a), 10);  // 高 5、宽 2（柱子 5 和 6）
  vector<int> b{2, 4};
  CHECK_EQ(s.largestRectangleArea(b), 4);
  vector<int> c{1, 1, 1, 1};  // 等高要能连成一片
  CHECK_EQ(s.largestRectangleArea(c), 4);
  vector<int> d{6, 7, 5, 2, 4, 5, 9, 3};
  CHECK_EQ(s.largestRectangleArea(d), 16);  // 高 2 撑满 8 根柱子；高>4 的最长段只有 3（→ 12）
  vector<int> e{0};
  CHECK_EQ(s.largestRectangleArea(e), 0);
  vector<int> f{2, 1, 2};
  CHECK_EQ(s.largestRectangleArea(f), 3);  // 高 1 × 宽 3（中间那根矮柱不阻断底边）
}
