
//Example POST method invocation
var Client = require('node-rest-client').Client;

let fs = require('fs');

var client = new Client();

console.log("Inicio");

fs.readFile('serial.txt', 'utf-8', (err, data) => {
    if(err) {
        console.log('error: ', err);
    } else {
        console.log("print " + data);
        sendPost(data);
    }
});

// set content-type header and data as json in args parameter
function sendPost(serial) {
    console.log(serial);

    var args = {
        data: {"serial": serial },
        headers: {"Content-Type": "application/json"}
    };


    console.log("Post");

    /*client.post("https://plataformamec.com/api/auth", args, function (data, response) {
     // parsed response body as js object
     console.log("client.post");
     console.log(data);
     // raw response
     //console.log(response);

     });*/

// registering remote methods
    client.registerMethod("postMethod", "https://plataformamec.com/api/auth", "POST");

    console.log("postMethod");

    client.methods.postMethod(args, function (data, response) {
        // parsed response body as js object
        console.log("client.methods.postMethod");
        console.log(data);
        // raw response
        //console.log(response);
    });

}






