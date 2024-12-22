window.onload = function() {
   loadConfig();
  //loadSettings();
  requestUpdate();
 
};


function loadSettings(){
  var xhttp = new XMLHttpRequest();
  xhttp.timeout = 2000; // time in milliseconds
  xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        decodeJson (this.responseText);     
      }
    };

  xhttp.open("GET", "settings.json", true);
  xhttp.send(); 
}

function loadConfig(){
  var xhttp = new XMLHttpRequest();
  xhttp.timeout = 2000; // time in milliseconds
  xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        decodeJson (this.responseText);   
        SetDevName();  
      }
    };

  xhttp.open("GET", "config.json", true);
  xhttp.send(); 
}

function requestUpdate(){
 
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          decodeJson (this.responseText);     
          setTimeout("requestUpdate()",500);
        }
      };
    //xhttp.open("GET", "settings.json", true);  
    xhttp.open("GET", "data.json", true);
    xhttp.send();     
}

function decodeJson (JSONText)
{
  const oJSON = JSON.parse(JSONText);

  Object.entries(oJSON).forEach((entry) => {
      const [key, value] = entry;
      var elements = document.getElementsByName(key);
      for (let i = 0; i < elements.length; i++) {
        const obj = elements[i];
        if (key == "Epoch")
        {
          var date = new Date(0);
          date.setUTCSeconds(value); 
          obj.innerHTML = date.toLocaleString();
        }
        else if (value == "on")
          obj.checked = true;
        else if (value == "off")
          obj.checked = false;
        else if (obj.tagName === "SELECT")
        {
          obj.value = value;
        }
        else
        {
          obj.innerHTML = value;
          obj.value = value;
        }
      }
    })
}

function SetDevName ()
{
  var DevName = document.getElementById("DevName").innerText;
  document.title = DevName ;
  document.getElementById("DevNameInput").value = DevName;
}


function ConfigDiv() {
  var x = document.getElementById("ConfigDiv");
  if (x.style.display === "none") {
    x.style.display = "block";
  } else {
    x.style.display = "none";
  }
} 
