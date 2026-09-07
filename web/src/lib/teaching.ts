import type { Problem } from './types';

export const categoryMeta: Record<string, { label: string; why: string; color: string }> = {
  hash: { label: '哈希', color: 'violet', why: '把“查找另一个值”从重复扫描变成常数时间查询。' },
  two_pointers: { label: '双指针', color: 'cyan', why: '利用有序性或两端信息，让两个指针只朝答案收缩。' },
  sliding_window: { label: '滑动窗口', color: 'cyan', why: '维护一个始终满足约束的连续区间，避免重复计算。' },
  substring: { label: '子串', color: 'cyan', why: '用前缀信息或窗口把连续区间问题拆成可复用的状态。' },
  array: { label: '数组', color: 'orange', why: '围绕下标、原地状态和局部不变量组织一次或有限次扫描。' },
  matrix: { label: '矩阵', color: 'orange', why: '把二维坐标关系转成有边界的行列遍历或原地标记。' },
  linked_list: { label: '链表', color: 'pink', why: '重点是指针不丢失：先保存 next，再改变连接关系。' },
  binary_tree: { label: '二叉树', color: 'pink', why: '每个节点都问同一个递归问题，再把左右子树答案合并。' },
  graph: { label: '图论', color: 'pink', why: '把“连通、依赖、扩散”转成 DFS、BFS 或拓扑状态。' },
  backtracking: { label: '回溯', color: 'violet', why: '通过选择、递归、撤销选择枚举搜索树，并尽早剪枝。' },
  binary_search: { label: '二分', color: 'cyan', why: '只要答案具有单调性，就能每次排除一半候选区间。' },
  stack: { label: '栈', color: 'orange', why: '栈保存“最近还没解决”的状态，适合嵌套和单调关系。' },
  heap: { label: '堆', color: 'orange', why: '只维护当前最需要的少量候选，不必对全部数据排序。' },
  greedy: { label: '贪心', color: 'green', why: '每一步做经过证明的局部最优选择，把全局问题逐步缩小。' },
  dp: { label: '动态规划', color: 'green', why: '把大问题写成状态，并让每个状态只算一次。' },
  dp_multidim: { label: '二维 DP', color: 'green', why: '明确 dp[i][j] 的含义，再按依赖方向填表。' },
  tricks: { label: '技巧', color: 'violet', why: '借助位运算、数学规律或原地映射，压缩额外空间和枚举。' },
};

export function inlineMarkdown(text: string): string {
  const escaped = text
    .replaceAll('&', '&amp;')
    .replaceAll('<', '&lt;')
    .replaceAll('>', '&gt;');
  return escaped
    .replace(/`([^`]+)`/g, '<code>$1</code>')
    .replace(/\*\*([^*]+)\*\*/g, '<strong>$1</strong>');
}

export function teaching(problem: Problem) {
  const lines = problem.body
    .map((line) => line.replace(/^[-·]\s*/, '').trim())
    .filter((line) => line && line !== '/');
  const complexity = lines.find((line) => /复杂度|O\(/.test(line)) ?? '复杂度请以本题 C++ 注释中的分析为准。';
  const tips = lines.filter((line) => /易错|注意|保证|对比|延伸|为什么|避免|不要/.test(line));
  const steps = lines.filter((line) => line !== complexity && !tips.includes(line));
  const meta = categoryMeta[problem.category] ?? { label: problem.category, color: 'violet', why: '先识别题目的数据关系，再选择能维护该关系的不变量。' };
  return {
    meta,
    complexity,
    tips: tips.slice(0, 3),
    steps: steps.slice(0, 6),
    takeaway: problem.tag || '先写清状态与不变量，再让循环或递归始终维护它。',
  };
}
