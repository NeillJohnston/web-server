class Point {
  constructor(x, y) {
    this.x = x;
    this.y = y;
  }
}

class Edge {
  constructor(p, q, i, j) {
    this.d = Math.pow(p.x-q.x, 2) + Math.pow(p.y-q.y, 2);
    this.i = i;
    this.j = j;
  }
}

// Turn a displacement component and a distance into a new component
const transform = (z, d) => {
  const r = 40;
  return z / (d/r + 1);
}

// Handle mouse interaction with a given point p
const interact = p => {
  const x = m.x - p.x;
  const y = m.y - p.y;
  const d = Math.sqrt(x*x + y*y);
  return new Point(
    p.x + transform(x, d),
    p.y + transform(y, d),
  );
}

// Set up canvas, context

const canvas = document.getElementById('tree');
const c = canvas.getContext('2d');
const scale = 2;
const m = new Point(0, 0);

var bounds;
var W;
var H;

const init = () => {
  bounds = canvas.getBoundingClientRect();
  W = scale*bounds.width;
  H = scale*bounds.height;
};

init();
canvas.width = W;
canvas.height = H;

// Set up MST

const N = 200;
const origins = new Array(N).fill().map(_ => new Point(
  (W-80)*(Math.random())+40,
  (H-80)*(Math.random())+40
));

const edges = [];
origins.forEach((p, i) => {
  origins.forEach((q, j) => {
    if (i < j) {
      edges.push(new Edge(p, q, i, j));
    }
  });
});
edges.sort((a, b) => a.d - b.d);

// Union-find (Disjoint Set Union data structure), for Prim's
const dsu = new Array(N).fill().map(_ => -1);
const size = new Array(N).fill().map(_ => 1);

const find = x => {
  if (dsu[x] === -1) return x;
  dsu[x] = find(dsu[x]);
  return dsu[x];
};

const union = (x, y) => {
  [x, y] = [find(x), find(y)];
  if (size[y] < size[x]) [x, y] = [y, x];
  [size[x], size[y]] = [0, size[x] + size[y]];
  dsu[x] = y;
}

// Prim's MST algorithm
const mst = [];
for (const edge of edges) {
  if (find(edge.i) !== find(edge.j)) {
    mst.push(edge);
    union(edge.i, edge.j);
  }
  if (mst.length === N-1) break;
}

// Draw the tree interacting with the mouse
const drawTree = () => {
  c.clearRect(0, 0, W, H);

  const points = origins.map(interact);
  
  c.strokeStyle = 'white';
  c.lineWidth = 1;
  for (const edge of mst) {
    const p = points[edge.i];
    const q = points[edge.j];
    c.beginPath();
    c.moveTo(p.x, p.y);
    c.lineTo(q.x, q.y);
    c.stroke();
  }
};

window.addEventListener('mousemove', (event => {
  m.x = scale*event.clientX - bounds.left;
  m.y = scale*event.clientY - bounds.top;
  drawTree();
}));

// window.addEventListener('resize', (event => {
//   init();
// }));

drawTree();