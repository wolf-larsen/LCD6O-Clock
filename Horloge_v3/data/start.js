function getParamData() {
//console.log("ParamData");
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
//console.log(this.responseText);
      var index = 0;
      var tb=this.responseText.split("|");

      var ssid = tb[index++];
      setValue("ssid",ssid);
      setValue("password",tb[index++]);

      var opt;
      aplist = gId("aplist");
      opt = document.createElement('option');
      opt.value = "";
      opt.innerHTML = "";
      aplist.appendChild(opt);
      var tbaps=tb[index++].split("$");
      for (var i = 0; i<tbaps.length; i++){
//console.log(tbaps[i]);
        var tbap = tbaps[i].split("#");
        opt = document.createElement('option');
        opt.value = tbap[0];
        opt.innerHTML = tbap[0] + " (" + tbap[1] + ")";
        aplist.appendChild(opt);
        if (tbap[0] == ssid) {
          aplist.selectedIndex = tbap[0];
        }
      }

      tmlist = gId("tmlist");
      var tbtms=tb[index++].split("#");
      for (var i = 0; i<tbtms.length; i++){
//console.log(tbtms[i]);
        var tbtm = tbtms[i].split("#");
        var opt1 = document.createElement('option');
        opt1.value = tbtm[0];
        opt1.innerHTML = tbtm[0];
        tmlist1.appendChild(opt1);
        var opt2 = document.createElement('option');
        opt2.value = tbtm[0];
        opt2.innerHTML = tbtm[0];
        tmlist2.appendChild(opt2);
      }

      setValue("displayMode",tb[index++]);
      setValue("ldroffv",tb[index++]);
      setValue("ldrfrac",tb[index++]);
      setValue("ldrhyst",tb[index++]);
      setValue("blmin",tb[index++]);
      setValue("blmax",tb[index++]);
      setValue("bltoff",tb[index++]);
      setValue("blton",tb[index++]);
      setValue("WeatherDelaySec",tb[index++]);
      setValue("IndoorWeatherDelaySec",tb[index++]);
      setValue("GPSlat",tb[index++]);
      setValue("GPSlon",tb[index++]);
      setValue("wtimezone",tb[index++]);
      setValue("NTPAdjustDelaySec",tb[index++]);
      setValue("NTPServer",tb[index++]);
      setValue("ntptimezone",tb[index++]);
      gId("al1c").checked=(tb[index++]=="Y");
      setValue("al1",tb[index++]);
      gId("al2c").checked=(tb[index++]=="Y");
      setValue("al2",tb[index++]);
      for (var il = 1; il<=3; il++){
       for (var id = 0; id<=3; id++){
          setValue("ss"+il+id, tb[index++]);
       }
      }
    }
  };
  xhr.open("GET", "getParamData", true);
  xhr.send();
}

function updssid() {
  gId("ssid").value=gId("aplist").value;
}

function sendOrder(order) {
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
        console.log("Cmd: "+order+"  ->OK");
    }
  };
  xhr.ontimeout = function() {
    console.log("TimeOut sendOrder");
  };
  xhr.open("GET", "sendOrder?order="+order, true);
  xhr.send();
}

function gId(id) {
  return document.getElementById(id);
}
function setValue(id,idvalue) {
  var gid=gId(id);
  if (gid) {
	gid.value=idvalue;
  }
}

window.onload = getParamData;

