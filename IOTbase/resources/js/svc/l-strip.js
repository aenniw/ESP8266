serviceOnLoad.set("svc/l-strip", function () {
    getLsConfig();
});

function decimalToHex(d, padding) {
    var hex = Number(d).toString(16).toUpperCase();
    while (hex.length < padding) {
        hex = "0" + hex;
    }
    return hex;
}

function hexToBrightness(hex) {
    var bigint = parseInt(hex, 16);
    return Math.max((bigint >> 16) & 255, (bigint >> 8) & 255, bigint & 255);
}

function getLsConfig() {
    var req = CORSRequest("GET", "led-strip/get-config");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText);
            set("ls-length", resp["length"]);
            set("ls-color", "#" + decimalToHex(resp["color"], 6));
            set("ls-speed", resp["speed"]);
            set("ls-brightness", resp["brightness"]);
            set("ls-animation-type", resp["animation-type"]);
            set("ls-mode", resp["mode"]);
            set("ls-type", resp["type"]);
        }
        refreshLsElemLayout();
    };
    req.send();
}

function setLsConfig() {
    CORSRequest("POST", "led-strip/set-config")
        .send("{ \"mode\" : " + getS("ls-mode").value + "," +
            " \"type\" : " + getS("ls-type").value + "," +
            " \"length\" : " + get("ls-length") + "}");
}

function refreshLsElemLayout() {
    if (getS("ls-animation-type").value == "0") {
        getE("ls-color").style.visibility = "visible";
        getE("ls-speed").style.visibility = "hidden";
        getE("color/speed-label").innerHTML = "Color:";
    } else {
        getE("ls-color").style.visibility = "hidden";
        getE("ls-speed").style.visibility = "visible";
        getE("color/speed-label").innerHTML = "Speed:";
    }
    var mode = getS("ls-mode").innerHTML.toUpperCase();
    if (mode.indexOf("DMA") >= 0) {
        getE("ls-pin-out").innerHTML = "RDX0/GPIO3";
    } else if (mode.indexOf("UART") >= 0) {
        getE("ls-pin-out").innerHTML = "TXD1/GPIO2";
    }
}

function updateLsAnimType() {
    CORSRequest("POST", "led-strip/set-mode")
        .send("{ \"mode\" : " + getS("ls-animation-type").value + "}");
}

function setLsColor() {
    var color = "0x" + get("ls-color").substring(1);
    CORSRequest("POST", "led-strip/set-color")
        .send("{ \"color\" : " + parseInt(color, 16) + "}");
    // update brightness value
    set("ls-brightness", hexToBrightness(color));
}
function setLsSpeed() {
    CORSRequest("POST", "led-strip/set-speed")
        .send("{ \"speed\" : " + get("ls-speed") + "}");
}

function setLsBrightness() {
    CORSRequest("POST", "led-strip/set-brightness")
        .send("{ \"brightness\" : " + get("ls-brightness") + "}");
}
