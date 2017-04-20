// Pre-exit scripts
exports.preExitFunc = function () {
    var preExit = [];

// Catch exit
    process.stdin.resume ();
    process.on ('exit', function (code) {
        var i;

        console.log ('Process exit');

        for (i = 0; i < preExit.length; i++) {
            preExit [i] (code);
        }

        process.exit (code);
    });

// Catch CTRL+C
    process.on ('SIGINT', function () {
        console.log ('\nCTRL+C...');
        process.exit (0);
    });

// Catch uncaught exception
    process.on ('uncaughtException', function (err) {
        console.dir (err, { depth: null });
        process.exit (1);
    });


// INSERT CODE
    console.log ('Se ha presentado un error - presiona CTRL+C para salir');

// Add pre-exit script
    preExit.push (function (code) {
        console.log ('Codigo de salida %d, limpiando...', code);
        // i.e. close database
    });

}

