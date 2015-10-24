function xhrRequest(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  console.log('Sending XHR request:', url);
  xhr.send();
}

// '10:23 pm' --> { hour: 22, minute: 23 }
// '10:23 am' --> { hour: 10, minute: 23 }
function parseTime(apiResponse) {
  var hour, minute, amPm;
  
  var parts1 = apiResponse.split(':'),
      parts2 = parts1[1].split(' ');
  
  amPm = parts2[1];
  minute = parseInt(parts2[0]);
  
  if ('am' === amPm) {
    if ('12' === parts1[0]) {
      hour = 0;      
    } else {
      hour = parseInt(parts1[0]);
    }
  } else {
    if (12 === parseInt(parts1[0])) {
      hour = 12;      
    } else {
      hour = parseInt(parts1[0]) + 12;
    }
  }  
  
  return {
    hour: hour,
    minute: minute
  };
}

function locationSuccess(pos) {
  // We will request the weather here
  console.log('Position is:', pos.coords.latitude + ',' + pos.coords.longitude);
  // var baseUrl = 'http://muslimsalat.com/37.7893240,-122.4281090.json
  var url = 'http://muslimsalat.com/' + pos.coords.latitude + ',' + pos.coords.longitude + '.json';
  console.log('AJAX url is:', url);

  xhrRequest(url, 'GET', function(responseText) {
    
    var data = JSON.parse(responseText),
        date, fajr, shurooq, dhuhr, asr, maghrib, isha;
        
    console.log('Date for prayers is:', data.items[0].date_for);        
    date = data.items[0].date_for.split('-');
    
    fajr = parseTime(data.items[0].fajr);
    console.log('fajr is:', fajr.hour, ':', fajr.minute);
    
    shurooq = parseTime(data.items[0].shurooq);
    console.log('shurooq is:', shurooq.hour, ':', shurooq.minute);
    
    dhuhr = parseTime(data.items[0].dhuhr);
    console.log('dhuhr is:', dhuhr.hour, ':', dhuhr.minute);
    
    asr = parseTime(data.items[0].asr);
    console.log('asr is:', asr.hour, ':', asr.minute);
    
    maghrib = parseTime(data.items[0].maghrib);
    console.log('maghrib is:', maghrib.hour, ':', maghrib.minute);
    
    isha = parseTime(data.items[0].isha);
    console.log('isha is:', isha.hour, ':', isha.minute);

    //  http://www.cplusplus.com/reference/ctime/tm/
    Pebble.sendAppMessage({

      KEY_DATE_YEAR: (parseInt(date[0]) - 1900),
      KEY_DATE_MONTH: (parseInt(date[1]) - 1),
      KEY_DATE_DAY: parseInt(date[2]),
      
      KEY_FAJR_HOUR: fajr.hour,
      KEY_FAJR_MINUTE: fajr.minute,
            
      KEY_SHUROOQ_HOUR: shurooq.hour,
      KEY_SHUROOQ_MINUTE: shurooq.minute,
      
      KEY_DHUHR_HOUR: dhuhr.hour,
      KEY_DHUHR_MINUTE: dhuhr.minute,
      
      KEY_ASR_HOUR: asr.hour,
      KEY_ASR_MINUTE: asr.minute,
      
      KEY_MAGHRIB_HOUR: maghrib.hour,
      KEY_MAGHRIB_MINUTE: maghrib.minute,
      
      KEY_ISHA_HOUR: isha.hour,
      KEY_ISHA_MINUTE: isha.minute
      
    }, function(e) {
      console.log("Weather info sent to Pebble successfully!");
    }, function(e) {
      console.log("Error sending weather info to Pebble!");
    });    
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

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage', function(e) {
  console.log('AppMessage received!');
  getPrayerTimes();
});