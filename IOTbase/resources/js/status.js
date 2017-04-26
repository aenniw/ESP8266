serviceOnLoad.set("status", function () {
    getSystemInfo();
    getCpuInfo();
    getMemInfo();
});

function getCpuInfo() {
    var req = CORSRequest("GET", "get-cpu-info");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText),
                cur = resp["cpu-freq-cur"], max = resp["cpu-freq-max"];
            getE("info-cpu").innerHTML = cur + "Mhz / " + max + "Mhz";
            getE("info-cpu-p").style.width = Math.round((cur / max ) * 100) + "%";
        }
    };
    req.send();
}

function getSystemInfo() {
    var req = CORSRequest("GET", "get-system-info");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText);
            getE("model").innerHTML = resp["model"];
            getE("firmware").innerHTML = resp["firmware"];
            getE("chip-id").innerHTML = resp["chip-id"];
        }
    };
    req.send();
}

function getMemInfo() {
    var req = CORSRequest("GET", "get-mem-info");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText),
                sketchCur = resp["sketch-mem-free"],
                sketchMax = resp["sketch-mem-total"],
                memCur = resp["heap-free"],
                memMax = resp["heap-total"];
            getE("sketch-info").innerHTML = (sketchCur / 1000) + " kB / " + (sketchMax / 1000) + " kB";
            getE("sketch-info-p").style.width = Math.round((sketchCur / sketchMax) * 100) + "%";
            getE("mem-info").innerHTML = (memCur / 1000) + " kB / " + (memMax / 1000) + " kB";
            getE("mem-info-p").style.width = Math.round((memCur / memMax) * 100) + "%";
        }
    };
    req.send();
}