I'm going to try setting up the wifi server from the C3. I've plugged it in and am trying the WifiServer example sketch and have this [tutorial](https://www.youtube.com/watch?v=aH3sLEQI4_w&t=21s) open. I don't really care about turning on/off the LED, I just want to see if it connects to the web server, I might connect it by toggling on/off the backlight LED of the touchscreen.

I kept trying to upload the WifiServer sketch and it wouldn't work, then remembered the C3 came with another piece that's like a connector, I'll try adding it and running. So it's a Ipex connector connects the WiFi, bluetooth antenna to increase signal strength, so the code should be running without it but it may help to add. Sweet it worked and printed my the C3's IP.

```
#include <Arduino.h>
#include <WiFi.h>

const char *ssid = "***";
const char *password = "***";

NetworkServer server(80);

void setup() {
  Serial.begin(115200);

  delay(10);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  NetworkClient client = server.accept();  // listen for incoming clients

  if (client) {                     // if you get a client,
    Serial.println("New Client.");  // print a message out the serial port
    String currentLine = "";        // make a String to hold incoming data from the client
    while (client.connected()) {    // loop while the client's connected
      if (client.available()) {     // if there's bytes to read from the client,
        char c = client.read();     // read a byte, then
        Serial.write(c);            // print it out the serial monitor
        if (c == '\n') {            // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> to turn the LED on pin 5 on.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn the LED on pin 5 off.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {  // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}
```

If I want to make an actual website (maybe use [this tumblr website](https://theslapdotgov.tumblr.com/) as a reference) I'll create the website files within the C3 project instead of putting it in another project and directing to its domain. Instead of using a specific IP address I could also look into using mDNS for friendlier links.
