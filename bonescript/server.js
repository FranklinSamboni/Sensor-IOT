var serialport = require('serialport');
var b = require('bonescript');
var nmea = require('nmea');
var port = '/dev/ttyO2';
var options = {
    baudrate: 9600,
    parser: serialport.parsers.readline('\r\n')
};

b.serialOpen(port, options, onSerial);

function onSerial(x) {
    if (x.err) {
        console.log('***ERROR*** ' + JSON.stringify(x));
    }
    if (x.event == 'open') {
       console.log('***OPENED***');
    }
    if (x.event == 'data') {
        console.log(String(x.data));
	console.log(nmea.parse(String(x.data)));
    }
}

