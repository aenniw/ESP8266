function updateNtpTime(provider) {
    var data = "{ \"provider\":" + provider + "}";
    CORSRequest("POST", "update-time-ntp").send(data);
}

function setTime(time) {
    var data = "{ \"time\":" + provider + "}";
    CORSRequest("POST", "update-time").send(data);
}

function getTime(setTime) {
    var req = CORSRequest("GET", "get-time");
    req.onreadystatechange = function () {
        if (xhr.readyState == 4 && xhr.status == 200) {
            setTime(JSON.parse(xhr.responseText).time);
        }
    };
    req.send(data)
}
