/**  @type {import('fastify').FastifyInstance} */

import * as path from 'node:path';

import 'dotenv/config';
const {
  VERSION = '1.0.0',
  OSC_HOST_PORT = 12000,
  OSC_REMOTE_IP = '0.0.0.0',
  OSC_REMOTE_PORT = 12001,
  OSC_DEST_IP = '0.0.0.0',
  OSC_DEST_PORT = 12000,
  DEFAULT_SERIALPORT_PATH = '/dev/ttyS0',
} = process.env;

import Fastify from 'fastify';
import * as FastifyView from '@fastify/view';
import * as FastifyStatic from '@fastify/static';
import * as FastifyWebsocket from '@fastify/websocket';
import cors from '@fastify/cors';

const fastify = Fastify({
  logger: { level: 'error' },
  pluginTimeout: 0,
});

import { Liquid } from 'liquidjs';
const engine = new Liquid({
  root: path.resolve('templates'),
  extname: '.liquid.html',
});

import { SerialPort } from 'serialport';
import { ByteLengthParser } from '@serialport/parser-byte-length';
import { ReadlineParser } from '@serialport/parser-readline';

import { Server, Client } from 'node-osc';
const oscs = new Server(OSC_HOST_PORT, '0.0.0.0');
const oscr = new Client(OSC_REMOTE_IP, OSC_REMOTE_PORT);

const port = parseInt(process.env.PORT, 10) || 3000;

import ip from 'ip';

import { sortBy } from 'lodash-es';

const start = Date.now();

/*
 * --
 */
let serialCount = 0;
let NumOfSerialBytes = 8;
let serialInArray = new Array(NumOfSerialBytes);

let DynamicArrayTime1 = [];
let DynamicArrayTime2 = [];
let DynamicArrayTime3 = [];
let Time1 = [];
let Time2 = [];
let Time3 = [];
let Voltage1 = [];
let Voltage2 = [];
let Voltage3 = [];
let current = [];
let DynamicArray1 = [];
let DynamicArray2 = [];
let DynamicArray3 = [];
let PowerArray = [];

let TotalRecieved = 0;
let ErrorCounter = 0;
let DataRecieved = false;
let DataRecieved2 = false;
let DataRecieved3 = false;
let xMSB, xLSB, yMSB, yLSB, xValue, yValue, Command;


const findPeaks = (n = 3) => {
  // - TODO:
  let r_peaks = [];

  if (Time3.length == Voltage3.length && Time3.length > 0) {
    let peaks = [];

    for (let i = 0; i < Time3.length; i++) {
      peaks.push({ index: i, value: Voltage3[i] });
    }

    r_peaks =  sortBy(peaks, ['value']);
    r_peaks = r_peaks.reverse();
  }

  return r_peaks.slice(0, n);
};


