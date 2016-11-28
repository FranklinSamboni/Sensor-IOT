var serialport = require('serialport');
var nmea = require('nmea');

var Serialport = serialport.Serialport; 
var port = new SerialPort('/dev/tty02', {
                baudrate: 9600,
                parser: serialport.parsers.readline('\r\n')});

port.on('data', function(line) {
    console.log(nmea.parse(line));
});
