function xhrRequest(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  console.log('Sending XHR request:', url);
  xhr.send();
}

function locationSuccess(pos) {
  // We will request the weather here
  console.log('Position is:', pos.coords.latitude + ',' + pos.coords.longitude);
  // var baseUrl = 'http://muslimsalat.com/37.7893240,-122.4281090.json
  var url = 'http://muslimsalat.com/' + pos.coords.latitude + ',' + pos.coords.longitude + '.json';
  console.log('AJAX url is:', url);

  xhrRequest(url, 'GET', function(responseText) {
    var data = JSON.parse(responseText);
    var payload = [];
    
    console.log('Date for prayers is:', data.items[0].date_for);
    
    payload.push(data.items[0].date_for);
    payload.push(data.items[0].fajr);
    payload.push(data.items[0].shurooq);
    payload.push(data.items[0].dhuhr);
    payload.push(data.items[0].asr);
    payload.push(data.items[0].maghrib);
    payload.push(data.items[0].isha);

    Pebble.sendAppMessage(payload);
    /*
    Pebble.sendAppMessage({
      KEY_DATE_FOR: data.items[0].date_for,
      KEY_FAJR: data.items[0].fajr,
      KEY_SHUROOQ: data.items[0].shurooq,
      KEY_DHUHR: data.items[0].dhuhr,
      KEY_ASR: data.items[0].asr,
      KEY_MAGHRIB: data.items[0].maghrib,
      KEY_ISHA: data.items[0].isha
    });
    */

  });
}

function locationError(err) {
  console.log('Error requesting location!');
}

function getPrayerTimes() {
  navigator.geolocation.getCurrentPosition(locationSuccess, locationError, {
    timeout: 15000,
    maximumAge: 60000
  });
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready!');

  // Get the initial weather
  getPrayerTimes();
});

// Listen for when the watchface is opened
Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready!');
});

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage', function(e) {
  console.log('AppMessage received!');
  getPrayerTimes();
});