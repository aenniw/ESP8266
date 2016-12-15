function hexToR(h) {
    return parseInt((cutHex(h)).substring(0, 2), 16)
}

function hexToG(h) {
    return parseInt((cutHex(h)).substring(2, 4), 16)
}

function hexToB(h) {
    return parseInt((cutHex(h)).substring(4, 6), 16)
}

function cutHex(h) {
    return (h.charAt(0) == "#") ? h.substring(1, 7) : h
}

function setStripMode(mode) {
    var data = "{ \"mode\":" + mode + "}";
    CORSRequest("POST", "set-strip-mode").send(data);
}

function setStripColor(color) {
    var r = hexToR(color), g = hexToG(color), b = hexToB(color);
    var data = "{ \"color\": { \"r\" : " + r + ",\"g\" : " + g + ",\"b\" : " + b + "}}";
    CORSRequest("POST", "set-strip-color").send(data);
}

function setStripDelay(delay) {
    var data = "{ \"delay\":" + delay + "}";
    CORSRequest("POST", "set-strip-delay").send(data);
}

function setStripBrightness(brightness) {
    var data = "{ \"brightness\":" + brightness + "}";
    CORSRequest("POST", "set-strip-brightness").send(data);
}