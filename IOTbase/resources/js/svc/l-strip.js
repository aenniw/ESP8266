serviceOnLoad.set("svc/l-strip", function () {
    getLsConfig();
    refreshLsElemLayout();
});

function getLsConfig() {

}

function setLsConfig() {

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
}

function updateLsMode() {

}

function setLsColor() {

}
function setLsSpeed() {

}

function setLsBrightness() {

}