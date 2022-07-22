// used when hosting the site on the ESP8266
var address = location.hostname;
var urlBase = "";

// used when hosting the site somewhere other than the ESP8266 (handy for testing without waiting forever to upload to SPIFFS/LittleFS)
// var address = "192.168.86.36";
// var urlBase = "http://" + address + "/";

$(document).ready(function() {
    $("#status").html("Ready");
});

function postValue(name, value) {
    $("#status").html("Setting " + name + ": " + value + ", please wait...");
    var body = { name: name, value: value };
    $.post(urlBase + name, body, function(data) {
      if (data.name != null) {
        $("#status").html("Set " + name + ": " + data.name);
      } else {
        $("#status").html("Set " + name + ": " + data);
      }
    });
  }