const runApp = async () => {
  /*
   * Set timezone
   */
  process.env.TZ = 'Asia/Tokyo';

  /*
   * Serialport setup
   */
  const sp = await SerialPort.list();
  console.info('Available serial ports, ', sp);

  const rp2040 = sp.filter((el) => (
    (/^Raspberry.*/.test(el.manufacturer) || el.vendorId == '2e8a')
    || (/^Microsoft.*/.test(el.manufacturer) || el.vendorId == '2e8a')
  ));

  const default_sp = sp.filter((el) => el.path == DEFAULT_SERIALPORT_PATH);

  if (rp2040 && rp2040.length > 0) {
    const { path, baudRate = 115200 } = rp2040[0];
    const serialport = new SerialPort({ path, baudRate });
    fastify.decorate('serialport', serialport);
    console.info('..., Open port (rp2040)', rp2040[0]);

    const parser = serialport.pipe(new ReadlineParser({ delimiter: '\n' }));
    parser.on('data', async (data) => {
      console.log(data);
    });
  } else if (default_sp.length > 0) {
    const serialport = new SerialPort({ path: DEFAULT_SERIALPORT_PATH, baudRate: 115200 });
    fastify.decorate('serialport', serialport);

    const parser = serialport.pipe(new ByteLengthParser({ length: 1 }));

    parser.on('data', async (data) => {
      const d = data.readUInt8();
      if (d == 0) { serialCount = 0; }

      if (d > 255) { return; }

      serialInArray[serialCount] = d;
      serialCount++;

      let err = true;
      if (serialCount >= NumOfSerialBytes) {
        serialCount = 0;

        TotalRecieved++;

        let Checksum = 0;
        for (let x = 0; x < serialInArray.length - 1; x++) {
          Checksum = Checksum + serialInArray[x];
        }

        Checksum = Checksum % 255;

        if (Checksum == serialInArray[serialInArray.length - 1]) {
          err = false;
          DataRecieved = true;
        }
        else {
          err = true;
          // console.log("Error:  " + ErrorCounter + " / " + TotalRecieved + " : " + parseFloat(ErrorCounter / TotalRecieved) * 100 + "%");
          DataRecieved = false;
          ErrorCounter++;
          // console.log("Error:  " + ErrorCounter + " / " + TotalRecieved + " : " + parseFloat(ErrorCounter / TotalRecieved) * 100 + "%");
        }
      }

      if (!err) {

        const zeroByte = serialInArray[6];

        xLSB = serialInArray[3];
        if ((zeroByte & 1) == 1) xLSB = 0;
        xMSB = serialInArray[2];
        if ((zeroByte & 2) == 2) xMSB = 0;

        yLSB = serialInArray[5];
        if ((zeroByte & 4) == 4) yLSB = 0;

        yMSB = serialInArray[4];
        if ((zeroByte & 8) == 8) yMSB = 0;


        Command = serialInArray[1];

        xValue = xMSB << 8 | xLSB;
        yValue = yMSB << 8 | yLSB;


        switch (Command) {

          case 1: // Data is added to dynamic arrays
            DynamicArrayTime3.push(parseFloat(xValue));
            DynamicArray3.push(parseFloat(yValue));

            break;

          case 2: // An array of unknown size is about to be recieved, empty storage arrays
            DynamicArrayTime3 = new Array();
            DynamicArray3 = new Array();
            break;

          case 3:  // Array has finnished being recieved, update arrays being drawn
            Time3 = [...DynamicArrayTime3];
            Voltage3 = [...DynamicArray3];
            DataRecieved3 = true;
            break;

          case 4: // Data is added to dynamic arrays
            DynamicArrayTime2.push(parseFloat(xValue));
            DynamicArray2.push((yValue - 16000.0) / 32000.0 * 20.0);
            break;

          case 5: // An array of unknown size is about to be recieved, empty storage arrays
            DynamicArrayTime2 = new Array();
            DynamicArray2 = new Array();
            break;

          case 6:  // Array has finnished being recieved, update arrays being drawn
            Time2 = [...DynamicArrayTime2];
            current = [...DynamicArray2];
            DataRecieved2 = true;
            break;

          case 20:
            PowerArray.push(yValue);
            break;

          case 21:
            DynamicArrayTime.push(xValue);
            DynamicArrayPower.push(yValue);
            break;
        }

        if (DataRecieved3) {
          let p = findPeaks();
          p = p.map((el) => Object.values(el));
          p = p.flat();
          // console.log('/peaks', p); // - index1, value1, index2, value2...
          oscr.send('/peaks', p);
        }
      }

    });
  }

  /*
   * OSC Responder API, TODO
   */
  // oscs.on('message', (msg) => {
  //   const [tag, ...data] = msg;

  //   console.log('Incomming OSC messages: ', tag, data);
  // });

  /*
   * HTTP API Routes
   */
  fastify
    .register(FastifyView, {
      engine: { liquid: engine },
      root: path.resolve('templates'),
    })
    .register(FastifyStatic, {
      root: path.resolve('public'),
    })
    .register(cors, { origin: '*' })
    .register(FastifyWebsocket)
    .register(async function (fastify) {
      fastify.get('/data', { websocket: true }, (connection, req) => {
        // TODO: performance
        console.info('new request: ', req);

        setInterval(() => {
          // const ua = Uint16Array.from(Voltage3);
          connection.socket.send(JSON.stringify(Voltage3));
        }, 1000 / 20);

        connection.socket.on('message', message => {
          connection.socket.send('hi from wildcard route');
        });
      });
    });

  fastify
    .after(() => {
      // ...
      fastify
        .get('/', (req, reply) => {
          reply.view('index.liquid.html', { HOST_IP_ADDR: ip.address() });
        });
    });

  // ...
  try {
    await fastify.listen({ host: '::', port });
  } catch (err) {
    throw err;
  }

  console.info(`Started server on port ${port}`);
  console.info('OSC remote:', OSC_REMOTE_IP, OSC_REMOTE_PORT);

};

runApp();
