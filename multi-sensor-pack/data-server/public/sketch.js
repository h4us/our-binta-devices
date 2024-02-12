let w = window.innerWidth;
let wh = window.innerWidth / 2;
let wq = window.innerWidth / 3;
let h = window.innerHeight;
let hh = window.innerHeight / 2;
let hq = window.innerHeight / 3;

let plotData = [];

class Graph {
  constructor(x, y, w, h, k) {
    this.xPos = x;
    this.yPos = y;
    this.Width = w;
    this.Height = h;
    this.GraphColor = k;
  }
}

function setup() {
  canvas = createCanvas(w, h);

  // TODO;
  const socket = new WebSocket(`ws://${window.HOST_IP_ADDR}:3000/data`);
  socket.addEventListener("message", (event) => {
    try {
      plotData = JSON.parse(event.data);
      // console.log('Message from server', JSON.parse(event.data));
    } catch (err) {
      console.error('Invalid data format');
    }
  });
}

function draw() {
  background(200, 200, 200);
  translate(20, 20);

  strokeWeight(4);
  stroke(230, 40, 0);

  const gw = w - 40;
  const gh = h - 40;

  for (let i = 0; i < plotData.length; i ++) {
    const _x = gw * (i / plotData.length);
    const _y = gh - gh * (plotData[i] / 1024);
    line(_x, gh, _x, _y);
  }
}

function mousePressed() {
}

function mouseDragged() {
}

function mouseReleased() {
}

function keyReleased(e) {
}

window.onresize = function () {
  w = window.innerWidth;
  wh = window.innerWidth / 2;
  wq = window.innerWidth / 3;
  h = window.innerHeight;
  hh = window.innerHeight / 2;
  hq = window.innerHeight / 3;

  canvas.resize(w, h);
};
