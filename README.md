# LeetCode Hot 100 · C++ 学习库

力扣「热题 100」**全部 100 道题**的 C++17 题解，按官方 17 个分类组织。
每道题都带**思路 / 复杂度 / 易错点 / 同类题套路**注释，并且**配了单元测试、全部实测跑通**（99 个用例）。

> 这不是一份「抄完就忘」的答案集。每个文件开头是一段**分类模板总结**（比如二分的两个模板、
> 单调栈的三行套路、写 DP 的固定五问），做完一类题再回头看，比单题刷十遍更管用。

---

## 图解网站（一题一页，图文并茂）

仓库自带一个**纯静态、可离线双击打开**的讲解网站：

```
site/
├── index.html         # 首页：100 题分类列表，支持搜索 / 难度筛选 / 学习进度
├── assets/            # 样式 + 脚本
└── problems/q1.html … # 每题一页：核心套路 / 讲解 / 结构图解 / 带注释代码 / 测试用例
```

如何打开 / 重新生成：

| 操作 | 命令 |
|---|---|
| 打开 | 双击 `site/index.html`（推荐浏览器里打开）|
| 重新生成 | Windows 双击 `build_site.bat`；或 `bash build_site.sh` |
| 从源码生成 | `python tools/gen_site.py`（自动读 `src/*.cpp` 的注释与 `README` 索引）|

每页的「结构图解」**直接取自本仓库的真实测试数据**（链表、二叉树、矩阵、数组/字符串），
所以图里的数字和你刷题时看到的一致；少数难画结构的题（DP 表、双指针走位、N 皇后等）
额外给了手工「示意图」，便于理解。

**能“动起来”的题都配了算法动图**：页面上点「▶ 播放」就能看指针怎么走、窗口怎么滑、树怎么遍历、DP 表怎么填。
目前已做 **78 题**（覆盖双指针 / 滑动窗口 / 哈希 / 链表 / 树 / 网格 DFS·BFS / 回溯 / 二分 / 栈 / 堆 / 贪心 / DP / 二维 DP / 技巧几乎全部），
其余题保留静态图解。控制条支持 **播放 / 暂停 / 单步 / 速度**（1× → 1.5× → 2× → **0.5× → 0.75×**）循环。
想新增或改动某个动图，编辑 `tools/gen_animations.py` 后运行 `bash build_site.sh` 即可。

**每题页首还自带原始「题目描述」**（含示例、约束，中文，来自 `data/descriptions.json`），
不用再点去力扣看题。该文件已一次性抓取好并提交；想刷新描述，运行：

```bash
python tools/fetch_descriptions.py   # 或 --force 全量重抓
```

> 该网站无任何外部依赖（不联网、无需服务器）。

---

## 快速开始

```bash
# 方式一：一行命令直接编译 + 跑测试（只需 g++ / clang++）
bash build.sh

# 方式二：CMake + CTest
cmake -S . -B build/cmake -G "MinGW Makefiles"   # Linux/macOS 去掉 -G 即可
cmake --build build/cmake -j8
ctest --test-dir build/cmake --output-on-failure

# 方式三：手动
g++ -std=c++17 -Wall -Wextra -Iinclude src/*.cpp -o build/hot100.exe && ./build/hot100.exe
```

只跑某一类或某一题（参数是对 `分类/用例名` 做子串匹配）：

```bash
./build/hot100 linked_list     # 只跑链表那一组
./build/hot100 q31_            # 只跑 Q31 的用例
./build/hot100 dp              # 命中 dp 与 dp_multidim 两组
```

输出示例（全绿）：

```
==> g++ 编译中（-std=c++17 -Wall -Wextra -O1）
==> 运行测试
[tricks]
    .....
[dp_multidim]
    .....
[dp]
    ..........
   …（按注册顺序倒序输出，一个点 = 一个用例）

99/99 test cases passed.
=> 全部通过
```

### 在 VS Code / Cursor 里打开

仓库带了 `.vscode/` 配置（编译器路径写的是 `C:/mingw64`，换机器改这三个文件里的路径即可）：

| 想干什么 | 怎么做 |
|---|---|
| 打开工程 | `code D:\CPP_Code\pi-test`，或 File → Open Folder 选这个目录 |
| 编译 + 跑测试 | `Ctrl+Shift+B`（跑 `build.sh`），或 `Terminal → Run Task…` 选 `cmake build + ctest` |
| 打断点调试 | 装 C/C++ 扩展后按 `F5`，选「只跑某一类 / 某一题」，弹出框里输过滤词（如 `q76`） |
| 看代码跳转 | 已配 `c_cpp_properties.json`（include 路径 + g++ 8.1 的标准库路径） |

> 调试必须让 `C:/mingw64/bin` 排在 PATH 最前，否则 exe 会加载到 Git for Windows 自带的
> `libwinpthread-1.dll`，gdb 启动时报 `0xc0000139`。`launch.json` 已用 `environment` 项处处理好了。

### 目录结构

