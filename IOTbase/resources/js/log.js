var logServer = null;
serviceOnLoad.set("log", function () {
    clearLogs();
    connectLogServer();
});

function connectLogServer() {
    if (logServer != null) {
        logServer.close();
    }
    logServer = new WebSocket("ws://" + location.host + ":8181");
    logServer.onmessage = function (event) {
        var msg = "[" + new Date().toLocaleTimeString() + "]> " + event.data + "\n";
        getE("log-text").innerHTML += msg;
        console.log(msg);
    }

}

function clearLogs() {
    getE("log-text").innerHTML = "";
}