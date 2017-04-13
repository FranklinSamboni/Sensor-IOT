
let URL_AUTH = "https://plataformamec.com/api/auth";
let SUCCESS = 1;
let ERROR = -1;

var Client = require('node-rest-client').Client;
var client = new Client();

let fs = require('fs');

//console.log("Inicio");

exports.doAuth = function (callback){

    var code = ERROR;
    var token = "";

    fs.readFile('serial.txt', 'utf-8', (err, serial) => {
        if(err) {

            console.log('error: ', err);
            code = ERROR;
            callback(code, token);

        } else {

            //console.log(serial);

            var args = {
                data: {"serial": serial},
                headers: {"Content-Type": "application/json"}
            };

            //console.log("Post");

            client.post(URL_AUTH, args, function (data, response) {
                //console.log("client.post");

                var jsonObj = data;

                if (jsonObj.code == "001") {

                    var token = jsonObj.data.token;
                    //console.log("El token es :" + token);

                    code = SUCCESS ;
                    callback(code,token);

                }
                else {

                    console.log(data);
                    code = ERROR ;
                    callback(code,token);
                    //console.log("El codigo es: " + jsonObj.code);
                }
            });

        }
    });
}

// registering remote methods
/*    client.registerMethod("postMethod", "https://plataformamec.com/api/auth", "POST");

    console.log("postMethod");

    client.methods.postMethod(args, function (data, response) {
        // parsed response body as js object
        console.log("client.methods.postMethod");
        console.log(data);
        // raw response
        //console.log(response);
    });
*/