```
.
├── include/
│   ├── common.h      # LeetCode 风格的 ListNode/TreeNode/Node + 建图建链/打印/释放工具
│   └── test.h        # 零依赖极简测试框架（自动注册 TEST、CHECK_EQ/TRUE/NEAR）
├── src/
│   ├── main.cpp      # 测试驱动器
│   ├── 01_hash.cpp   # 每类一个文件，文件名 = 序号 + 分类
│   ├── ...
│   └── 17_tricks.cpp
├── build.sh          # 不用 CMake 的一键编译脚本
└── CMakeLists.txt    # 同上，走 ctest
```

---

## 代码约定

1. **每题一个 `namespace lcNNNN`，里面只有一个 `class Solution`**
   —— 和力扣提交的类名、函数签名完全一致，可以直接整段复制粘贴到答题区（去掉 `using namespace std;`
   那行也行，力扣自带）。命名空间是为了让 100 个 `Solution` 能共存于一个可执行文件里。
2. **函数签名对齐力扣**（返回类型、参数个数、类型逐个核过）。
   只有 **12 道题**把入参从「按值 / 非 const 引用」改成 `const&`（纯粹为了测试里不用先拷贝一份），
   它们是：**Q4、Q5、Q20、Q32、Q34、Q35、Q72、Q131、Q139、Q394、Q763、Q1143**。
   粘到力扣时**不改也能编译**（`const string&` 能绑定 driver 传进来的实参），
   但若你习惯一字不差对齐题面，把这 12 处的 `const X&` 换回 `X`（string）或 `X&`（vector）即可。
3. **题解注释块固定结构**：

   ```cpp
   /*
    * Q42. 接雨水  Hard
    * https://leetcode.cn/problems/trapping-rain-water/
    *
    * 思路：…（为什么这样做是对的）
    * 复杂度：时间 O(n)，空间 O(1)
    * 易错：…（这一题 90% 的 bug 出在哪）
    * 变体：…（顺手带出同套路的其他题号）
    */
   ```
4. **测试用例名用 ASCII**（`TEST(binary_tree, q104_max_depth)`），这样命令行过滤不用处理中文编码。
5. 每题至少覆盖：**官方示例 + 边界（空/单元素/全相同）+ 一个反直觉的 case**。
   测试框架不带颜色依赖，失败时会打印 `文件:行号 + actual/expected`。

---

## 覆盖率

| 分类 | 题数 | Easy | Medium | Hard |
|---|---:|---:|---:|---:|
| 哈希 | 3 | 1 | 2 | 0 |
| 双指针 | 4 | 1 | 2 | 1 |
| 滑动窗口 | 2 | 0 | 2 | 0 |
| 子串 / 前缀和 | 3 | 0 | 1 | 2 |
| 普通数组 | 5 | 0 | 4 | 1 |
| 矩阵 | 4 | 0 | 4 | 0 |
| 链表 | 14 | 5 | 7 | 2 |
| 二叉树 | 15 | 6 | 8 | 1 |
| 图论 | 4 | 0 | 4 | 0 |
| 回溯 | 8 | 0 | 7 | 1 |
| 二分查找 | 6 | 1 | 4 | 1 |
| 栈 | 5 | 1 | 3 | 1 |
| 堆 | 3 | 0 | 2 | 1 |
| 贪心 | 4 | 1 | 3 | 0 |
| 动态规划（一维） | 10 | 2 | 7 | 1 |
| 多维动态规划 | 5 | 0 | 5 | 0 |
| 技巧 | 5 | 2 | 3 | 0 |
| **合计** | **100** | **20** | **68** | **12** |

题号、题面链接与难度均已与 `leetcode.cn` 官方接口逐条核对（100 题、100 个不同官方题号，无缺无重）。

---

## 100 题索引

### 01 · 哈希 (Hash)
源码 [`src/01_hash.cpp`](src/01_hash.cpp) — 3 题 / 3 个用例

