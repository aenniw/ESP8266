var pageCashe = new Map(),
    serviceOnLoad = new Map();

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
                setTimeout(function () {
                    if (serviceOnLoad.get(service) != null) {
                        serviceOnLoad.get(service)();
                    }
                }, 200);
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