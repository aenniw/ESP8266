var pageCashe = new Map(),
    serviceOnLoad = new Map();

serviceOnLoad.set("status", function statusLoad() {
    getSystemInfo();
    getCpuInfo();
    getMemInfo();
});
serviceOnLoad.set("administration", function adminLoad() {
    getGlobalInfo();
    getAPInfo();
    getSTAInfo();
});


function loadResource(id, service, loadJs) {
    if (pageCashe.get(service) != null) {
        getE(id).innerHTML = pageCashe.get(service);
        if (serviceOnLoad.get(service) != null) serviceOnLoad.get(service)();
    } else {
        if (loadJs) {
            var script = document.createElement('script');
            script.type = 'text/javascript';
            script.src = "js/" + service + ".js";
            document.getElementsByTagName('head')[0].appendChild(script);
        }
        var req = CORSRequest('GET', "html/" + service + ".html");
        req.onreadystatechange = function () {
            if (req.readyState == 4 && req.status == 200) {
                pageCashe.set(service, req.responseText);
                getE(id).innerHTML = req.responseText;
                if (serviceOnLoad.get(service) != null) serviceOnLoad.get(service)();
            }
        };
        req.send();
    }
}

function restart() {
    CORSRequest("POST", "restart").send();
}

function login() {
    var req = CORSRequest('GET', 'login');
    req.onreadystatechange = function () {
        if (req.readyState == 4) {
            location.reload();
        }
    };
    req.send();
}

function logout() {
    var req = CORSRequest('GET', 'logout');
    req.onreadystatechange = function () {
        if (req.readyState == 4) {
            location.reload();
        }
    };
    req.send();
}

function get(id) {
    return getE(id).value;
}

function set(id, value) {
    getE(id).value = value;
}

function getE(id) {
    return document.getElementById(id);
}

function CORSRequest(method, path) {
    var xhr = new XMLHttpRequest(),
        url = "http://" + location.host + "/" + path;
    if ("withCredentials" in xhr) {
        xhr.open(method, url, true);
    } else if (typeof XDomainRequest != "undefined") {
        xhr = new XDomainRequest();
        xhr.open(method, url);
    } else {
        xhr = null;
    }
    if (getE("user") != null && getE("pass") != null) {
        xhr.setRequestHeader("Authorization", "Basic " + btoa(getE("user").value + ":" + getE("pass").value));
    }
    xhr.setRequestHeader("Content-Type", "application/json");
    xhr.setRequestHeader("Accept", "application/json");
    xhr.onreadystatechange = function () {
        if (xhr.readyState == 4) {
            console.log(xhr.responseText);
        }
    };
    return xhr;
}


function getGlobalInfo() {
    var req = CORSRequest("GET", "get-config-global");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText);
            set("user", resp["acc"]);
            set("pass", resp["pass"]);
        }
    };
    req.send();
}


function getCpuInfo() {
    var req = CORSRequest("GET", "get-cpu-info");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText),
                cur = resp["cpu-freq-cur"], max = resp["cpu-freq-max"];
            getE("info-cpu").innerHTML = cur + "Mhz / " + max + "Mhz";
            getE("info-cpu-p").style.width = ((cur / max ) * 100) + "%";
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
            getE("up-time").innerHTML = resp["up-time"];
        }
    };
    req.send();
}

function getMemInfo() {
    var req = CORSRequest("GET", "get-system-info");
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == 200) {
            var resp = JSON.parse(req.responseText),
                sketchCur = resp["sketch-mem-free"],
                sketchMax = resp["sketch-mem-total"],
                memCur = resp["heap-free"],
                memMax = resp["heap-total"];
            getE("sketch-info").innerHTML = sketchCur + " b / " + sketchMax + " b";
            getE("sketch-info-p").innerHTML = ((sketchCur / sketchMax) * 100 ) + "%";
            getE("mem-info").innerHTML = memCur + " b / " + memMax + " b";
            getE("mem-info-p").innerHTML = ((memCur / memMax) * 100 ) + "%";
        }
    };
    req.send();
}