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

const char *html1 = "\
  <!DOCTYPE HTML>\
  <html>\
  <head><title>Kaboom fire control</title></head>\
  <body>\
  <style>body {display: flex; font-size: 20px; flex-direction: column; justify-content: center; align-items: center; }\
  table {width:80%; border:1px solid black; border-collapse: collapse;}\
  td {border: 1px solid black; border-collapse: collapse;}\
  button {margin: 10px;}\
  </style>\
  <table>\
  <tr><th>Name</th><th>Pin #</th><th>Firing</th><th>Fired</th><th></th></tr>\
";

const char *html2 = "\
  </table>\
  <script type=\"text/javascript\">\
    function fire(e) {\
    var pin = e.currentTarget.id.substring(6);\
    document.getElementById('firing' + pin).backgroundColor = \"#red\";\
    setTimeout(function(p) { return function() {\
      document.getElementById('firing' + p).backgroundColor = \"#white\";\
    };}(pin), 2000);\
    var xhr = new XMLHttpRequest();\
    xhr.onreadystatechange = function(p) {\
      return function() {\
        if (this.readyState == XMLHttpRequest.DONE) {\
          if (this.status == 200) {\
            document.getElementById('button' + p).disabled = true;\
            document.getElementById('fired' + p).backgroundColor = \"#green\";\
            document.getElementById('fired' + p).color = \"#black\";\
            document.getElementById('fired' + p).innerHTML = \"<b>OK</b>\";\
          } else {\
            document.getElementById('fired' + p).backgroundColor = \"#red\";\
            document.getElementById('fired' + p).color = \"#black\";\
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
