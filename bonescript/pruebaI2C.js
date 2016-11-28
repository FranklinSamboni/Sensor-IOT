var b = require('bonescript');
var port = '/dev/i2c-1'
var address = 0x68;

//var data = [0xAA, 0x55, 0x33, 0xEE, 65, 51, 51, 53, 66, 78, 76, 84];
//var GSCALE = 2;

b.i2cOpen(port, address, {}, onI2C);
b.i2cScan(port, onScan);
//b.i2cWriteBytes(port, 0x2a, [1], onWriteBytes);
b.i2cReadBytes(port, null, 12, onReadBytes);

function onI2C(x) {
    //if(x.event == 'data') {
      //  console.log('data = ' + JSON.stringify(x.data));
    //}
    console.log('onI2C');
}

function onScan(x) {
    console.log('scan');
    //console.log('scan = ' + JSON.stringify(arguments));
}

function onReadBytes(x) {
    console.log('read');
    // console.log('readBytes = ' + JSON.stringify(arguments));
    //if(x.event == 'callback') process.exit(0);
}
