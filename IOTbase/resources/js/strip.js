serviceOnLoad.set("strip", getConfigStrip);
getConfigStrip();

function getConfigStrip() {
    var req = CORSRequest("GET", "get-config-strip");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText);
            set('count', resp["led-count"]);
            set('mode', resp["mode"]);
            set('color', "#" + resp["color"].toString(16));
            set('brightness', resp["brightness"]);
            set('delay', resp["delay"]);
        }
    };
    req.send()
}

function setConfigStrip(p) {
    CORSRequest("POST", "set-config-strip").send("{\"persist\" : " + p +
        ",\"led-count\": " + get('count') +
        ",\"mode\": " + get('mode') +
        (get('mode') == -2 ? ",\"color\": " + parseInt(get('color').toString().replace("#", "0x"), 16) : "") +
        ",\"brightness\": " + get('brightness') +
        ",\"delay\": " + get('delay') + "}");
}