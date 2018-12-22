struct DetonatorChannel {
  byte pinNumber;
  String name;
  bool on;
  bool fired;
  bool error;
  unsigned long changed;
};

const char *title[] = {
    " __  ___      ___      .______     ______     ______   .___  ___. ",
    "|  |/  /     /   \\     |   _  \\   /  __  \\   /  __  \\  |   \\/   | ",
    "|  '  /     /  ^  \\    |  |_)  | |  |  |  | |  |  |  | |  \\  /  | ",
    "|    <     /  /_\\  \\   |   _  <  |  |  |  | |  |  |  | |  |\\/|  | ",
    "|  .  \\   /  _____  \\  |  |_)  | |  `--'  | |  `--'  | |  |  |  | ",
    "|__|\\__\\ /__/     \\__\\ |______/   \\______/   \\______/  |__|  |__| "
};
const char *title2[] = {
" __ _   __   ____   __    __   _  _ ",
"(  / ) / _\\ (  _ \\ /  \\  /  \\ ( \\/ )",
" )  ( /    \\ ) _ ((  O )(  O )/ \\/ \\",
"(__\\_)\\_/\\_/(____/ \\__/  \\__/ \\_)(_/"
};

const char *html1 = "\
  <!DOCTYPE HTML>\
  <html>\
  <head><title>Kaboom fire control</title></head>\
  <body>\
  <style>body {display: flex; font-size: 20px; flex-direction: column; align-items: center;}\
  table {display: flex; flex-direction: column; align-items: center; width:80%; border-collapse: collapse;}\
  td {border: 1px solid black; border-collapse: collapse; padding: 20px; min-width: 50px; text-align: center;}\
  button {padding: 15px 30px; border-radius: 10px; border: 1px solid red; background-image: linear-gradient(to bottom, rgba(255,120,120), rgba(255,40,40)); font-size: 16px; font-weight:bold;}\
  button:hover {opacity: 0.8;}\
  button:disabled{opacity: 0.5;}\
  </style>\
  <table>\
  <tr><th>Name</th><th>Pin #</th><th>Firing</th><th>Fired</th><th></th></tr>\
";

const char *html2 = "\
  </table>\
  <script type=\"text/javascript\">\
  var buttonPushed;\
  function fire(e) {\
    var pin = e.currentTarget.id.substring(6);\
    document.getElementById('firing' + pin).style.backgroundColor = \"orange\";\
    setTimeout(function(p) { return function() {\
      document.getElementById('firing' + p).style.backgroundColor = \"white\";\
    };}(pin), 2000);\
    var xhr = new XMLHttpRequest();\
    xhr.onreadystatechange = function(p) {\
      return function() {\
        if (this.readyState == XMLHttpRequest.DONE) {\
          if (this.status == 200) {\
            document.getElementById('button' + p).disabled = true;\
            document.getElementById('fired' + p).style.backgroundColor = \"#8f8\";\
            document.getElementById('fired' + p).color = \"black\";\
            document.getElementById('fired' + p).innerHTML = \"<b>OK</b>\";\
          } else {\
            document.getElementById('fired' + p).style.backgroundColor = \"#f88\";\
            document.getElementById('fired' + p).color = \"black\";\
            document.getElementById('fired' + p).innerHTML = \"<b><i>ERR</b></i>\";\
          }\
        }\
      };\
    }(pin);\
    xhr.open('GET', '/fire?pin=' + pin, true);\
    xhr.send();\
  }\
  </script></body></html>\
";
