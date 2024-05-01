const { dmxnet } = require('dmxnet');
const { Server, Client } = require('node-osc');
require('dotenv').config();

const artnet_dest = (process.env.ARTNET_DEST || '127.0.0.1');
const artnet_univ = (process.env.ARTNET_UNIV || 0);
const artnet_ch_start = (process.env.ARTNET_CH_START || 71);
const artnet_ch_mode = (process.env.ARTNET_CH_MODE || 5);
const artnet_ch_size = (process.env.ARTNET_CH_SIZE || 10);

console.info('ARTNET config: ', artnet_dest, artnet_univ, artnet_ch_start, artnet_ch_mode,  artnet_ch_size);

const oscs = new Server(13000);
const dn = new dmxnet();
const dmxsend = dn.newSender({
  ip: artnet_dest,
  universe: parseInt(artnet_univ)
});


oscs.on('message', (msg) => {
  console.log(msg);
  const [path, ...rest] = msg;
  const [r = 0, g = 0, b = 0, dim = 0, flash = 0] = rest;

  if (path == '/all') {
    for (let i = artnet_ch_start; i < (artnet_ch_start + (artnet_ch_mode * artnet_ch_size)); i += artnet_ch_mode) {
      console.info(`${i}:R:${r},${i + 1}:G:${g},${i + 2}:B:${b},${i + 3}:DIMMER:${dim},${i + 4}:FLASH:${flash}`);

      dmxsend.prepChannel(i, r);
      dmxsend.prepChannel(i + 1, g);
      dmxsend.prepChannel(i + 2, b);
      dmxsend.prepChannel(i + 3, dim);
      dmxsend.prepChannel(i + 4, flash);
    }

    dmxsend.transmit();

    // const data = new Map([
    //   [70, [255, 0, 10, 125, 0]],
    //   [75, [25, 170, 160, 255, 0]],
    //   [80, [25, 200, 25, 255, 0]]
    // ]);

    // data.forEach((el, key) => {
    //   for (let i = 0; i < el.length; i++) {
    //     dmxsend.prepChannel(key + i, el[i]);
    //   }
    // });

    // dmxsend.transmit();

    // dmxsend.fillChannels(start, stop, value)
  }

  if (path == '/part') {

  }
});
