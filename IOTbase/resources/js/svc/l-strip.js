serviceOnLoad.set("svc/l-strip", function () {
    getLsConfig();
    getColorPalette();
});

const SWITCH_MIN = 130, RAINBOW_MIN = 185,
    SWITCH_RATIO = 0.8, RAINBOW_RATIO = 0.7;

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
            set("ls-brightness", resp["brightness"]);
            set("ls-animation-type", resp["animation-type"]);
            set("ls-mode", resp["mode"]);
            set("ls-type", resp["type"]);
            if (getS("ls-animation-type").value < 3) {
                set("ls-speed", Math.round(resp["speed"] - RAINBOW_MIN) / RAINBOW_RATIO);
            } else {
                set("ls-speed", Math.round(resp["speed"] - SWITCH_MIN) / SWITCH_RATIO);
            }
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
        .send("{ \"rgb\" : " + parseInt(color, 16) + "}");
    // update brightness value
    set("ls-brightness", hexToBrightness(color));
}

function setLsSpeed() {
    var lsSpeed = 0;
    if (getS("ls-animation-type").value < 3) {
        lsSpeed = RAINBOW_MIN + Math.round(get("ls-speed") * RAINBOW_RATIO);
    } else {
        lsSpeed = SWITCH_MIN + Math.round(get("ls-speed") * SWITCH_RATIO);
    }
    CORSRequest("POST", "led-strip/set-speed")
        .send("{ \"speed\" : " + lsSpeed + "}");
}

function setLsBrightness() {
    CORSRequest("POST", "led-strip/set-brightness")
        .send("{ \"brightness\" : " + get("ls-brightness") + "}");
}

// ColorPalettes stuff

function getColorPalette() {
    var req = CORSRequest("GET", "led-strip/get-animation-colors");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            clearColorsFromPalette();
            var resp = JSON.parse(req.responseText);
            for (var i = 0; i < resp["animation-colors"].length; i++) {
                addColorToPalette("#" + decimalToHex(resp["animation-colors"][i], 6), false)
            }
        }
        refreshLsElemLayout();
    };
    req.send();
}

function setColorPalette() {
    CORSRequest("POST", "led-strip/set-animation-colors")
        .send("{ \"animation-colors\" : " + getPaletteColorsJson() + "}");
}

function clearColorsFromPalette() {
    var colors = getE("ls-palette-colors").childNodes;
    while (colors.length > 2) {
        getE("ls-palette-colors").removeChild(colors[0]);
    }
}

function removeColorFromPalette(id) {
    getE("ls-palette-colors").removeChild(getE("ls-palette-" + id));
    setColorPalette();
}

function getPaletteColorsJson() {
    var colors = getE("ls-palette-colors").childNodes;
    var json = "[";
    for (var i = 0; i < colors.length; i++) {
        if (colors[i].id && colors[i].id.indexOf("ls-palette-#") !== -1) {
            json += parseInt("0x" + colors[i].id.split("-")[2].substring(1), 16);
            json += ",";
        }
    }
    if (json.charAt(json.length - 1) === ',') {
        json = json.substring(0, json.length - 2);
    }
    return json + "]";
}

function addColorToPalette(color, update) {
    var newColor = htmlToElement("<li id=\"ls-palette-" + color + "\" " +
        "onclick=\"removeColorFromPalette('" + color + "')\" " +
        "class='ls-palette-color' " +
        "type=\"button\" " +
        "style=\"background: " + color + "\">&minus;</li>");
    getE("ls-palette-colors").insertBefore(newColor, getE("ls-palette-button"));
    if (update) {
        setColorPalette();
    }
}