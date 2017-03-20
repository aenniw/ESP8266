var logServer = null;

function connectLogServer() {
    if (logServer == null) {
        logServer = new WebSocket("ws://" + location.host + ":8181");
        logServer.onmessage = function (event) {
            var msg = "  " + new Date().toLocaleTimeString() + "  " + event.data + "\n";
            getE("log-text").innerHTML += msg;
            console.log(msg);
        }
    }
}

function disconnectLogServer() {
    if (logServer != null) {
        logServer.close();
        logServer = null;
    }
}

function clearLogs() {
    getE("log-text").innerHTML = "";
}