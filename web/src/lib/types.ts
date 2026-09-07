export type Difficulty = 'Easy' | 'Medium' | 'Hard';

export type Paint = {
  f: 'box' | 'cell' | 'node' | 'tn' | 'pt' | 'npt';
  i?: number;
  id?: number;
  r?: number;
  col?: number;
  c?: string;
  color?: string;
  k?: string;
  label?: string;
};

export type AnimationData = Array<string | number | null> | Array<Array<string | number | null>>;
export type AnimationFrame = {
  note: string;
  paint: Paint[];
  values?: AnimationData;
  state?: Array<{ label: string; value: string }>;
};
export type AnimationScript = {
  kind: 'array' | 'grid' | 'list' | 'tree';
  data: AnimationData;
  cycle?: number;
  frames: AnimationFrame[];
};

export type Problem = {
  num: number;
  title: string;
  diff: Difficulty;
  url: string;
  slug: string;
  tag: string;
  category: string;
  body: string[];
  code: string;
  tests: [string, string, string][];
  desc: { zh?: string };
  anim?: AnimationScript;
  diagramHtml?: string;
};

export type ProblemSummary = Pick<Problem, 'num' | 'title' | 'diff' | 'tag' | 'category' | 'anim'>;
