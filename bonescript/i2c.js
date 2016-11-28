var i2c = require('i2c');
var address = 0x68;
var length = 0x13;
var cRead = 0x00;
var timeRead = 1000;
var wire = new i2c(address, {device: '/dev/i2c-1'}); // point to your i2c address, debug provides REPL interface 

wire.on('data', function(data) {
//	console.log('on')
//	console.log(data.data)
	var buff = new Buffer(data.data.toString('hex'),'hex')
//	console.log(buff[0].toString(16))
	console.log('Fecha: ' + buff[4].toString(16) + '/' + buff[5].toString(16) + '/' + buff[6].toString(16) + ' Hora: ' + buff[2].toString(16) + ':' + buff[1].toString(16) + ':' +  buff[0].toString(16))
  // result for continuous stream contains data buffer, address, length, timestamp 
});
wire.stream(cRead, length, timeRead);