| 题号 | 题目 | 难度 | 核心套路 | 官方题面 |
|:----:|---|:----:|---|---|
| **Q1** | 两数之和 | Easy | 哈希存「值→下标」，查 target-x | [two-sum](https://leetcode.cn/problems/two-sum/) |
| **Q49** | 字母异位词分组 | Medium | 排序后的串当分组 key | [group-anagrams](https://leetcode.cn/problems/group-anagrams/) |
| **Q128** | 最长连续序列 | Medium | set + 只在连续段起点向右扩 | [longest-consecutive-sequence](https://leetcode.cn/problems/longest-consecutive-sequence/) |

### 02 · 双指针 (Two Pointers)
源码 [`src/02_two_pointers.cpp`](src/02_two_pointers.cpp) — 4 题 / 4 个用例

| 题号 | 题目 | 难度 | 核心套路 | 官方题面 |
|:----:|---|:----:|---|---|
| **Q283** | 移动零 | Easy | 快慢指针：非零元素往前搬 | [move-zeroes](https://leetcode.cn/problems/move-zeroes/) |
| **Q11** | 盛最多水的容器 | Medium | 对撞指针，每次移动较矮的一侧 | [container-with-most-water](https://leetcode.cn/problems/container-with-most-water/) |
| **Q15** | 三数之和 | Medium | 排序 + 固定一个数 + 双指针，逐位去重 | [3sum](https://leetcode.cn/problems/3sum/) |
| **Q42** | 接雨水 | Hard | 双指针维护 leftMax / rightMax | [trapping-rain-water](https://leetcode.cn/problems/trapping-rain-water/) |

### 03 · 滑动窗口 (Sliding Window)
源码 [`src/03_sliding_window.cpp`](src/03_sliding_window.cpp) — 2 题 / 2 个用例

| 题号 | 题目 | 难度 | 核心套路 | 官方题面 |
|:----:|---|:----:|---|---|
| **Q3** | 无重复字符的最长子串 | Medium | 右扩，撞重复把 left 跳到重复位之后 | [longest-substring-without-repeating-characters](https://leetcode.cn/problems/longest-substring-without-repeating-characters/) |
| **Q438** | 找到字符串中所有字母异位词 | Medium | 定长窗口 + 计数比较，进出各 O(1) | [find-all-anagrams-in-a-string](https://leetcode.cn/problems/find-all-anagrams-in-a-string/) |

### 04 · 子串 / 前缀和 (Substring & Prefix Sum)
源码 [`src/04_substring.cpp`](src/04_substring.cpp) — 3 题 / 3 个用例

| 题号 | 题目 | 难度 | 核心套路 | 官方题面 |
|:----:|---|:----:|---|---|
| **Q560** | 和为 K 的子数组 | Medium | 前缀和 + 哈希计数（有负数不能用滑窗） | [subarray-sum-equals-k](https://leetcode.cn/problems/subarray-sum-equals-k/) |
| **Q239** | 滑动窗口最大值 | Hard | 单调递减双端队列，队头即窗口最大值 | [sliding-window-maximum](https://leetcode.cn/problems/sliding-window-maximum/) |
| **Q76** | 最小覆盖子串 | Hard | 滑窗 + need/window 计数，先扩后缩 | [minimum-window-substring](https://leetcode.cn/problems/minimum-window-substring/) |

### 05 · 普通数组 (Array)
源码 [`src/05_array.cpp`](src/05_array.cpp) — 5 题 / 5 个用例

| 题号 | 题目 | 难度 | 核心套路 | 官方题面 |
|:----:|---|:----:|---|---|
| **Q53** | 最大子数组和 | Medium | Kadane：cur 变负就从下一位重启 | [maximum-subarray](https://leetcode.cn/problems/maximum-subarray/) |
| **Q56** | 合并区间 | Medium | 按左端排序 + 合并相交区间 | [merge-intervals](https://leetcode.cn/problems/merge-intervals/) |
| **Q189** | 轮转数组 | Medium | 整体反转 + 两段各自反转 | [rotate-array](https://leetcode.cn/problems/rotate-array/) |
| **Q238** | 除自身以外数组的乘积 | Medium | 左右两遍前缀积，不用除法 | [product-of-array-except-self](https://leetcode.cn/problems/product-of-array-except-self/) |
| **Q41** | 缺失的第一个正数 | Hard | 下标即哈希桶：用正负号标记谁出现过 | [first-missing-positive](https://leetcode.cn/problems/first-missing-positive/) |

### 06 · 矩阵 (Matrix)
源码 [`src/06_matrix.cpp`](src/06_matrix.cpp) — 4 题 / 4 个用例

| 题号 | 题目 | 难度 | 核心套路 | 官方题面 |
|:----:|---|:----:|---|---|
| **Q73** | 矩阵置零 | Medium | 借第一行第一列当标记位，O(1) 空间 | [set-matrix-zeroes](https://leetcode.cn/problems/set-matrix-zeroes/) |
| **Q54** | 螺旋矩阵 | Medium | 四条边界收缩，走完一条砍一条 | [spiral-matrix](https://leetcode.cn/problems/spiral-matrix/) |
| **Q48** | 旋转图像 | Medium | 顺时针 = 转置 + 每行左右翻转 | [rotate-image](https://leetcode.cn/problems/rotate-image/) |
| **Q240** | 搜索二维矩阵 II | Medium | 从右上角出发，大了左移、小了下移 | [search-a-2d-matrix-ii](https://leetcode.cn/problems/search-a-2d-matrix-ii/) |

### 07 · 链表 (Linked List)
源码 [`src/07_linked_list.cpp`](src/07_linked_list.cpp) — 14 题 / 13 个用例

| 题号 | 题目 | 难度 | 核心套路 | 官方题面 |
|:----:|---|:----:|---|---|
| **Q141** | 环形链表 | Easy | Floyd 快慢指针，相遇即有环 | [linked-list-cycle](https://leetcode.cn/problems/linked-list-cycle/) |
| **Q142** | 环形链表 II | Medium | a=(k-1)(b+c)+c → 从头与从相遇点同速 | [linked-list-cycle-ii](https://leetcode.cn/problems/linked-list-cycle-ii/) |
| **Q160** | 相交链表 | Easy | 先对齐起点：长链表多走差值步 | [intersection-of-two-linked-lists](https://leetcode.cn/problems/intersection-of-two-linked-lists/) |
| **Q206** | 反转链表 | Easy | 三指针迭代反转 / 递归版 | [reverse-linked-list](https://leetcode.cn/problems/reverse-linked-list/) |
| **Q234** | 回文链表 | Easy | 快慢找中 + 反转后半 + 逐一比对 | [palindrome-linked-list](https://leetcode.cn/problems/palindrome-linked-list/) |
| **Q21** | 合并两个有序链表 | Easy | 哨兵头结点 + 逐一接较小者 | [merge-two-sorted-lists](https://leetcode.cn/problems/merge-two-sorted-lists/) |
| **Q2** | 两数相加 | Medium | 逐位相加，哨兵结点简化尾插 | [add-two-numbers](https://leetcode.cn/problems/add-two-numbers/) |
| **Q19** | 删除链表的倒数第 N 个结点 | Medium | 快指针先走 n 步，再同步走到尾部 | [remove-nth-node-from-end-of-list](https://leetcode.cn/problems/remove-nth-node-from-end-of-list/) |
| **Q24** | 两两交换链表中的节点 | Medium | dummy + 每次交换相邻一对 | [swap-nodes-in-pairs](https://leetcode.cn/problems/swap-nodes-in-pairs/) |
| **Q25** | K 个一组翻转链表 | Hard | 数够 k 个就整组反转，再串接 | [reverse-nodes-in-k-group](https://leetcode.cn/problems/reverse-nodes-in-k-group/) |
| **Q138** | 随机链表的复制 | Medium | 原地把 next 指向克隆结点，再拆两条链 | [copy-list-with-random-pointer](https://leetcode.cn/problems/copy-list-with-random-pointer/) |
| **Q148** | 排序链表 | Medium | 归并：快慢找中 + 递归 + 合并两链 | [sort-list](https://leetcode.cn/problems/sort-list/) |
| **Q23** | 合并 K 个升序链表 | Hard | 小顶堆按值合并 k 路 / 分治 | [merge-k-sorted-lists](https://leetcode.cn/problems/merge-k-sorted-lists/) |
| **Q146** | LRU 缓存 | Medium | 哈希表 + 双向链表，get/put 都 O(1) | [lru-cache](https://leetcode.cn/problems/lru-cache/) |

### 08 · 二叉树 (Binary Tree)
源码 [`src/08_binary_tree.cpp`](src/08_binary_tree.cpp) — 15 题 / 15 个用例

| 题号 | 题目 | 难度 | 核心套路 | 官方题面 |
|:----:|---|:----:|---|---|
| **Q94** | 二叉树的中序遍历 | Easy | 迭代栈 / Morris；递归就是左根右 | [binary-tree-inorder-traversal](https://leetcode.cn/problems/binary-tree-inorder-traversal/) |
| **Q104** | 二叉树的最大深度 | Easy | 递归 max(左高,右高)+1 | [maximum-depth-of-binary-tree](https://leetcode.cn/problems/maximum-depth-of-binary-tree/) |
| **Q226** | 翻转二叉树 | Easy | 递归交换左右孩子 | [invert-binary-tree](https://leetcode.cn/problems/invert-binary-tree/) |
| **Q101** | 对称二叉树 | Easy | 递归比较(左左,右右)与(左右,右左) | [symmetric-tree](https://leetcode.cn/problems/symmetric-tree/) |
| **Q543** | 二叉树的直径 | Easy | 后序：过每个结点的直径=左高+右高 | [diameter-of-binary-tree](https://leetcode.cn/problems/diameter-of-binary-tree/) |
| **Q102** | 二叉树的层序遍历 | Medium | BFS 按层出队，用 size 定层宽 | [binary-tree-level-order-traversal](https://leetcode.cn/problems/binary-tree-level-order-traversal/) |
| **Q108** | 将有序数组转换为二叉搜索树 | Easy | 取中点做根，左右两半递归 | [convert-sorted-array-to-binary-search-tree](https://leetcode.cn/problems/convert-sorted-array-to-binary-search-tree/) |
| **Q98** | 验证二叉搜索树 | Medium | 上下界区间收缩（只比父子会错） | [validate-binary-search-tree](https://leetcode.cn/problems/validate-binary-search-tree/) |
| **Q230** | 二叉搜索树中第 K 小的元素 | Medium | 中序遍历第 k 个 / BST 迭代器 | [kth-smallest-element-in-a-bst](https://leetcode.cn/problems/kth-smallest-element-in-a-bst/) |
| **Q199** | 二叉树的右视图 | Medium | BFS 每层最后一个结点 | [binary-tree-right-side-view](https://leetcode.cn/problems/binary-tree-right-side-view/) |
| **Q114** | 二叉树展开为链表 | Medium | 先序展开：左子树右端接右子树 | [flatten-binary-tree-to-linked-list](https://leetcode.cn/problems/flatten-binary-tree-to-linked-list/) |
| **Q105** | 从前序与中序遍历序列构造二叉树 | Medium | 前序定根 + 中序切两半（下标哈希加速） | [construct-binary-tree-from-preorder-and-inorder-traversal](https://leetcode.cn/problems/construct-binary-tree-from-preorder-and-inorder-traversal/) |
| **Q437** | 路径总和 III | Medium | 树上前缀和 + 哈希（Q560 的树版） | [path-sum-iii](https://leetcode.cn/problems/path-sum-iii/) |
| **Q236** | 二叉树的最近公共祖先 | Medium | 后序：两侧都有即答案，否则返回非空那侧 | [lowest-common-ancestor-of-a-binary-tree](https://leetcode.cn/problems/lowest-common-ancestor-of-a-binary-tree/) |
| **Q124** | 二叉树中的最大路径和 | Hard | DFS 只返回「单边贡献」，答案全局取 max | [binary-tree-maximum-path-sum](https://leetcode.cn/problems/binary-tree-maximum-path-sum/) |

### 09 · 图论 (Graph)
源码 [`src/09_graph.cpp`](src/09_graph.cpp) — 4 题 / 4 个用例

| 题号 | 题目 | 难度 | 核心套路 | 官方题面 |
|:----:|---|:----:|---|---|
| **Q200** | 岛屿数量 | Medium | 网格 DFS 洪水填充沉岛 | [number-of-islands](https://leetcode.cn/problems/number-of-islands/) |
| **Q994** | 腐烂的橘子 | Medium | 多源 BFS 按层扩散 + fresh 计数收尾 | [rotting-oranges](https://leetcode.cn/problems/rotting-oranges/) |
| **Q207** | 课程表 | Medium | 拓扑排序判环（Kahn / DFS 三色） | [course-schedule](https://leetcode.cn/problems/course-schedule/) |
| **Q208** | 实现 Trie（前缀树） | Medium | 26 叉树 + isEnd 区分词与前缀 | [implement-trie-prefix-tree](https://leetcode.cn/problems/implement-trie-prefix-tree/) |

### 10 · 回溯 (Backtracking)
源码 [`src/10_backtracking.cpp`](src/10_backtracking.cpp) — 8 题 / 8 个用例

| 题号 | 题目 | 难度 | 核心套路 | 官方题面 |
|:----:|---|:----:|---|---|
| **Q46** | 全排列 | Medium | used[] 标记，选满 n 个即收集 | [permutations](https://leetcode.cn/problems/permutations/) |
| **Q78** | 子集 | Medium | 每个结点都是答案；传 start 防重复 | [subsets](https://leetcode.cn/problems/subsets/) |
| **Q17** | 电话号码的字母组合 | Medium | 多叉树按数字逐层展开 | [letter-combinations-of-a-phone-number](https://leetcode.cn/problems/letter-combinations-of-a-phone-number/) |
| **Q39** | 组合总和 | Medium | 组合回溯，递归传 i 允许重复选 | [combination-sum](https://leetcode.cn/problems/combination-sum/) |
| **Q22** | 括号生成 | Medium | open<n 才能加左，close<open 才能加右 | [generate-parentheses](https://leetcode.cn/problems/generate-parentheses/) |
| **Q79** | 单词搜索 | Medium | 网格 DFS + 占位符标记，回溯时还原 | [word-search](https://leetcode.cn/problems/word-search/) |
| **Q131** | 分割回文串 | Medium | 区间 DP 预处理回文表 + 切分回溯 | [palindrome-partitioning](https://leetcode.cn/problems/palindrome-partitioning/) |
| **Q51** | N 皇后 | Hard | 逐行放皇后，cols / 两条 diag 三个标记 | [n-queens](https://leetcode.cn/problems/n-queens/) |

### 11 · 二分查找 (Binary Search)
源码 [`src/11_binary_search.cpp`](src/11_binary_search.cpp) — 6 题 / 6 个用例

| 题号 | 题目 | 难度 | 核心套路 | 官方题面 |
|:----:|---|:----:|---|---|
| **Q35** | 搜索插入位置 | Easy | 手写 lower_bound，返回插入位 | [search-insert-position](https://leetcode.cn/problems/search-insert-position/) |
| **Q74** | 搜索二维矩阵 | Medium | 矩阵拉平成 [0,mn) 做一维二分 | [search-a-2d-matrix](https://leetcode.cn/problems/search-a-2d-matrix/) |
| **Q34** | 在排序数组中查找元素的第一个和最后一个位置 | Medium | 两次边界：lower_bound 与 upper_bound | [find-first-and-last-position-of-element-in-sorted-array](https://leetcode.cn/problems/find-first-and-last-position-of-element-in-sorted-array/) |
| **Q33** | 搜索旋转排序数组 | Medium | 先判断哪一半有序，再决定往哪收缩 | [search-in-rotated-sorted-array](https://leetcode.cn/problems/search-in-rotated-sorted-array/) |
| **Q153** | 寻找旋转排序数组中的最小值 | Medium | 与右端点比较收缩，答案落在 lo | [find-minimum-in-rotated-sorted-array](https://leetcode.cn/problems/find-minimum-in-rotated-sorted-array/) |
| **Q4** | 寻找两个正序数组的中位数 | Hard | 只在短数组上枚举合法切点 | [median-of-two-sorted-arrays](https://leetcode.cn/problems/median-of-two-sorted-arrays/) |

### 12 · 栈 (Stack)
源码 [`src/12_stack.cpp`](src/12_stack.cpp) — 5 题 / 5 个用例

| 题号 | 题目 | 难度 | 核心套路 | 官方题面 |
|:----:|---|:----:|---|---|
| **Q20** | 有效的括号 | Easy | 左括号入栈，右括号配栈顶 + 结束判空 | [valid-parentheses](https://leetcode.cn/problems/valid-parentheses/) |
| **Q155** | 最小栈 | Medium | 辅助最小栈同步压入/弹出 | [min-stack](https://leetcode.cn/problems/min-stack/) |
| **Q394** | 字符串解码 | Medium | 双栈处理嵌套：数字栈 + 字符串栈 | [decode-string](https://leetcode.cn/problems/decode-string/) |
| **Q739** | 每日温度 | Medium | 单调递减栈求右向第一个更大 | [daily-temperatures](https://leetcode.cn/problems/daily-temperatures/) |
| **Q84** | 柱状图中最大的矩形 | Hard | 单调递增栈 + 首尾 0 哨兵结算宽度 | [largest-rectangle-in-histogram](https://leetcode.cn/problems/largest-rectangle-in-histogram/) |

### 13 · 堆 (Heap / Priority Queue)
源码 [`src/13_heap.cpp`](src/13_heap.cpp) — 3 题 / 3 个用例

| 题号 | 题目 | 难度 | 核心套路 | 官方题面 |
|:----:|---|:----:|---|---|
| **Q215** | 数组中的第 K 个最大元素 | Medium | K 个小顶堆 / 随机化快速选择 | [kth-largest-element-in-an-array](https://leetcode.cn/problems/kth-largest-element-in-an-array/) |
| **Q347** | 前 K 个高频元素 | Medium | 按频次桶排序（值域有界比堆更快） | [top-k-frequent-elements](https://leetcode.cn/problems/top-k-frequent-elements/) |
| **Q295** | 数据流的中位数 | Hard | 对顶堆：大顶装小半、小顶装大半 | [find-median-from-data-stream](https://leetcode.cn/problems/find-median-from-data-stream/) |

### 14 · 贪心 (Greedy)
源码 [`src/14_greedy.cpp`](src/14_greedy.cpp) — 4 题 / 4 个用例

| 题号 | 题目 | 难度 | 核心套路 | 官方题面 |
|:----:|---|:----:|---|---|
| **Q121** | 买卖股票的最佳时机 | Easy | 边走边记历史最低价，当天价减它取最大 | [best-time-to-buy-and-sell-stock](https://leetcode.cn/problems/best-time-to-buy-and-sell-stock/) |
| **Q55** | 跳跃游戏 | Medium | 贪心维护最远可达 reach，不可达即失败 | [jump-game](https://leetcode.cn/problems/jump-game/) |
| **Q45** | 跳跃游戏 II | Medium | BFS 分层视角：走到边界才 +1 步 | [jump-game-ii](https://leetcode.cn/problems/jump-game-ii/) |
| **Q763** | 划分字母区间 | Medium | 记录每个字母末次位置 + 区间合并 | [partition-labels](https://leetcode.cn/problems/partition-labels/) |

### 15 · 动态规划 (Dynamic Programming) —— 一维
源码 [`src/15_dp.cpp`](src/15_dp.cpp) — 10 题 / 10 个用例

| 题号 | 题目 | 难度 | 核心套路 | 官方题面 |
|:----:|---|:----:|---|---|
| **Q70** | 爬楼梯 | Easy | dp[i]=dp[i-1]+dp[i-2]，滚动两变量 | [climbing-stairs](https://leetcode.cn/problems/climbing-stairs/) |
| **Q118** | 杨辉三角 | Easy | tri[i][j]=上一行相邻两数之和 | [pascals-triangle](https://leetcode.cn/problems/pascals-triangle/) |
| **Q198** | 打家劫舍 | Medium | 偷/不偷：max(prev1, prev2+x) | [house-robber](https://leetcode.cn/problems/house-robber/) |
| **Q279** | 完全平方数 | Medium | 完全背包求最少平方数个数 | [perfect-squares](https://leetcode.cn/problems/perfect-squares/) |
| **Q322** | 零钱兑换 | Medium | 完全背包求最少硬币（贪心必错） | [coin-change](https://leetcode.cn/problems/coin-change/) |
| **Q139** | 单词拆分 | Medium | dp[i]=前 i 个能否拆成词典词（布尔背包） | [word-break](https://leetcode.cn/problems/word-break/) |
| **Q300** | 最长递增子序列 | Medium | tails[k]=长度 k 的最小结尾 + lower_bound | [longest-increasing-subsequence](https://leetcode.cn/problems/longest-increasing-subsequence/) |
| **Q152** | 乘积最大子数组 | Medium | 同时维护最大积与最小积，遇负交换 | [maximum-product-subarray](https://leetcode.cn/problems/maximum-product-subarray/) |
| **Q416** | 分割等和子集 | Medium | 0-1 背包判定，一维数组必须逆序 | [partition-equal-subset-sum](https://leetcode.cn/problems/partition-equal-subset-sum/) |
| **Q32** | 最长有效括号 | Hard | 栈底存最后一个破坏位 / 也可区间 DP | [longest-valid-parentheses](https://leetcode.cn/problems/longest-valid-parentheses/) |

### 16 · 多维动态规划 (2D DP)
源码 [`src/16_dp_multidim.cpp`](src/16_dp_multidim.cpp) — 5 题 / 5 个用例

| 题号 | 题目 | 难度 | 核心套路 | 官方题面 |
|:----:|---|:----:|---|---|
| **Q62** | 不同路径 | Medium | dp[j]+=dp[j-1] 滚动一行；等价 C(m+n-2,m-1) | [unique-paths](https://leetcode.cn/problems/unique-paths/) |
| **Q64** | 最小路径和 | Medium | 格值 + min(上,左)，首行首列单独初始化 | [minimum-path-sum](https://leetcode.cn/problems/minimum-path-sum/) |
| **Q5** | 最长回文子串 | Medium | 枚举 2n-1 个中心向两边扩（奇偶都要） | [longest-palindromic-substring](https://leetcode.cn/problems/longest-palindromic-substring/) |
| **Q1143** | 最长公共子序列 | Medium | 相等取对角+1，不等取 max(上,左) | [longest-common-subsequence](https://leetcode.cn/problems/longest-common-subsequence/) |
| **Q72** | 编辑距离 | Medium | 替换/删除/插入三格取最小 +1 | [edit-distance](https://leetcode.cn/problems/edit-distance/) |

### 17 · 技巧 (Techs)
源码 [`src/17_tricks.cpp`](src/17_tricks.cpp) — 5 题 / 5 个用例

| 题号 | 题目 | 难度 | 核心套路 | 官方题面 |
|:----:|---|:----:|---|---|
| **Q136** | 只出现一次的数字 | Easy | 全体异或，成对的自动抵消 | [single-number](https://leetcode.cn/problems/single-number/) |
| **Q169** | 多数元素 | Easy | 摩尔投票，O(1) 空间 | [majority-element](https://leetcode.cn/problems/majority-element/) |
| **Q75** | 颜色分类 | Medium | 荷兰国旗三向切分；遇 2 交换后 i 不动 | [sort-colors](https://leetcode.cn/problems/sort-colors/) |
| **Q31** | 下一个排列 | Medium | 找下降位→换「刚好更大」的→反转后缀 | [next-permutation](https://leetcode.cn/problems/next-permutation/) |
| **Q287** | 寻找重复数 | Medium | 值当下一跳构成链表 → Floyd 找环入口 | [find-the-duplicate-number](https://leetcode.cn/problems/find-the-duplicate-number/) |
---

## 建议用法（三轮刷法）

**第一轮 · 只看注释不看代码。** 读某分类文件开头的模板总结，然后盖住实现，
自己口头说出这题的思路和「坑在哪」，说不出来再看注释。Hot 100 的价值在于**一类题一个模板**，
不是 100 个孤立技巧。

**第二轮 · 自己手写实现，用测试验证。** 把某题的 `Solution` 删掉重写（或直接在力扣写），
回来跑 `./build/hot100 q152_` 之类的过滤，用例不过就是真错了 —— 这里没有「感觉对了」。
开发过程中测试用例本身错过 5 次，而这 5 次恰好暴露了最容易想错的点：
Q94 把 `[1,null,2,3]` 的中序写成 `[1,2,3]`（3 是 2 的左孩子，应为 `[1,3,2]`）、
Q437 用「根到叶」的答案去套「任意起点」（1 条 vs 2 条）、
Q152 以为 `[-4,-3,-2]` 能取到 24（实际 12，三个一起乘变负）、
Q560 前缀和计数题的期望值算错、Q287 给了不满足值域约束的输入直接段错误。

**第三轮 · 按套路横向串题。** 索引表最后一列就是「套路标签」，同标签的题连起来做：

| 套路 | 连着做这几题 |
|---|---|
| 对撞双指针 | Q11 → Q15 → Q42 |
| 滑动窗口（变长/定长） | Q3 → Q438 → Q76 → Q560（对比：有负数只能前缀和） |
| 单调栈 | Q739 → Q84 →（回去看 Q42 的单调栈解法） |
| 快慢指针 / 改链表指针 | Q206 → Q141 → Q142 → Q19 → Q160 → Q234 |
| 前缀和 + 哈希 | Q560 → Q437（树版）→ Q1（同一张哈希表的三种用法） |
| 二分找边界 | Q35 → Q34 → Q74 → Q33 → Q153 |
| BFS 分层 | Q102 → Q199 → Q994 → Q207（拓扑排序就是分层） |
| 回溯三行模板 | Q46 → Q78 → Q39 → Q22 → Q131 → Q51 |
| 完全背包 vs 0-1 背包 | Q322 / Q279（正序）↔ Q416（逆序）—— 一个内层顺序之差就是错题库之首 |
| 树形 DP「返回值 ≠ 答案」 | Q543 → Q124 → Q236（后序返回值到底代表什么） |
| 一维滚动降维 | Q62 → Q64 → Q1143 → Q72 |
| 对顶堆 / 第 K 大 | Q215 → Q347 → Q295 |

---

## 值得单独背下来的 12 个坑

从 100 题的注释里挑出来的高频错误，每条都对应至少一道会写错的题：

1. **`mid = lo + (hi - lo) / 2`** —— 防 `(lo+hi)` 溢出（Q35 及所有二分题）。
2. **区间语义要统一**：`[lo, hi]` 闭区间配 `hi = mid - 1`；`[lo, hi)` 半开配 `hi = mid`。混用 = 死循环。
3. **一维 0-1 背包内层必须逆序**（Q416）；完全背包正序（Q322/Q279）。正序写错会让同一个数用两次。
4. **「以 i 结尾」型 DP，答案是 `max(dp[i])` 而不是 `dp[n-1]`**（Q300、Q152、Q53）。
5. **乘积最大子数组初值必须是 `nums[0]`**，写 0 会在全负数组上返回 0（Q152）。
6. **Q437 是「任意起点向下」的路径**，不是根到叶；所以要用前缀和 + 哈希，且 `presum[0]=1` 要先塞进表。
7. **验证 BST（Q98）要比上下界**，只比较父子结点会被 `[5,4,6,null,null,3,7]` 打穿。
8. **链表改指针前先存 `next`，且用 dummy 头结点**统一处理「删/换的是头结点」（Q19、Q24、Q25）。
9. **荷兰国旗遇到 2 时与 `p2` 交换后 `i` 不能自增**（Q75）—— 换回来的值还没看过。
10. **Q287 的输入必须满足「长度 n+1、值域 [1,n]」**，否则快指针真的会越界（本地写完测试直接段错误）。
11. **`unordered_map` 的 `operator[]` 会插入默认值**，判存在用 `count()` / `find()`（Q1、Q560、Q437）。
12. **`std::priority_queue` 默认是大顶堆**，要小顶堆得写满三个模板参数（Q23、Q215、Q347）。

---

## 测试框架

`include/test.h` 是 ~120 行的极简框架（无第三方依赖）：

```cpp
TEST(hash, q1_two_sum) {          // 自动注册，不用手改任何列表
  lc0001::Solution s;
  vector<int> a{2, 7, 11, 15};
  CHECK_EQ(s.twoSum(a, 9), vector<int>{0, 1});
  CHECK_TRUE(a.size() == 4);
  CHECK_NEAR(2.5, 2.5, 1e-9);     // 浮点
}
```

- 用例体抛异常会被捕获并继续跑后面的用例，一个错误不会中断整轮。
- `CHECK_*` 宏故意做成变参，`CHECK_EQ(x, vector<int>{0,1})` 里的逗号不会把宏参数劈断。
- `lc::toStr` 支持标量 / `string` / `vector` / `pair` / `optional` 并可递归嵌套（`vector<vector<int>>`
  会打成 `[[1, 2], [3]]`），失败信息可读。
- 退出码：全过 0；有失败或**过滤条件没匹配到任何用例**都是 1（方便接 CI）。
- 输出是行缓冲，万一某个用例把进程跑崩，崩溃前的进度仍然看得到。
- 想加自己的题：新建 `src/18_xxx.cpp`（CMake 用 `GLOB_RECURSE`，`build.sh` 用通配，都会自动吃到），
  照抄一个 `namespace lcNNNN` + `TEST(...)` 的壳即可。

## 环境

- 语言：**C++17**（用了结构化绑定、`if` 初始化语句、`std::optional`）。
- 实测编译：MinGW **g++ 8.1.0** `-Wall -Wextra` 零警告通过；CMake 4.3.3 + `mingw32-make`；ctest 全绿。
- Linux/macOS 下把编译器换成 clang/g++ 一般无需改动（没有平台相关代码）。
- 测试是纯计算 + 内存操作，**不联网、不读文件**。

## 许可与说明

代码为本仓库原创题解，注释里标注了每题的官方题面链接与难度；题号、名称来自 LeetCode。
如果某处签名与题面不同（为了可测性把入参改成 `const&`），注释中会写明「提交时按题面改回」。
