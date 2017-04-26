serviceOnLoad.set("services/relays", function () {
    getRelays();
    getAvailablePins();
});

function generateRelayHTML(id, state) {
    id = String(id).toLowerCase();
    return '<tr id="relay-"' + id + '>' +
        '<td width="33%" id="relay-' + id + '-name">Relay ' + String(id).toUpperCase() + '</td>' +
        '<td id="relay-' + id + '-state">' + (state ? 'ON' : 'OFF') + '</td>' +
        '<td><input type="button" onclick="switchRelayState(' + id + ')" value="Switch"></td>' +
        '<td><input type="button" onclick="removeRelay(' + id + ')" value="Remove"></td>' +
        '</tr>';
}

function switchRelayState(id) {
    var data = '{' + '"state":' + (getE("relay-" + id + "state").innerHTML == "ON" ? false : true) + '}';
    var req = CORSRequest("POST", "set-relay-state");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText);
            getE("relay-" + id + "state").innerHTML = (resp["status"] == 1 ? "ON" : "OFF");
        }
    };
    req.send(data);
}

function addRelay(id) {
    var data = '{' + '"pin":"' + getS(id).value + '"}';
    var req = CORSRequest("POST", "devices-add");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText);
            if (resp["status"] == 1) {
                getE("relays").innerHTML += generateRelayHTML(id, false);
            }
        }
    };
    req.send(data);
}

function removeRelay(id) {
    var data = '{' + '"pin":"' + getS(id).value + '"}';
    var req = CORSRequest("POST", "devices-remove");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText);
            if (resp["status"] == 1) {
                getE("relays").removeChild(getE("relay-" + id));
            }
        }
    };
    req.send(data);
}

function getAvailablePins() {
    var req = CORSRequest("GET", "devices-get-available-pins");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText);
            var pinSelections = "";
            for (var pin in resp["available-pins"]) {
                pinSelections += '<option value="' + pin + '">PIN ' + pin + '</option>';
            }
            getE("available-relay-pins").innerHTML = pinSelections;
        }
    };
    req.send();
}

function getRelays() {
    var req = CORSRequest("GET", "devices-get-relays");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText);
            var relays = "";
            for (var i = 0; i < resp["len"]; i++) {
                relays += generateRelayHTML(resp[i]["id"], resp[i]["state"]);
            }
            getE("relays").innerHTML = relays;
        }
    };
    req.send();
}