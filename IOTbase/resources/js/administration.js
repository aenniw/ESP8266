serviceOnLoad.set("administration", function () {
    getGlobalConfig();
    getWifiConfig();
    getWifiApConfig();
    getWifiStaConfig();
});

function getGlobalConfig() {
    var req = CORSRequest("GET", "get-config-global");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText);
            set("user", resp["rest-acc"]);
            set("pass", resp["rest-pass"]);
        }
    };
    req.send();
}

function setGlobalConfig() {
    var data = "{" +
        " rest-acc:" + "\"" + get("user") + "\"" + "," +
        " rest-pass:" + "\"" + get("pass") + "\"" +
        "}";
    var req = CORSRequest("POST", "set-config-global");
    req.onreadystatechange = function () {
        if (req.readyState == 4) {
            getGlobalConfig();
        }
    };
    req.send(data);
}

function getNetworksInfo() {
    var req = CORSRequest("GET", "get-networks");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText);
            getE("sta-ssid").innerHTML = "";
            for (var i in resp["networks"]) {
                getE("sta-ssid").innerHTML += "<option value=" + i + ">" + resp["networks"][i] + "</option>\n";
            }
        }
    };
    req.send();
}

function getWifiConfig() {
    var req = CORSRequest("GET", "get-config-wifi");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText);
            getE("mac").innerHTML = resp["mac"];
            set("wifi-mode", resp["mode"]);
            set("hostname", resp["hostname"]);
        }
    };
    req.send();
}

function setWifiConfig() {
    // TODO: Add Mac update
    var data = "{" +
        " mode:" + "\"" + getS("wifi-mode").value + "\"" + "," +
        " hostname:" + "\"" + get("hostname") + "\"" +
        "}";
    var req = CORSRequest("POST", "set-config-wifi");
    req.onreadystatechange = function () {
        if (req.readyState == 4) {
            getWifiConfig();
        }
    };
    req.send(data);
}

function getWifiApConfig() {
    var req = CORSRequest("GET", "get-config-wifi-ap");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText);
            getE("ap-ip").innerHTML = resp["ip"];
            set("ap-ssid", resp["ssid"]);
            set("ap-channel", resp["channel"]);
            set("ap-hidden", resp["hidden"]);
        }
    };
    req.send();
}

function setWifiApConfig() {
    var data = "{" +
        " ssid:" + "\"" + get("ap-ssid") + "\"" + "," +
        " pass:" + "\"" + get("ap-pass") + "\"" + "," +
        " hidden:" + "\"" + get("ap-hidden") + "\"" + "," +
        " channel:" + "\"" + get("ap-channel") + "\"" +
        "}";
    var req = CORSRequest("POST", "set-config-wifi-ap");
    req.onreadystatechange = function () {
        if (req.readyState == 4) {
            getWifiApConfig();
        }
    };
    req.send(data);
}

function getWifiStaConfig() {
    var req = CORSRequest("GET", "get-config-wifi-sta");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText);
            getE("sta-status").innerHTML = (resp["status"] == 1 ? "Connected" : "Disconnected");
            getE("sta-ssid").innerHTML = "<option value=\"0\">" + resp["ssid"] + "</option>";
            getE("sta-ip").innerHTML = resp["ip"];
        }
    };
    req.send();
}

function setWifiStaConfig() {
    var data = "{" +
        " ssid:" + "\"" + getS("sta-ssid").innerHTML + "\"" + "," +
        " pass:" + "\"" + get("sta-pass") + "\"" +
        "}";
    var req = CORSRequest("POST", "set-config-wifi-sta");
    req.onreadystatechange = function () {
        if (req.readyState == 4) {
            getWifiStaConfig()
        }
    };
    req.send(data);
}