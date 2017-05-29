var hidServer = null;
serviceOnLoad.set("hid", function () {
    getE("log-text-hid").innerHTML = "";
    connectHIDServer();
});

function connectHIDServer() {
    if (hidServer != null) {
        hidServer.close();
    }
    hidServer = new WebSocket("ws://" + location.host + ":9011");
    hidServer.onmessage = function (event) {
        var msg = "[" + new Date().toLocaleTimeString() + "]> " + event.data + "\n";
        getE("log-text-hid").innerHTML += msg;
        console.log(msg);
    }

}

function sendHIDCommand() {
    if (event.keyCode == 13) {
        var cmd = getS("at-mode").value + " " + get("hid-command");
        hidServer.send(cmd);
        getE("log-text-hid").innerHTML += "[" + new Date().toLocaleTimeString() + "]> " + cmd + "\n";
        set("hid-command", "");
    }
}

