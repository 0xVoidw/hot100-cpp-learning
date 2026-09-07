#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gen_animations.py — 生成 算法动图 脚本库 data/animations.json。
每个脚本被 site 里的动画引擎 LCAnim(anim.js) 播放。静态图仍保留。

帧格式（配套 tools/anim.js）：
  { 'kind':'array'|'grid'|'list'|'tree', 'data':..., 'frames':[{note, paint:[...]}] }
  paint: {f:'box',i,c}   数组方格        {f:'cell',r,c,c}  网格单元
         {f:'node',i,c}  链表结点        {f:'tn',id,c}     树结点
         {f:'pt',k,i,color,label}    数组指针   {f:'npt',k,i,color,label}  链表指针
         {f:'pt',k,r,c,...}           网格指针
"""
import os, json

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
OUT = os.path.join(ROOT, 'data', 'animations.json')

HL='#ffe9a8'; DONE='#cdeccd'; HOT='#ffd1cc'; B='#4f6ef7'; R='#e2554d'; P='#8a5cf6'

def box(i,c=HL): return {'f':'box','i':i,'c':c}
def cell(r,c,cc=HL): return {'f':'cell','r':r,'col':c,'color':cc}
def node(i,c=HL): return {'f':'node','i':i,'c':c}
def tn(id,c=HL): return {'f':'tn','id':id,'c':c}
def pt(k,i,label=None,color=B): return {'f':'pt','k':k,'i':i,'label':label or k,'color':color}
def npt(k,i,label=None,color=B): return {'f':'npt','k':k,'i':i,'label':label or k,'color':color}
def stat(*pairs): return [{'label': str(k), 'value': str(v)} for k, v in pairs]
def fr(note,*p,values=None,state=None):
    d = {'note':note, 'paint':list(p)}
    if values is not None: d['values'] = values
    if state is not None: d['state'] = state
    return d
def A(vals,frs): return {'kind':'array','data':vals,'frames':frs}
def G(rows,frs): return {'kind':'grid','data':rows,'frames':frs}
def L(vals,frs,cyc=None):
    d={'kind':'list','data':vals,'frames':frs}
    if cyc is not None: d['cycle']=cyc
    return d

ANIMS = {}

# ---------- 数组 / 双指针 ----------
ANIMS[1] = {'kind':'array','data':[2,7,11,15],'frames':[
  fr('数组 [2,7,11,15]，目标 target=9。用哈希表存「值→下标」，从 i=0 开始扫描。', box(0,HL), pt('i',0,color=P)),
  fr('i=0：查 target-2=7，不在表里。把 2→下标0 记入 seen。', box(0,DONE), pt('i',0,color=P)),
  fr('i=1：查 target-7=2，seen 里已存在 2→下标0，命中！', box(0,DONE), box(1,HL), pt('i',1,color=P)),
  fr('返回下标 [0,1]。顺序「先查后存」保证不会用同一个元素两次。', box(0,DONE), box(1,DONE)),
]}
ANIMS[11] = {'kind':'array','data':[1,8,6,2,5,4,8,3,7],'frames':[
  fr('两根柱子之间能盛的水 = 宽度 × min(左高,右高)。让 L=0，R=8 从两端出发。', box(0,HL), box(8,HL), pt('L',0,color=B), pt('R',8,color=R)),
  fr('L=0, R=8：宽=8，高=min(1,7)=1，面积=8。移动「偏矮」的那一侧，这回 L。', box(0,HL), box(8,HL), pt('L',1,color=B), pt('R',8,color=R)),
  fr('L=1, R=8：宽=7，高=min(8,7)=7，面积=49（目前最大）。矮的是 R，往左移。', box(1,HL), box(8,HL), pt('L',1,color=B), pt('R',7,color=R)),
  fr('L=1, R=7：宽=6，高=min(8,3)=3，面积=18。矮的是 R(3)，继续左移。', box(1,HL), box(7,HL), pt('L',1,color=B), pt('R',6,color=R)),
  fr('L=1, R=6：宽=5，高=min(8,8)=8，面积=40。两侧一样高，移动任一。', box(1,HL), box(6,HL), pt('L',2,color=B), pt('R',6,color=R)),
  fr('始终只移动偏矮一侧，最大值 49 保持到最后。', box(1,HL), box(8,HL), pt('L',1,color=B), pt('R',8,color=R)),
]}
ANIMS[3] = {'kind':'array','data':list('abcabcbb'),'frames':[
  fr('滑动窗口 [L,R)，右指针扩张。初始 L=0,R=0，窗口 "a"。', box(0,HL), pt('L',0,color=B), pt('R',0,color=R)),
  fr('R 右移 → 窗口 "ab"（下标 0..1）。', box(0,HL), box(1,HL), pt('L',0,color=B), pt('R',1,color=R)),
  fr('再扩 → "abc"（0..2），没有重复，继续。', box(0,HL), box(1,HL), box(2,HL), pt('L',0,color=B), pt('R',2,color=R)),
  fr('R 到下标3 的 a 与窗口里的 a 重复 → L 跳到该重复位之后（下标1）。窗口变 "bca"。', box(1,HL), box(2,HL), box(3,HL), pt('L',1,color=B), pt('R',3,color=R)),
  fr('R 继续扩，维护窗口内无重复。最长窗口长度 = 3（如 "abc"/"bca"/"cab"）。', box(1,HL), box(2,HL), box(3,HL), pt('L',1,color=B), pt('R',4,color=R)),
]}
ANIMS[560] = {'kind':'array','data':[1,1,1],'frames':[
  fr('数组 [1,1,1]，k=2。用「前缀和 + 计数」：pre 每加一个，统计前面有多少 pre-k。', box(0,HL), pt('j',0,color=P)),
  fr('j=0：pre=1，mp[1]++ → 1。 ', box(0,HL), pt('j',0,color=P)),
  fr('j=1：pre=2，mp 里有 pre-k=0（1 次）→ ans=1；mp[2]++。', box(1,HL), pt('k',1,color=P)),
  fr('j=2：pre=3，mp 里有 pre-k=1（1 次）→ ans=2；mp[3]++。', box(2,HL), pt('k',2,color=P)),
  fr('总数 = 2（子数组 [1,1]（前两个）和 [1,1]（后两个））。', box(0,HL), box(1,HL), box(2,HL)),
]}
ANIMS[283] = {'kind':'array','data':[0,1,0,3,12],'frames':[
  fr('把非零数往左挪：pos 记录下一个应放非零的位置，遍历 i。', pt('pos',0,color=P), pt('i',0,color=B), box(0,HOT)),
  fr('i=0 是 0，跳过（pos 不动）。', pt('pos',0,color=P), pt('i',1,color=B), box(0,HOT)),
  fr('i=1 是 1：与 nums[pos=0] 交换 → 1 到前面；pos 和 i 都 +1。', box(0,DONE), box(1,HL), pt('pos',1,color=P), pt('i',2,color=B)),
  fr('i=2 是 0，跳过。', box(0,DONE), box(1,HL), box(2,HOT), pt('pos',1,color=P), pt('i',3,color=B)),
  fr('i=3 是 3：与 nums[pos=1]（0）交换 → 3 到前面；pos→2, i→4。', box(0,DONE), box(1,DONE), box(2,HL), box(3,HL), pt('pos',2,color=P), pt('i',4,color=B)),
  fr('i=4 是 12：与 nums[pos=2]（0）交换 → 12 到前面。结果 [1,3,12,0,0]。', box(0,DONE), box(1,DONE), box(2,DONE), box(3,DONE), pt('pos',3,color=P)),
]}
ANIMS[75] = {'kind':'array','data':[2,0,2,1,1,0],'frames':[
  fr('荷兰国旗：三段 [0 | 1 | 2]。zero、i 从 0 走，two 从末尾走。', pt('zero',0,color=B), pt('i',0,color=P), pt('two',5,color=R)),
  fr('nums[i]=2 → 与 nums[two] 交换，two 左移（i 不动，等换回来再看）。', box(5,HOT), pt('zero',0,color=B), pt('i',0,color=P), pt('two',4,color=R)),
  fr('nums[i]=0 → 与 nums[zero] 交换，zero 和 i 都右移。', box(0,DONE), pt('zero',1,color=B), pt('i',1,color=P), pt('two',4,color=R)),
  fr('…逐步归位 0、1、2，最终 [0,0,1,1,2,2]。', box(0,DONE), box(1,DONE), box(2,DONE), box(3,DONE), box(4,DONE), box(5,DONE)),
]}
ANIMS[33] = {'kind':'array','data':[4,5,6,7,0,1,2],'frames':[
  fr('搜索旋转数组：先判断 mid 落在哪一段「有序」上。lo=0, hi=6, mid=3。', box(3,HL), pt('lo',0,color=B), pt('hi',6,color=R), pt('mid',3,color=P)),
  fr('nums[mid]=7 ≥ nums[lo]，说明左段 [4..7] 有序；target=0 不在其中 → 去右边找。', box(3,HL), pt('lo',4,color=B), pt('hi',6,color=R), pt('mid',5,color=P)),
  fr('nums[mid]=1 < nums[lo]=5，右段 [..1..2] 有序；target=0 < 1 → 往左半。', box(5,HL), pt('lo',4,color=B), pt('hi',4,color=R), pt('mid',4,color=P)),
  fr('找到 nums[4]=0 = target。返回 4。', box(4,DONE), pt('lo',4,color=B), pt('hi',4,color=R), pt('mid',4,color=P)),
]}

# ---------- 链表 ----------
ANIMS[206] = {'kind':'list','data':[1,2,3,4,5],'frames':[
  fr('反转链表：prev=null，cur=head(1)。', npt('cur',0,color=P), node(0,HL)),
  fr('把 cur.next 指向 prev(null)。pre 和 cur 各前进一步。', npt('cur',1,color=P), npt('prev',0,color=B), node(0,DONE)),
  fr('把 2 的 next 指向 1。', npt('cur',2,color=P), npt('prev',1,color=B), node(0,DONE), node(1,DONE)),
  fr('把 3 的 next 指向 2。', npt('cur',3,color=P), npt('prev',2,color=B), node(0,DONE), node(1,DONE), node(2,DONE)),
  fr('继续到末尾：5→4→3→2→1，反转完成，返回 prev（新头 5）。', npt('cur',4,color=P), npt('prev',3,color=B), node(0,DONE), node(1,DONE), node(2,DONE), node(3,DONE)),
]}
ANIMS[141] = {'kind':'list','data':[3,2,0,-4],'cycle':1,'frames':[
  fr('Floyd 快慢指针：slow 每次 1 步，fast 每次 2 步。都从头出发。', npt('S',0,color=B), npt('F',0,color=R)),
  fr('slow→1，fast→2。', npt('S',1,color=B), npt('F',2,color=R)),
  fr('slow→2，fast→0（绕环一周）。', npt('S',2,color=B), npt('F',0,color=R)),
  fr('slow→0，fast→2。', npt('S',0,color=B), npt('F',2,color=R)),
  fr('slow→1，fast→…它们会相遇 → 有环，返回 true。', npt('S',2,color=B), npt('F',0,color=R), node(0,HL)),
]}

# ---------- 树 ----------
ANIMS[102] = {'kind':'tree','data':[3,9,20,15,7],'frames':[
  fr('层序遍历 BFS：先根节点 3。', tn(0,HL)),
  fr('再第二层 9、20。', tn(0,DONE), tn(1,HL), tn(2,HL)),
  fr('再第三层 15、7。结果 [3],[9,20],[15,7]。', tn(0,DONE), tn(1,DONE), tn(2,DONE), tn(3,HL), tn(4,HL)),
]}
ANIMS[94] = {'kind':'tree','data':[1,None,2,3],'frames':[
  fr('中序遍历：左→根→右。先一路向左走到头。', tn(0,HL)),
  fr('访问 1。', tn(0,DONE)),
  fr('返回到 2，先访问其左孩子 3。', tn(2,HL), tn(3,HL)),
  fr('访问 3 再访问 2。序列：1,3,2。', tn(0,DONE), tn(3,DONE), tn(2,DONE)),
]}

# ---------- 网格 / 图 ----------
ANIMS[62] = {'kind':'grid','data':[[1,1,1,1,1,1,1],[1,2,3,4,5,6,7],[1,3,6,10,15,21,28]],'frames':[
  fr('只能向右/向下走。dp[i][j] = 上面 + 左边。', cell(0,0,HL)),
  fr('顶行、左列都是 1。', cell(0,0,DONE), cell(0,1,DONE), cell(0,2,DONE), cell(0,3,DONE), cell(0,4,DONE), cell(0,5,DONE), cell(0,6,DONE), cell(1,0,DONE), cell(2,0,DONE)),
  fr('填格子：dp[1][1] = dp[0][1]+dp[1][0] = 1+1 = 2。', cell(1,1,HL), cell(0,1,DONE), cell(1,0,DONE)),
  fr('右下角 dp[2][6] = 28，即 3×7 网格的不同路径数。', cell(2,6,HL)),
]}
ANIMS[200] = {'kind':'grid','data':[['1','1','1','1','0'],['1','1','0','1','0'],['1','1','0','0','0'],['0','0','0','0','0']],'frames':[
  fr('从 (0,0) 的 1 出发做 DFS（或 BFS），把连通的 1 全标记成 0。', cell(0,0,HL)),
  fr('扩散相连的 1 → 第一片岛屿。', cell(0,0,DONE), cell(0,1,DONE), cell(0,2,DONE), cell(0,3,DONE), cell(1,0,DONE), cell(1,1,DONE), cell(1,3,DONE), cell(2,0,DONE), cell(2,1,DONE)),
  fr('扫描到下一个未访问的 1（已没有）→ 岛屿总数 = 1。', cell(0,0,DONE), cell(0,1,DONE), cell(0,2,DONE), cell(0,3,DONE), cell(1,0,DONE), cell(1,1,DONE), cell(1,3,DONE), cell(2,0,DONE), cell(2,1,DONE)),
]}
ANIMS[994] = {'kind':'grid','data':[[2,1,1],[1,1,0],[0,1,1]],'frames':[
  fr('多源 BFS：所有腐烂橘子(2)同时作为起点。', cell(0,0,HOT)),
  fr('第 1 分钟：相邻的橘子腐烂 (0,1)、(1,0)。', cell(0,0,DONE), cell(0,1,HOT), cell(1,0,HOT)),
  fr('第 2 分钟：(1,1)、(2,1) 腐烂。', cell(0,0,DONE), cell(0,1,DONE), cell(1,0,DONE), cell(1,1,HOT), cell(2,1,HOT)),
  fr('第 3 分钟：(2,2) 腐烂。全部新鲜橘子腐烂完，耗时 3 分钟。', cell(0,0,DONE), cell(0,1,DONE), cell(1,0,DONE), cell(1,1,DONE), cell(2,1,DONE), cell(2,2,HOT)),
]}

# ---------- DP ----------
ANIMS[198] = {'kind':'array','data':[1,2,3,1],'frames':[
  fr('打家劫舍：dp[i] = max(偷第 i 家 + dp[i-2]，不偷 dp[i-1])。', box(0,HL), pt('i',0,color=P)),
  fr('dp[0]=1。', box(0,DONE), pt('i',0,color=P)),
  fr('dp[1]=max(2+0, dp0=1)=2。', box(0,DONE), box(1,HL), pt('i',1,color=P)),
  fr('dp[2]=max(3+dp0=1, dp1=2)=4。', box(0,DONE), box(1,DONE), box(2,HL), pt('i',2,color=P)),
  fr('dp[3]=max(1+dp1=2, dp2=4)=4。答案 4（偷 1(下标0)+3(下标2)）。', box(0,DONE), box(1,DONE), box(2,HL), box(3,HL), pt('i',3,color=P)),
]}
ANIMS[300] = {'kind':'array','data':[10,9,2,5,3,7,101,18],'frames':[
  fr('最长递增子序列：dp[i] = 以 nums[i] 结尾的最长长度。', box(0,HL), pt('i',0,color=P)),
  fr('dp[0]=1；dp[1]=1（9 不大于 10）。', box(0,DONE), box(1,DONE), pt('i',1,color=P)),
  fr('dp[2]=1（2 最小）；dp[3]=dp[2]+1=2（5>2）。', box(0,DONE), box(1,DONE), box(2,DONE), box(3,HL), pt('i',3,color=P)),
  fr('…dp[7]=4：18 → 2,5,7,18（长度 4）。', box(0,DONE), box(1,DONE), box(2,DONE), box(3,DONE), box(4,DONE), box(5,DONE), box(6,DONE), box(7,HL), pt('i',7,color=P)),
]}

# ---------- 栈 / 括号 ----------
ANIMS[20] = {'kind':'array','data':list('()[]{}'),'frames':[
  fr('遇到左括号就入栈。i=0 是 ( → 栈 [(]。', box(0,HL), pt('i',0,color=P)),
  fr('i=1 是 )，与栈顶匹配 → 出栈。', box(1,HL), pt('i',1,color=P)),
  fr('i=2 是 [ → 入栈；i=3 是 ] 匹配出栈。', box(2,HL), box(3,HL), pt('i',3,color=P)),
  fr('i=4 是 { → 入栈；i=5 是 } 匹配出栈。栈空 → 全部匹配。', box(4,HL), box(5,HL), pt('i',5,color=P)),
]}

# ---------------- 新增批次：能动的题尽量配齐 --------------
# 双指针
ANIMS[15] = A([-4,-1,-1,0,1,2], [
  fr('排序后 [-4,-1,-1,0,1,2]。固定 i，用 left/right 双指针在剩余区间找两数。', pt('i',0,color=P), pt('L',1,color=B), pt('R',5,color=R)),
  fr('i=-4：L=-1、R=2，和=-4-1+2=-3<0 → 太小，L 右移。', box(0,HL),box(1,HL),box(5,HL), pt('i',0,color=P),pt('L',2,color=B),pt('R',5,color=R)),
  fr('i=-4：L=-1(第2个)、R=2，和仍<0 → L 继续右移。', box(0,HL),box(2,HL),box(5,HL), pt('i',0,color=P),pt('L',3,color=B),pt('R',5,color=R)),
  fr('i=-4：L=0、R=2，和=2>0 → R 左移；三个数不能重复，i 前进时跳过重复值。', box(0,HL),box(3,HL),box(5,HL), pt('i',0,color=P),pt('L',3,color=B),pt('R',4,color=R)),
])
ANIMS[42] = A([0,1,0,2,1,0,1,3,2,1,2,1], [
  fr('接雨水：双指针 L、R 从两端，谁矮动谁，维护左右最高柱。', pt('L',0,color=B), pt('R',11,color=R)),
  fr('L=0 高0，R=11 高1，R 侧更矮 → 先动 R。每根柱子水 = min(左max,右max)-height[i]。', box(11,HL), pt('L',0,color=B), pt('R',10,color=R)),
  fr('逐步向中间收，遇到更矮的柱子就累加它上面能接的水。', box(0,HL),box(1,HOT),box(2,HL),box(3,HL), pt('L',1,color=B), pt('R',9,color=R)),
  fr('接的水总量 = 6。', box(0,HL),box(1,HOT),box(2,HL),box(3,HL),box(4,HOT),box(5,HL),box(6,HOT),box(7,HL),box(8,HL),box(9,HOT),box(10,HL),box(11,HL)),
])
# 滑动窗口
ANIMS[76] = A(list('ADOBECODEBANC'), [
  fr('最小覆盖子串：目标 t="ABC"。用窗口 [L,R) 扩到包含所有必需字母。', pt('L',0,color=B), pt('R',0,color=R)),
  fr('R 右移，统计窗口里各字符够不够；不够就继续扩。', box(0,HL),pt('L',0,color=B),pt('R',3,color=R)),
  fr('窗口已包含 ABC（下标0..5 “ADOBEC”），L 尝试右移收缩找更短。', box(1,HL),box(2,HL),box(3,HL),box(4,HL),box(5,HL), pt('L',1,color=B),pt('R',5,color=R)),
  fr('最短覆盖子串 = "BANC"（下标10..13）。', box(10,HL),box(11,HL),box(12,HL),box(13,HL), pt('L',10,color=B),pt('R',13,color=R)),
])
ANIMS[239] = A([1,3,-1,-3,5,3,6,7], [
  fr('滑动窗口最大值：k=3。用单调递减队列，队头是当前最大。', box(0,HL),box(1,HL),box(2,HL), pt('L',0,color=B), pt('R',2,color=R)),
  fr('窗口 [1,3,-1]，最大 3。队列里存下标，大数替掉前面的小数。', box(0,HL),box(1,DONE),box(2,HL), pt('L',0,color=B), pt('R',2,color=R)),
  fr('窗口右移 → [3,-1,-3]，最大 3；过期的小标被弹出。', box(1,HL),box(2,HL),box(3,HL), pt('L',1,color=B), pt('R',3,color=R)),
  fr('每个窗口最大依次：3,3,5,5,6,7。', box(0,HL),box(1,HL),box(2,HL),box(3,HL),box(4,HL),box(5,HL),box(6,HL),box(7,HL)),
])
ANIMS[438] = A(list('cbaebabacd'), [
  fr('找字母异位词：p="abc"，长度 3。固定长度窗口，统计字符频次对比。', box(0,HL),box(1,HL),box(2,HL), pt('L',0,color=B), pt('R',2,color=R)),
  fr('窗口 [c,b,a] 与 p 频次相同 → 命中一个起始下标 0。', box(0,DONE),box(1,DONE),box(2,DONE), pt('L',0,color=B), pt('R',2,color=R)),
  fr('窗口右移 [b,a,e] 不匹配；继续滑。', box(1,HL),box(2,HL),box(3,HL), pt('L',1,color=B), pt('R',3,color=R)),
  fr('命中下标 6（[b,a,c]，即 s[6..8]）。', box(6,DONE),box(7,DONE),box(8,DONE), pt('L',6,color=B), pt('R',8,color=R)),
])
# 数组扫描
ANIMS[53] = A([-2,1,-3,4,-1,2,1,-5,4], [
  fr('最大子数组和：cur 保存“以当前结尾”的和，best 保存最大。', pt('i',0,color=P)),
  fr('i=0：cur=-2，best=-2。', box(0,HL), pt('i',0,color=P)),
  fr('i=1：cur=max(1, -2+1)=1（干脆从这重开），best=1。', box(1,HL), pt('i',1,color=P)),
  fr('i=3：cur=max(4, -2+4)=4 → 前面累加为负就丢掉。best 一路更新到 6。', box(0,HL),box(1,HL),box(2,HOT),box(3,HL),box(4,HL),box(5,HL),box(6,HL), pt('i',3,color=P)),
])
ANIMS[121] = A([7,1,5,3,6,4], [
  fr('买股票：记录到今天为止的最低价 minP，每天算 当天价-minP。', pt('i',0,color=P)),
  fr('i=0：minP=7，利润 0。', box(0,HL), pt('i',0,color=P)),
  fr('i=1：价 1 更低 → minP=1。', box(1,HL), pt('i',1,color=P)),
  fr('i=4：价 6，利润 6-1=5 最大，best 更新。', box(1,HL),box(4,HL), pt('i',4,color=P)),
])
ANIMS[136] = A([2,2,1], [
  fr('只出现一次：用异或，两个相同数异或为 0。', pt('i',0,color=P)),
  fr('x=0^2=2。', box(0,HL), pt('i',0,color=P)),
  fr('x=2^2=0（2 成对抵消）。', box(0,HL),box(1,HL), pt('i',1,color=P)),
  fr('x=0^1=1。只出现一次的是 1。', box(0,DONE),box(1,DONE),box(2,HL), pt('i',2,color=P)),
])
ANIMS[169] = A([2,2,1,1,1,2,2], [
  fr('多数元素：Boyer-Moore。cand 记录候选，cnt 计净胜。', pt('i',0,color=P)),
  fr('i=0：cand=2,cnt=1。', box(0,HL), pt('i',0,color=P)),
  fr('i=2：值 1 与 cand 不同 → cnt-1=0，换 cand。', box(2,HOT), pt('i',2,color=P)),
  fr('多数元素 2 最后 cnt 一定为正。', box(0,HL),box(1,HL),box(6,HL), pt('i',6,color=P)),
])
ANIMS[287] = A([1,3,4,2,2], [
  fr('找重复数：把值当下标来“跳”。nums[0]=1 → 下一步到下标 1。', box(0,HL), pt('i',0,color=P)),
  fr('1→下标3 → 又回到 nums[3]=2 → 下标2 → nums[2]=4 → 下标4 → nums[4]=2 …成环。', box(1,HL),box(2,HL),box(3,HL),box(4,HL), pt('i',4,color=P)),
  fr('环的入口（或相遇点）就是重复数 2。', box(4,DONE),box(2,DONE), pt('i',2,color=P)),
])
ANIMS[31] = A([1,2,3], [
  fr('下一个排列：从右往左找第一个下降点 i（nums[i] < nums[i+1]）。', box(1,HL), pt('i',1,color=P)),
  fr('i=1（nums[1]=2）。从右找第一个比 2 大的数（3），交换。', box(1,HL),box(2,HL), pt('i',1,color=P)),
  fr('交换后 [1,3,2]，再把 i 之后逆序成升序 → [1,3,2]。', box(0,DONE),box(1,DONE),box(2,DONE)),
])
ANIMS[55] = A([2,3,1,1,4], [
  fr('跳跃游戏：reach 记录当前能到的最远下标。', pt('i',0,color=P)),
  fr('i=0：可跳 2，reach=2。', box(0,HL), pt('i',0,color=P)),
  fr('i=1：可到 1+3=4，reach 更新为 4。', box(1,HL), pt('i',1,color=P)),
  fr('能覆盖到末尾 → true。（reach<0 则 false）', box(4,DONE), pt('i',2,color=P)),
])
# 矩阵
ANIMS[73] = G([[1,1,1],[1,0,1],[1,1,1]], [
  fr('矩阵置零：先记录有 0 的行 / 列，再整行整列清零。', cell(1,1,HOT)),
  fr('第 1 行第 1 列有 0 → 把整行、整列都标为 0。', cell(1,0,DONE),cell(1,1,DONE),cell(1,2,DONE),cell(0,1,DONE),cell(2,1,DONE)),
  fr('结果：仍保留 (0,0)、(0,2)、(2,0)、(2,2)。', cell(0,0,HL),cell(0,2,HL),cell(2,0,HL),cell(2,2,HL)),
])
ANIMS[54] = G([[1,2,3],[4,5,6],[7,8,9]], [
  fr('螺旋矩阵：按 上→右→下→左 一圈圈收。', cell(0,0,HL)),
  fr('先走顶行 1→2→3，再右列 6→9。', cell(0,0,DONE),cell(0,1,DONE),cell(0,2,DONE),cell(1,2,HL),cell(2,2,HL)),
  fr('再底行 8→7，再左列 4，最后收中间 5。', cell(1,0,HL),cell(2,0,HL),cell(2,1,HL),cell(1,1,HL)),
])
ANIMS[48] = G([[1,2,3],[4,5,6],[7,8,9]], [
  fr('旋转图像（顺时针90°）：先转置（把 (i,j) 与 (j,i) 交换）。', cell(1,0,HL),cell(0,1,HL)),
  fr('转置后第一行变 [1,4,7]。', cell(0,1,DONE),cell(1,0,DONE),cell(0,2,DONE),cell(2,0,DONE),cell(1,2,DONE),cell(2,1,DONE)),
  fr('再逐行左右反转 → 顺时针旋转结果。', cell(0,0,DONE),cell(0,1,DONE),cell(0,2,DONE),cell(1,0,DONE),cell(1,1,DONE),cell(1,2,DONE),cell(2,0,DONE),cell(2,1,DONE),cell(2,2,DONE)),
])
ANIMS[240] = G([[1,4,7,11,15],[2,5,8,12,19],[3,6,9,16,22],[10,13,14,17,24],[18,21,23,26,30]], [
  fr('搜索二维矩阵 II：从右上角 (0,4) 开始，比 target 大就左移，小就下移。', cell(0,4,HL)),
  fr('15 > target=5 → 左移。', cell(0,3,HL)),
  fr('11、7 都大 → 继续左移到 (0,1)=4 < 5 → 下移。', cell(0,1,HL)),
  fr('(1,1)=5 == target，找到。', cell(1,1,DONE)),
])

# ---- 链表 ----
ANIMS[19] = L([1,2,3,4,5], [
  fr('删除倒数第 n 个（n=2）：快指针先走 n 步。', npt('fast',2,color=R), npt('slow',0,color=B)),
  fr('快慢一起走，直到 fast 到末尾。slow 停在要删结点的前一个。', npt('fast',5,color=R), npt('slow',3,color=B)),
  fr('删掉 slow->next（这里是4）→ [1,2,3,5]。', npt('slow',3,color=B), node(3,HOT)),
])
ANIMS[24] = L([1,2,3,4], [
  fr('两两交换：1、2 交换，3、4 交换。', node(0,HL),node(1,HL), pt('p',0,color=P)),
  fr('交换后 2→1→4→3。', node(0,DONE),node(1,HL),node(2,HL),node(3,HL), pt('p',2,color=P)),
])
ANIMS[234] = L([1,2,2,1], [
  fr('回文链表：快慢指针找中点。', npt('slow',0,color=B), npt('fast',0,color=R)),
  fr('slow 走1步、fast 走2步 → slow 到中点的后半段。', npt('slow',2,color=B), npt('fast',3,color=R), node(0,HL),node(1,HL)),
  fr('反转后半段再与前半段逐位比较。', node(3,DONE),node(2,DONE),node(0,HL),node(1,HL)),
])
# ---- 树（指针/遍历/高亮）----
ANIMS[104] = {"kind":"tree","data":[3,9,20,None,None,15,7],
  "frames":[fr('最大深度：递归 左/右 子树取大 + 1。', tn(0,HL)),
    fr('根 3 深度 = 1 + max(左树深,右树深)。', tn(0,HL),tn(1,HL),tn(2,HL)),
    fr('最深是右子树 3→20→15/7，深度 3。', tn(0,DONE),tn(2,DONE),tn(4,DONE),tn(5,HL)),
  ]}
ANIMS[543] = {"kind":"tree","data":[1,2,3,4,5],
  "frames":[fr('直径：任意两点间最长路径。设经过某节点的左深+右深。', tn(0,HL)),
    fr('节点 2：左子树深2、右子树深0 → 经过 2 的路径=3。', tn(1,HL),tn(3,HL),tn(4,HL)),
    fr('直径 = 3（路径 3→? 最长）。', tn(3,DONE),tn(1,DONE),tn(0,DONE),tn(2,DONE)),
  ]}
ANIMS[199] = {"kind":"tree","data":[1,2,3,None,5,None,4],
  "frames":[fr('二叉树的右视图：每层取最右边那个。', tn(0,HL)),
    fr('层1 最右：3。', tn(0,DONE),tn(1,HL),tn(2,HL)),
    fr('层2 最右：4。结果 [1,3,4]。', tn(1,DONE),tn(2,DONE),tn(4,HL),tn(6,HL)),
  ]}
ANIMS[98] = {"kind":"tree","data":[2,1,3],
  "frames":[fr('验证 BST：判断是否 左<根<右 且整棵子树都满足。', tn(0,HL)),
    fr('比较：2 的左孩子 1<2，右孩子 3>2 → 当前合法。', tn(0,HL),tn(1,HL),tn(2,HL)),
    fr('整棵树合法 → true。', tn(0,DONE),tn(1,DONE),tn(2,DONE)),
  ]}
# ---- 二分 ----
ANIMS[35] = A([1,3,5,6], [
  fr('搜索插入位置 target=2：二分找第一个 ≥ target 的下标。', pt('lo',0,color=B), pt('hi',3,color=R)),
  fr('mid=1（值为3）≥2 → hi=mid-1。', box(1,HL), pt('lo',0,color=B), pt('hi',0,color=R)),
  fr('mid=0（值1）<2 → lo=1。', box(0,HL), pt('lo',1,color=B), pt('hi',0,color=R)),
  fr('lo=1 是第一个 ≥2 的位置 → 返回 1。', box(1,DONE), pt('lo',1,color=B), pt('hi',0,color=R)),
])
ANIMS[34] = A([5,7,7,8,8,10], [
  fr('排序数组中 target=8 的范围：找左界和右界。', pt('lo',0,color=B), pt('hi',5,color=R)),
  fr('二分找第一个 8 → 下标 3。', box(3,HL), pt('lo',0,color=B), pt('hi',5,color=R)),
  fr('再找最后一个 8 → 下标 4。', box(4,HL), pt('lo',0,color=B), pt('hi',5,color=R)),
  fr('结果是 [3,4]。', box(3,DONE),box(4,DONE), pt('lo',3,color=B), pt('hi',4,color=R)),
])
ANIMS[153] = A([3,4,5,1,2], [
  fr('旋转有序数组的最小值：看 mid 落在左段还是右段。', pt('lo',0,color=B), pt('hi',4,color=R)),
  fr('mid=2（值5）> hi(值2) → 最小在右半。', box(2,HL), pt('lo',3,color=B), pt('hi',4,color=R)),
  fr('mid=3（值1）< hi(值2) → 收 hi。', box(3,HL), pt('lo',3,color=B), pt('hi',3,color=R)),
  fr('nums[3]=1 是最小值。', box(3,DONE), pt('lo',3,color=B), pt('hi',3,color=R)),
])
ANIMS[74] = G([[1,3,5,7],[10,11,16,20],[23,30,34,60]], [
  fr('搜索二维矩阵（每行升序、每行行首比上一行大，可当一维二分）。', cell(1,1,HL)),
  fr('mid 定位到行/列，若 < target 往右/下找，> 往左/上。', cell(1,1,HL),cell(1,2,HL)),
  fr('找到 16 == target。', cell(1,2,DONE)),
])
# ---- 栈 ----
ANIMS[739] = A([73,74,75,71,69,72,76,73], [
  fr('每日温度：单调递减栈，遇到更高就弹并算出“几天后”。', pt('i',0,color=P)),
  fr('i=1（74>73）→ 73 答案 1 天，弹出。', box(0,DONE),box(1,HL), pt('i',1,color=P)),
  fr('i=6（76）会让前面所有更矮的温度一次“等到答案”。', box(6,HL), pt('i',6,color=P)),
])
ANIMS[155] = A([-2,0,-3], [
  fr('最小栈：再维护一个辅助栈，存“当前最小值”。', pt('i',0,color=P)),
  fr('push(-2)：主栈[-2]，min 栈[-2]。', box(0,HL), pt('i',0,color=P)),
  fr('push(0)：0 > 当前最小-2 → min 栈仍压 -2。', box(0,DONE),box(1,HL), pt('i',1,color=P)),
  fr('push(-3)：新最小 -3。getMin 返回 -3。', box(0,DONE),box(1,DONE),box(2,HL), pt('i',2,color=P)),
])
ANIMS[394] = A(list('3[a2[c]]'), [
  fr('字符串解码：遇到数字、[ 就把“待拼的前缀+数字”入栈；遇到 ] 出栈展开。', pt('i',0,color=P)),
  fr('栈：num=3, 前缀=""。', box(0,HL), pt('i',0,color=P)),
  fr('进入 a2[c]，读到 ] 展开成 cc。', box(1,HL),box(2,HL),box(4,HL), pt('i',4,color=P)),
  fr('最终 3 个 "a2[c]" → "accaccacc"。', box(0,DONE),box(6,DONE), pt('i',6,color=P)),
])
ANIMS[84] = A([2,1,5,6,2,3], [
  fr('柱状图最大矩形：单调栈，遇到更矮的柱子就结算“以这根为高”的矩形。', pt('i',0,color=P)),
  fr('高度 5、6 连续增高，先入栈。', box(2,HL),box(3,HL), pt('i',3,color=P)),
  fr('遇到高度 2 < 6 → 弹出6，宽度算到能延伸的最左 → 面积6*1=6。', box(3,DONE), pt('i',4,color=P)),
  fr('最大面积 = 10（高度2 × 宽5）。', box(1,DONE),box(2,DONE),box(3,DONE),box(4,DONE),box(5,DONE), pt('i',5,color=P)),
])
# ---- 堆 ----
ANIMS[215] = A([3,2,1,5,6,4], [
  fr('数组第 k 大（k=2）：可以堆化大顶堆，或快速选择。', box(0,HL), pt('i',0,color=P)),
  fr('用小顶堆维护最大的 k 个：比堆顶大就替换堆顶。', box(0,HL),box(1,HL),box(2,HL), pt('i',2,color=P)),
  fr('扫完，堆里是最大的 2 个 → 堆顶 = 第 2 大 = 5。', box(4,DONE),box(5,DONE),box(0,HL),box(1,HL),box(2,HL),box(3,HL), pt('i',5,color=P)),
])
ANIMS[347] = A([1,1,1,2,2,3], [
  fr('前 k 个高频元素：先统计频次，再桶/堆取频次最高的 k 个。', box(0,HL), pt('i',0,color=P)),
  fr('频次：1→3次，2→2次，3→1次。', box(2,DONE),box(4,DONE),box(5,HL), pt('i',5,color=P)),
  fr('k=2 → [1,2]。', box(0,DONE),box(3,DONE),box(1,HL),box(4,HL)),
])
ANIMS[295] = A([1,2,3], [
  fr('数据流中位数：左=大顶堆（小的一半），右=小顶堆（大的一半）。', box(0,HL), pt('i',0,color=P)),
  fr('addNum(1)：左 [1]，右 []。', box(0,HL), pt('i',0,color=P)),
  fr('addNum(2)：平衡后左[1]右[2]，中位(1+2)/2。', box(0,DONE),box(1,HL), pt('i',1,color=P)),
  fr('addNum(3)：左[2,1]右[3]，中位数 = 左堆顶 = 2。', box(0,DONE),box(1,DONE),box(2,HL), pt('i',2,color=P)),
])
# ---- 贪心 ----
ANIMS[763] = A(list('ababcbacadefegdehijhklij'), [
  fr('划分字母区间：先记每个字母最后出现位置，再扫一遍扩右边界。', pt('L',0,color=B), pt('R',0,color=R)),
  fr('遇到字母就把它最后位置并入当前段右边界。', box(0,HL),box(1,HL),box(2,HL), pt('L',0,color=B), pt('R',2,color=R)),
  fr('当前段到 R 闭合 → 记录一段；各段 [ ababcbaca | defegde | hijhklij ]。', box(0,DONE),box(1,DONE),box(2,DONE),box(3,DONE),box(4,DONE),box(5,DONE),box(6,DONE),box(7,DONE),box(8,DONE), pt('L',9,color=B), pt('R',14,color=R)),
])

# ---- 一维 DP ----
ANIMS[70] = A([1,2,3,5,8], [
  fr('爬楼梯：到第 n 阶只能从 n-1 或 n-2 来。', box(0,HL), pt('i',0,color=P)),
  fr('dp[1]=1，dp[2]=2。', box(0,DONE),box(1,DONE), pt('i',1,color=P)),
  fr('dp[n]=dp[n-1]+dp[n-2]：1,2,3,5,8…（斐波那契）。', box(0,DONE),box(1,DONE),box(2,HL),box(3,HL),box(4,HL), pt('i',4,color=P)),
])
ANIMS[118] = G([[1],[1,1],[1,2,1],[1,3,3,1],[1,4,6,4,1]], [
  fr('杨辉三角：每行首尾为 1，中间 = 上一行相邻两个之和。', cell(0,0,HL)),
  fr('第 3 行：1,2,1；第 5 行：1,4,6,4,1。', cell(4,2,HL)),
  fr('第 k 行第 j 个 = C(k,j)。', cell(4,2,DONE),cell(3,1,DONE),cell(2,0,DONE)),
])
ANIMS[322] = A([0,1,2,3,4,5,6,7,8,9,10,11], [
  fr('零钱兑换：coins=[1,2,5]，amount=11。dp[i]=凑成 i 所需最少硬币数。', box(0,HL), pt('i',0,color=P)),
  fr('dp[i] = min(dp[i-1],dp[i-2],dp[i-5]) + 1。', box(0,HL),box(1,HL),box(2,HL),box(3,HL),box(4,HL),box(5,HL),box(6,HL),box(7,HL),box(8,HL),box(9,HL),box(10,HL),box(11,HL), pt('i',11,color=P)),
  fr('dp[11] = min(10,9,6)+1 = 3（如 5+5+1）。', box(11,DONE), pt('i',11,color=P)),
])
ANIMS[139] = A(list('leetcode'), [
  fr('单词拆分：wordDict=["leet","code"]。dp[i]=前 i 字符能否被拆成词。', pt('i',0,color=P)),
  fr('dp[4]："leet" 在字典 → true。', box(0,HL),box(1,HL),box(2,HL),box(3,HL), pt('i',4,color=P)),
  fr('dp[8]："code" 也在 → true，整个可拆。', box(0,DONE),box(1,DONE),box(2,DONE),box(3,DONE),box(4,DONE),box(5,DONE),box(6,DONE),box(7,DONE), pt('i',8,color=P)),
])
ANIMS[152] = A([2,3,-2,4], [
  fr('最大乘积子数组：有负数，要同时记录 curMax 和 curMin。', box(0,HL), pt('i',0,color=P)),
  fr('i=0：curMax=2。', box(0,DONE), pt('i',0,color=P)),
  fr('负数让 curMax/curMin 交换：2*3=6 最大。', box(0,DONE),box(1,HL), pt('i',1,color=P)),
  fr('乘到 -2 后 curMax=-12 变负，但后续可能再翻正；答案 6（子序列 [2,3]）。', box(0,DONE),box(1,DONE),box(2,HL),box(3,HL), pt('i',3,color=P)),
])
ANIMS[416] = A([1,5,11,5], [
  fr('分割等和子集：判断能否凑到 sum/2=11。dp[j]=能否凑成 j。', box(0,HL), pt('i',0,color=P)),
  fr('0/1 背包：每数只用一次，从后往前更新 dp。', box(0,HL),box(1,HL),box(2,HL),box(3,HL), pt('i',3,color=P)),
  fr('dp[11] 可达 → 可以分割成 [1,5,5] 和 [11]。', box(3,DONE), pt('i',3,color=P)),
])
ANIMS[32] = A(list(')()())'), [
  fr('最长有效括号：dp[i]=以 i 结尾的最长有效长度。', box(0,HOT), pt('i',0,color=P)),
  fr('i=3（下标3的)）与前面的(匹配：dp[3]=2。', box(1,HL),box(2,HL),box(3,HL), pt('i',3,color=P)),
  fr('答案 = 4（子串 "()()"）。', box(1,DONE),box(2,DONE),box(3,DONE),box(4,DONE), pt('i',4,color=P)),
])
# ---- 二维 DP ----
ANIMS[64] = G([[1,3,1],[1,5,1],[4,2,1]], [
  fr('最小路径和：只能向右/向下。dp[i][j]=左边/上边较小+当前。', cell(0,0,HL)),
  fr('dp[1][1] = min(1,1)+5 = 6。', cell(1,1,HL),cell(0,1,DONE),cell(1,0,DONE)),
  fr('到达右下角 dp[2][2] = 7。', cell(2,2,DONE)),
])
ANIMS[5] = A(list('babad'), [
  fr('最长回文子串：从中心向两边扩展。', box(2,HL), pt('i',2,color=P)),
  fr('以 b 为中心 或 以两个字符为中心扩展。', box(1,HL),box(3,HL), pt('i',2,color=P)),
  fr('最长的回文是 "bab" 或 "aba"（长度 3）。', box(0,DONE),box(1,DONE),box(2,DONE), pt('i',1,color=P)),
])
ANIMS[1143] = G([[0,0,0,0],[0,1,1,1],[0,1,1,1],[0,1,2,2]], [
  fr('最长公共子序列：text1="abc",text2="ace"。字符相等取对角+1，否则取较大。', cell(1,1,HL)),
  fr('dp[3][3] = 2（"ac"）。', cell(3,3,DONE)),
])
ANIMS[72] = G([[0,1,2,3],[1,1,2,3],[2,2,1,2],[3,2,2,2],[4,3,3,2],[5,4,4,3]], [
  fr('编辑距离：word1="horse",word2="ros"。等则取左上，否则 1+min(删/增/替)。', cell(0,0,HL)),
  fr('dp[i][j] = 不同字符时的最小操作数。', cell(2,2,HL)),
  fr('右下角 = 3（horse→ros）。', cell(5,3,DONE)),
])
# ---- 回溯 ----
ANIMS[46] = A([1,2,3], [
  fr('全排列：回溯，逐位填入，用过的标掉。', box(0,HL), pt('i',0,color=P)),
  fr('填第1位：从 [1,2,3] 选一个，递归填后两位。', box(0,HL),box(1,HL),box(2,HL), pt('i',0,color=P)),
  fr('回溯换下一个候选。最终 6 种排列。', box(0,DONE),box(1,DONE),box(2,DONE), pt('i',2,color=P)),
])
ANIMS[78] = A([1,2,3], [
  fr('子集：每个数“选/不选”两种分支，叶子就是这个子集。', box(0,HL), pt('i',0,color=P)),
  fr('递归到末尾收集一个子集；回溯再试“不选”。', box(0,DONE),box(1,DONE),box(2,DONE), pt('i',2,color=P)),
  fr('共 2^3=8 个子集。', box(0,DONE),box(1,DONE),box(2,DONE), pt('i',2,color=P)),
])
ANIMS[39] = A([2,3,6,7], [
  fr('组合总和：target=7，数字可重复用。回溯“选当前数或换下一个”。', box(0,HL), pt('i',0,color=P)),
  fr('选 2、2、3 → 7，收集。', box(0,HL),box(1,HL), pt('i',1,color=P)),
  fr('选 7 → 7。组合 [2,2,3]、[7]。', box(3,DONE), pt('i',0,color=P)),
])
ANIMS[22] = A(list('((()))'), [
  fr('括号生成：能放左就放左；右括号仅在“已放右<已放左”时放。', box(0,HL), pt('i',0,color=P)),
  fr('先放完左括号再放右，保证前缀 左≥右。', box(0,DONE),box(1,DONE),box(2,DONE), pt('i',2,color=P)),
  fr('生成所有合法括号串（卡特兰数）。', box(0,DONE),box(1,DONE),box(2,DONE),box(3,DONE),box(4,DONE),box(5,DONE), pt('i',5,color=P)),
])
ANIMS[51] = G([['.','Q','.','.'],['.','.','.','Q'],['Q','.','.','.'],['.','.','Q','.']], [
  fr('N 皇后：逐行放皇后，用列/两条对角线判冲突。', cell(0,1,HL)),
  fr('放不下去就回溯换一列。', cell(1,3,HL),cell(2,0,HL),cell(3,2,HL)),
  fr('一种合法解。', cell(0,1,DONE),cell(1,3,DONE),cell(2,0,DONE),cell(3,2,DONE)),
])

# ---- 数组/技巧补充 ----
ANIMS[238] = A([1,2,3,4], [
  fr('除自身以外数组的乘积：先算每个位置左边乘积，再乘右边乘积。', box(0,HL), pt('L',0,color=B)),
  fr('left：前缀积 [1,1,2,6]。', box(0,HL),box(1,HL),box(2,HL),box(3,HL), pt('L',3,color=B)),
  fr('再从右往左乘后缀积 → [24,12,8,6]。', box(0,HL),box(1,HL),box(2,HL),box(3,HL), pt('R',0,color=R)),
])
ANIMS[45] = A([2,3,1,1,4], [
  fr('跳跃游戏 II：记录当前步能到的最远（end），到边界就加一步。', box(0,HL), pt('i',0,color=P)),
  fr('i=0：最远到 2，已是本步边界，steps=1。', box(0,HL),box(1,HL),box(2,HL), pt('i',1,color=P)),
  fr('i=1：最远到 4（1+3）。到 i=2 边界 → steps=2，到终点。', box(3,HL),box(4,HL), pt('i',4,color=P)),
])
ANIMS[128] = A([100,4,200,1,3,2], [
  fr('最长连续序列：先放入 set 去重。只从“没有前驱”的数开始向后数。', box(0,HL), pt('i',0,color=P)),
  fr('100 前面没有 99 → 向后数 [100] 长度1。', box(0,HL), pt('i',1,color=P)),
  fr('4 有前驱 3，跳过；从 1 数 1,2,3,4 → 长度4。', box(3,DONE),box(5,DONE),box(4,DONE),box(2,DONE),box(1,HL), pt('i',1,color=P)),
])
ANIMS[41] = A([3,4,-1,1], [
  fr('缺失的第一个正数：把每个正数放到「值-1」的下标处。', box(0,HL), pt('i',0,color=P)),
  fr('3 放到下标2，1 放到下标0…尽量就位。', box(0,DONE),box(2,DONE),box(1,HL), pt('i',1,color=P)),
  fr('扫一遍：下标0=1,1=1?…第一个不对的下标就是缺的数。缺 2。', box(0,HL),box(2,HL), pt('i',1,color=P)),
])
ANIMS[189] = A([1,2,3,4,5,6,7], [
  fr('轮转数组 k=3：先整体反转，再前 k 反转、后段反转。', box(0,HL),box(6,HL), pt('L',0,color=B),pt('R',6,color=R)),
  fr('整体反转 → [7,6,5,4,3,2,1]。', box(0,HL),box(6,HL), pt('L',0,color=B),pt('R',6,color=R)),
  fr('前 3 个、后 4 个分别反转 → [5,6,7,1,2,3,4]。', box(0,DONE),box(1,DONE),box(2,DONE),box(3,DONE),box(4,DONE),box(5,DONE),box(6,DONE), pt('L',0,color=B),pt('R',6,color=R)),
])
ANIMS[101] = {"kind":"tree","data":[1,2,2,3,4,4,3],
  "frames":[fr('对称二叉树：左子树与右子树镜像比较。', tn(0,HL),tn(1,HL),tn(2,HL)),
    fr('比较 左孩子的左 vs 右孩子的右，左孩子的右 vs 右孩子的左。', tn(1,DONE),tn(2,DONE),tn(3,HL),tn(6,HL)),
    fr('都相等 → 对称，返回 true。', tn(0,DONE),tn(1,DONE),tn(2,DONE),tn(3,DONE),tn(4,DONE),tn(5,DONE),tn(6,DONE)),
  ]}
ANIMS[230] = {"kind":"tree","data":[3,1,4,None,2],
  "frames":[fr('BST 中第 k 小（k=3）：中序遍历是递增的。', tn(0,HL)),
    fr('中序：1→2→3→4。', tn(3,HL),tn(1,HL),tn(4,HL),tn(2,HL)),
    fr('第 3 个 = 3。', tn(1,DONE),tn(3,HL),tn(0,HL)),
  ]}
ANIMS[108] = A([-10,-3,0,5,9], [
  fr('有序数组转平衡 BST：取中点为根，左右半递归。', box(2,HL), pt('mid',2,color=P)),
  fr('中点 0 为根；左半 [-10,-3] 建左子树，右半 [5,9] 建右子树。', box(0,HL),box(1,HL),box(2,DONE),box(3,HL),box(4,HL), pt('mid',2,color=P)),
  fr('得到平衡二叉搜索树。', box(0,DONE),box(1,DONE),box(2,DONE),box(3,DONE),box(4,DONE), pt('mid',2,color=P)),
])
ANIMS[17] = A(list('23'), [
  fr('电话号码字母组合：digit "23"。回溯逐位选一个字母。', box(0,HL),box(1,HL), pt('i',0,color=P)),
  fr('第1位从 {a,b,c} 选，第2位从 {d,e,f} 选。', box(0,DONE),box(1,HL), pt('i',1,color=P)),
  fr('组合：ad,ae,af,bd… 共 9 种。', box(0,DONE),box(1,DONE), pt('i',1,color=P)),
])
ANIMS[79] = G([['A','B','C','E'],['S','F','C','S'],['A','D','E','E']], [
  fr('单词搜索：从每个格子出发 DFS 匹配 word，走过的格子临时标记。', cell(0,0,HL)),
  fr('沿 A→B→C→E 走；走不通就回溯换方向。', cell(0,0,DONE),cell(0,1,DONE),cell(0,2,DONE),cell(0,3,DONE),cell(1,2,HL)),
  fr('找到 "ABCCED" → true。', cell(0,0,DONE),cell(0,1,DONE),cell(0,2,DONE),cell(0,3,DONE),cell(1,2,DONE),cell(2,2,DONE),cell(2,3,DONE)),
])

# ---------- 真实数据状态补丁 ----------
# 这些题包含交换/反转/扩散等“数据本身会变”的操作；旧版只高亮，
# 这里给每帧补上可视数据，让播放器真正展示变化过程。
def enrich_motion():
    def values(q, frame, data):
        ANIMS[q]['frames'][frame]['values'] = data
    def state(q, frame, *pairs):
        ANIMS[q]['frames'][frame]['state'] = stat(*pairs)

    # 原地数组变换
    for i, data in enumerate([[1,2,3], [1,2,3], [1,3,2]]): values(31, i, data)
    for i, data in enumerate([[3,4,-1,1], [1,-1,3,4], [1,-1,3,4]]): values(41, i, data)
    for i, data in enumerate([[2,0,2,1,1,0], [0,0,2,1,1,2], [0,0,2,1,1,2], [0,0,1,1,2,2]]): values(75, i, data)
    for i, data in enumerate([[0,1,0,3,12], [0,1,0,3,12], [1,0,0,3,12], [1,0,0,3,12], [1,3,0,0,12], [1,3,12,0,0]]): values(283, i, data)
    for i, data in enumerate([[1,2,3,4,5,6,7], [7,6,5,4,3,2,1], [5,6,7,1,2,3,4]]): values(189, i, data)

    # 链表的连接顺序/长度变化
    values(19, 0, [1,2,3,4,5]); values(19, 1, [1,2,3,4,5]); values(19, 2, [1,2,3,5])
    values(24, 0, [1,2,3,4]); values(24, 1, [2,1,4,3])
    values(234, 2, [1,2,1,2])

    # 二维原地变换与 BFS 扩散
    values(48, 0, [[1,2,3],[4,5,6],[7,8,9]])
    values(48, 1, [[1,4,7],[2,5,8],[3,6,9]])
    values(48, 2, [[7,4,1],[8,5,2],[9,6,3]])
    values(73, 0, [[1,1,1],[1,0,1],[1,1,1]])
    values(73, 1, [[1,1,1],[1,0,1],[1,1,1]])
    values(73, 2, [[1,0,1],[0,0,0],[1,0,1]])
    cleared = [['0','0','0','0','0'],['0','0','0','0','0'],['0','0','0','0','0'],['0','0','0','0','0']]
    values(200, 0, [['1','1','1','1','0'],['1','1','0','1','0'],['1','1','0','0','0'],['0','0','0','0','0']])
    values(200, 1, cleared); values(200, 2, cleared)
    state(200, 0, ('已发现岛屿', 1), ('已访问格子', 1))
    state(200, 1, ('已发现岛屿', 1), ('已访问格子', 9))
    state(200, 2, ('最终岛屿数', 1))
    values(994, 0, [[2,1,1],[1,1,0],[0,1,1]])
    values(994, 1, [[2,2,1],[2,1,0],[0,1,1]])
    values(994, 2, [[2,2,1],[2,2,0],[0,2,1]])
    values(994, 3, [[2,2,1],[2,2,0],[0,2,2]])
    for i, minute in enumerate(range(4)): state(994, i, ('经过分钟', minute))

    # 以前只有文字变化的状态型动画，补出可观察的状态面板。
    for i, pairs in enumerate([
        (('pre',0), ('ans',0), ('mp','{0:1}')),
        (('pre',1), ('ans',0), ('mp','{0:1, 1:1}')),
        (('pre',2), ('ans',1), ('命中 pre-k',0)),
        (('pre',3), ('ans',2), ('命中 pre-k',1)),
        (('最终答案',2),),
    ]): state(560, i, *pairs)
    for i, data in enumerate([['左堆','右堆','中位数'], ['1','—','1'], ['1','2','1.5'], ['2,1','3','2']]): values(295, i, data)
    for i, pairs in enumerate([(('左堆', '[]'), ('右堆', '[]')), (('左堆','[1]'), ('右堆','[]')), (('左堆','[1]'), ('右堆','[2]')), (('左堆','[2,1]'), ('右堆','[3]'))]): state(295, i, *pairs)
    state(78, 2, ('子集总数', '2³ = 8'))

enrich_motion()

def main():
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    json.dump(ANIMS, open(OUT, 'w', encoding='utf-8'), ensure_ascii=False, indent=1)
    print(f"[anim] 共 {len(ANIMS)} 个算法动图 -> {OUT}")

if __name__ == '__main__':
    main()
