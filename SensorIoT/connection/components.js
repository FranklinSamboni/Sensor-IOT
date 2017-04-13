let SUCCESS = 1;
let ERROR = -1;

let CORRECT_STATUS_COMPONENT = "Correcto";
let ERROR_STATUS_COMPONENT = "Error";

let URL_BASE = "https://plataformamec.com/api/";
let URL_ACCELEROMETER = URL_BASE + "accelerometer";
let URL_ADC = URL_BASE + "adc";
let URL_RTC = URL_BASE + "rtc";
let URL_WIFI = URL_BASE + "wifi";
let URL_CPU = URL_BASE + "cpu";
let URL_BATTERY = URL_BASE + "battery";
let URL_GPS = URL_BASE + "gps";

var Client = require('node-rest-client').Client;
var client = new Client();

var auth = require('./auth.js');
var exit = require('./exit.js');
let fs = require('fs');

var authToken = "";

var authNose = auth.doAuth(function(code, token){
    if(code == ERROR){
        exit.preExitFunc();
    }else{
        authToken = token;
        console.log(token);

        //var config = require('./componentsFiles/accelerometer.json');
        //console.log(config);
        acelerometerData(token);
        adcData(token);
        rtcData(token);
        cpuData(token);
        batteryData(token);
        gpsData(token);

    }

});

//exports.acelerometerData =
function acelerometerData (token) {

    console.log("acelerometerData");

    fs.readFile('componentsFiles/accelerometer.json', 'utf-8', (err, json) => {
        if(err) {
            console.log('error: ', err);
        }
        else {

            var jsonObj = JSON.parse(json);

            var args = {
                data: jsonObj,
                headers: {"Content-Type": "application/json",
                    "Authorization":token,}
            };

            console.log(args);

            client.post(URL_ACCELEROMETER, args, function (data, response) {
                console.log("acelerometerData");
                var jsonObj = data;
                console.log(jsonObj);
                if (jsonObj.code == "001") {
                    code = SUCCESS ;
                    //callback(code,token);
                }
                else {
                    code = ERROR ;
                    exit.preExitFunc();
                    //callback(code,token);
                }
            });

        }
    });
}

//exports.adcData =
function adcData (token) {
    console.log("adcData");

    fs.readFile('componentsFiles/adc.json', 'utf-8', (err, json) => {
        if(err) {
            console.log('error: ', err);
        }
        else {

            var jsonObj = JSON.parse(json);
            var args = {
                data: jsonObj,
                headers: {"Content-Type": "application/json",
                    "Authorization":token,}
            };

            console.log(args);

            client.post(URL_ADC, args, function (data, response) {
                console.log("adcData");
                var jsonObj = data;
                console.log(jsonObj);
                if (jsonObj.code == "001") {
                    code = SUCCESS ;
                    //callback(code,token);
                }
                else {
                    code = ERROR ;
                    exit.preExitFunc();
                    //callback(code,token);
                }
            });
        }
    });

}

//exports.rtcData =
function rtcData(token) {
    console.log("rtcData");

    fs.readFile('componentsFiles/rtc.json', 'utf-8', (err, json) => {
        if(err) {
            console.log('error: ', err);
        }
        else {

            var jsonObj = JSON.parse(json);

            var args = {
                data: jsonObj,
                headers: {"Content-Type": "application/json",
                    "Authorization":token,}
            };

            console.log(args);

            client.post(URL_RTC, args, function (data, response) {
                console.log("rtcData");
                var jsonObj = data;
                console.log(jsonObj);
                if (jsonObj.code == "001") {
                    code = SUCCESS ;
                    //callback(code,token);
                }
                else {
                    code = ERROR ;
                    exit.preExitFunc();
                    //callback(code,token);
                }
            });
        }
    });

}


//exports.cpuData =
function cpuData (token) {
    console.log("cpuData");

    fs.readFile('componentsFiles/cpu.json', 'utf-8', (err, json) => {
        if(err) {
            console.log('error: ', err);
        }
        else {

            var jsonObj = JSON.parse(json);

            var args = {
                data: jsonObj,
                headers: {"Content-Type": "application/json",
                    "Authorization":token,}
            };

            console.log(args);

            client.post(URL_CPU, args, function (data, response) {
                console.log("cpuData");
                var jsonObj = data;
                console.log(jsonObj);
                if (jsonObj.code == "001") {
                    code = SUCCESS ;
                    //callback(code,token);
                }
                else {
                    code = ERROR ;
                    exit.preExitFunc();
                    //callback(code,token);
                }
            });
        }
    });

}

//exports.batteryData =
function batteryData (token) {
    console.log("batteryData");

    fs.readFile('componentsFiles/battery.json', 'utf-8', (err, json) => {
        if(err) {
            console.log('error: ', err);
        }
        else {

            var jsonObj = JSON.parse(json);

            var args = {
                data: jsonObj,
                headers: {"Content-Type": "application/json",
                    "Authorization":token,}
            };

            console.log(args);

            client.post(URL_BATTERY, args, function (data, response) {
                console.log("batteryData");
                var jsonObj = data;
                console.log(jsonObj);
                if (jsonObj.code == "001") {
                    code = SUCCESS ;
                    //callback(code,token);
                }
                else {
                    code = ERROR ;
                    exit.preExitFunc();
                    //callback(code,token);
                }
            });
        }
    });

}

//exports.gpsData =
function gpsData (token) {
    console.log("gpsData");

    fs.readFile('componentsFiles/gps.json', 'utf-8', (err, json) => {
        if(err) {
            console.log('error: ', err);
        }
        else {

            var jsonObj = JSON.parse(json);

            var args = {
                data: jsonObj,
                headers: {"Content-Type": "application/json",
                    "Authorization":token,}
            };

            console.log(args);

            client.post(URL_GPS, args, function (data, response) {
                console.log("gpsData");
                var jsonObj = data;
                console.log(jsonObj);
                if (jsonObj.code == "001") {
                    code = SUCCESS ;
                    //callback(code,token);
                }
                else {
                    code = ERROR ;
                    exit.preExitFunc();
                    //callback(code,token);
                }
            });

        }
    });

}

exports.wifiData = function () {

}




