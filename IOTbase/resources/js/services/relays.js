serviceOnLoad.set("services/relays", function () {
    getRelays();
    getAvailablePins();
});

function generateRelayHTML(id, state) {
    id = String(id).toLowerCase();
    return '<tr id="relay-' + id + '">' +
        '<td width="33%" id="relay-' + id + '-name">Relay PIN ' + String(id).toUpperCase() + '</td>' +
        '<td id="relay-' + id + '-state">' + (state ? 'OFF' : 'ON') + '</td>' +
        '<td><input type="button" onclick="switchRelayState(' + id + ')" value="Switch"></td>' +
        '<td><input type="button" onclick="removeRelay(' + id + ')" value="Remove"></td>' +
        '</tr>';
}

function switchRelayState(id) {
    var state = (getE("relay-" + id + "-state").innerHTML == "OFF" ? false : true);
    var data = '{' + '"pin":' + id + ',"state": ' + state + ' }';
    var req = CORSRequest("POST", "set-relay-state");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText);
            if (resp["result"]) {
                getE("relay-" + id + "-state").innerHTML = (state ? "OFF" : "ON");
            }
        }
    };
    req.send(data);
}

function addRelay(id) {
    var data = '{' + '"pin":' + id + ',"type": 1 }';
    var req = CORSRequest("POST", "devices-add");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText);
            if (resp["result"]) {
                getE("relays").innerHTML += generateRelayHTML(id, false);
            }
        }
    };
    req.send(data);
}

function removeRelay(id) {
    var data = '{' + '"pin":' + id + '}';
    var req = CORSRequest("POST", "devices-remove");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText);
            if (resp["result"]) {
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
            for (var pin in resp["pins"]) {
                pinSelections += '<option value="' + resp["pins"][pin] + '">PIN ' + resp["pins"][pin] + '</option>';
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
            for (var relay in resp["devices"]) {
                relays += generateRelayHTML(resp["devices"][relay]["id"], resp["devices"][relay]["state"]);
            }
            getE("relays").innerHTML = relays;
        }
    };
    req.send();
}