serviceOnLoad.set("svc/d-io", function () {
    getDigitalIO();
    getAvailablePins();
});

function generateDigitalIOHTML(id, state) {
    id = String(id).toLowerCase();
    return '<tr id="d-io-' + id + '">' +
        '<td width="33%" id="d-io-' + id + '-name">IO PIN ' + String(id).toUpperCase() + '</td>' +
        '<td id="d-io-' + id + '-state">' + (state ? 'OFF' : 'ON') + '</td>' +
        '<td><input type="button" onclick="switchDigitalIOState(' + id + ')" value="Switch"></td>' +
        '<td><input type="button" onclick="removeDigitalIO(' + id + ')" value="Remove"></td>' +
        '</tr>';
}

function switchDigitalIOState(id) {
    var state = (getE("d-io-" + id + "-state").innerHTML == "OFF" ? false : true);
    var data = '{' + '"pin":' + id + ',"state": ' + state + ' }';
    var req = CORSRequest("POST", "set-d-io-state");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText);
            if (resp["result"]) {
                getE("d-io-" + id + "-state").innerHTML = (state ? "OFF" : "ON");
            }
        }
    };
    req.send(data);
}

function addDigitalIO(id) {
    var data = '{' + '"pin":' + id + ',"type": 1 }';
    var req = CORSRequest("POST", "devices-add");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText);
            if (resp["result"]) {
                getE("d-ios").innerHTML += generateDigitalIOHTML(id, false);
            }
        }
    };
    req.send(data);
}

function removeDigitalIO(id) {
    var data = '{' + '"pin":' + id + '}';
    var req = CORSRequest("POST", "devices-remove");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText);
            if (resp["result"]) {
                getE("d-ios").removeChild(getE("d-io-" + id));
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
            getE("available-d-io-pins").innerHTML = pinSelections;
        }
    };
    req.send();
}

function getDigitalIO() {
    var req = CORSRequest("GET", "devices-get-d-io");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText);
            var devices = "";
            for (var device in resp["devices"]) {
                devices += generateDigitalIOHTML(resp["devices"][device]["id"], resp["devices"][device]["state"]);
            }
            getE("d-ios").innerHTML = devices;
        }
    };
    req.send();
}