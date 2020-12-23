package main

import (
	rpio "github.com/stianeikeland/go-rpio"
	"net/http"
	"encoding/json"
	"fmt"
	"time"
	"errors"
	"strings"
	"strconv"
)

type Chan struct {
	Pin int `json:"pin"`
	Fired bool `json:"fired"`
	Firing bool `json:"firing"`
	StartedFiring time.Time `json:"started_firing"`
}

var ( 
	chans []*Chan = []*Chan{
		&Chan{ Pin: 25 },
		&Chan{ Pin: 24 },
		&Chan{ Pin: 21 },
		&Chan{ Pin: 12 },
		&Chan{ Pin: 23 },
		&Chan{ Pin: 16 },
		&Chan{ Pin: 18 },
		&Chan{ Pin: 20 },
	}
	ErrChanNotFound error = errors.New("channel not found")
	ErrChanFiring error = errors.New("channel already firing")
	debuglevel int = 1
)

func main() {
	if err := rpio.Open(); err != nil {
		panic(fmt.Sprintf("Failed to init go-rpio: %s\n", err.Error()))
	}
	for _, ch := range chans {
		p := rpio.Pin(ch.Pin)
		p.Mode(rpio.Output)
		p.Write(rpio.Low)
	}
	http.HandleFunc("/", HandleRequest)
	http.ListenAndServe(":8888", nil)
}

func HandleRequest(w http.ResponseWriter, r *http.Request) {
	fmt.Printf("Got request for %s\n", r.URL.Path)
	if strings.HasPrefix(r.URL.Path, "/fire") {
		r.ParseForm()
		if chNumbers, ok := r.Form["chan"]; ok {
			for _, chStr := range chNumbers {
				DoFire(w, chStr)
			}
		} else {
			http.Error(w, "Missing chan=n/test query parameter", http.StatusBadRequest)
			return
		}
		if jsonStr, err := json.Marshal(chans); err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		} else {
			w.Header().Set("Content-Type", "application/json")
			fmt.Fprint(w, jsonStr)
		}
	} else {
		DoIndex(w, r)
	}
}

func _fireStop(chNumberInternal int) {
	c := chans[chNumberInternal]
	fmt.Printf("%s _fireStop: turning off channel %d (pin %d)\n", time.Now().String(), chNumberInternal, c.Pin)
	p := rpio.Pin(c.Pin)
	p.Write(rpio.Low)
	c.Fired = true
	c.Firing = false
}

func _fireStart(chNumberInternal int) error {
	c := chans[chNumberInternal]
	fmt.Printf("%s _fireStart: got told to fire internal channel %d (pin %d)\n", time.Now().String(), chNumberInternal, c.Pin)
	if !c.Firing {
		fmt.Printf("_fireStart: channel %d was not firing, proceeding with fire\n", chNumberInternal)
		p := rpio.Pin(c.Pin)
		p.Write(rpio.High)
		c.Firing = true
		c.StartedFiring = time.Now()
		go func() {
			time.Sleep(time.Duration(2 * time.Second))
			_fireStop(chNumberInternal)
		}()
		return nil
	}
	return ErrChanFiring
}

func DoFire(w http.ResponseWriter, chStr string) {
	fmt.Printf("DoFire: trying to fire channel %s\n", chStr)
	if chStr == "test" {
		for chNumber, _ := range chans {
			if err := _fireStart(chNumber); err != nil {
				http.Error(w, fmt.Sprintf("Warning: channel %d was already firing", chNumber+1), 218)
			}
		}
	} else {
		chNumber, err := strconv.Atoi(chStr)
		if err != nil {
			http.Error(w, "Bad request: no such channel", http.StatusBadRequest)
			return
		}
		if chNumber < 1 || chNumber > len(chans) {
			http.Error(w, "Bad request: no such channel", http.StatusBadRequest)
			return
		}
		if err = _fireStart(chNumber-1); err != nil {
			http.Error(w, err.Error(), 555)
			return
		}
	}
}



func DoIndex(w http.ResponseWriter, r *http.Request) {
	fmt.Fprint(w, html1)
	for chNum, chOb := range chans {
		outStr := strings.Replace(html2, "NAME", fmt.Sprintf("Ch %d", chNum+1),1)
		outStr = strings.ReplaceAll(outStr, "CHAN", fmt.Sprintf("%d", chNum+1))
		if chOb.Fired {
			outStr = strings.Replace(outStr, "DISABLED", "disabled", 1)
		} else {
			outStr = strings.Replace(outStr, "DISABLED", "", 1)
		}
		fmt.Fprint(w, outStr)
	}
	fmt.Fprint(w, html3)
}



const html1 = `
<!DOCTYPE HTML>
<html>
	<head>
		<title>Kaboom fire control</title>
	</head>
	<body>
		<style>
			body {
				display: flex;
				flex-direction: column;
				align-items: center;
				font-size: 20px;
			}
			body > * {
				margin-bottom: 20px;
			}
			table {
				display: flex;
				flex-direction: column;
				align-items: center;
				width: 80%;
				border-collapse: collapse;
			}
			td {
				border: 1px solid black;
				border-collapse: collapse;
				padding: 20px;
				min-width: 50px;
				text-align: center;
			}
			button {
				padding: 15px 30px;
				border-radius: 10px;
				border: 1px solid red;
				background-image: linear-gradient(to bottom, rgba(255,120,120), rgba(255,40,40));
				font-size: 16px;
				font-weight: bold;
			}
			button:hover {
				opacity: 0.8;
			}
			button:disabled {
				opacity: 0.5;
			}
		</style>
		<table>
			<tr>
				<th>Name</th>
				<th>Channel #</th>
				<th>Firing</th>
				<th>Fired</th>
				<th></th>
			</tr>

`

const html2 = `
			<tr>
				<td>NAME</td>
				<td>CHAN</td>
				<td id="firingCHAN" style="background-color: white; color: black;"></td>
				<td id="firedCHAN" style="background-color: white; color: black;"></td>
				<td><button id="buttonCHAN" onclick="fire(event);"DISABLED>fire</button></td>
			</tr>
`

const html3 = `
		</table>
		<button onclick="testFire(event);">Fire all channels</button>
		<script type="text/javascript">
			async function testFire(e) {
				for (var chan = 1; chan <= 8; chan++) {
					_fire(chan);
					await new Promise(r => setTimeout(r, 500));
				}
			}
			function _fire(chan) {
				document.getElementById("firing" + chan).style.backgroundColor = "orange";
				setTimeout(function(p) { return function() {
					document.getElementById("firing" + p).style.backgroundColor = "white";
				};}(chan), 2000);
				var xhr = new XMLHttpRequest();
				xhr.onreadystatechange = function(p) {
					return function() {
						if (this.readyState == XMLHttpRequest.DONE) {
							if (this.status == 200) {
								document.getElementById("button" + p).disabled = true;
								document.getElementById("fired" + p).style.backgroundColor = "#8f8";
								document.getElementById("fired" + p).color = "black";
								document.getElementById("fired" + p).innerHTML = "<b>OK</b>";
							} else {
								document.getElementById("fired" + p).style.backgroundColor = "#f88";
								document.getElementById("fired" + p).color = "black";
								document.getElementById("fired" + p).innerHTML = "<b><i>ERR</b></i>";
							}
						}
					};
				}(chan);
				xhr.open("GET", "/fire?chan=" + chan, true);
				xhr.send();
			}
			function fire(e) {
				var chan = e.currentTarget.id.substring(6);
				_fire(chan);
			}
		</script>
	</body>
</html>

`


