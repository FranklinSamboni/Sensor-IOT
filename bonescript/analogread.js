var b = require('bonescript');
var sleep = require('sleep');

setInterval(getDataAnalog, 100);
setInterval(getDatay, 130);

function getDatay(){
	b.analogRead('P9_40', function(datay){
		var valory = (datay.value / 3 ) * 1024 /  1.8
		console.log('y: ' + valory)
	});
}


function getDataAnalog(){
	sleep.usleep(25);
	b.analogRead('P9_37', function(data){
		var valor = (data.value / 3) * 1024 / 1.8
		console.log('x: ' + valor);
	});
	/*sleep.usleep(25);
	b.analogRead('P9_40', function(datay){
		var valory = (datay.value / 3)* 1024 / 1.8
		console.log('y: ' + valory)
	});
	sleep.usleep(25);
	b.analogRead('P9_38', function(dataz){
		var valorz = (dataz.value / 3) * 1024 / 1.8
		console.log('z: ' + valorz)
	});
	sleep.usleep(25);*/
}

