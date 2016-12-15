var pageCashe = new Map();


function onMenuClick(id, uri) {
    if (pageCashe.get(uri) != null) {
        document.getElementById(id).innerHTML = pageCashe.get(uri);
    } else {
        var req = CORSRequest('GET', uri);
        req.onreadystatechange = function () {
            if (req.readyState == 4 && req.status == 200) {
                pageCashe.set(uri, req.responseText);
                document.getElementById(id).innerHTML = req.responseText
            }
        };
        req.send();
    }
}

function restartDevice() {
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
    if (document.getElementById("user") != null && document.getElementById("pass") != null) {
        xhr.setRequestHeader("Authorization", "Basic " + btoa(document.getElementById("user").value + ":" + document.getElementById("pass").value));
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