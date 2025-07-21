// wifi.cpp
#include "WiFiWebConfigurator.h"

ESP8266WebServer server(80);

static String ssid_global;
static String password_global;
static bool ledFlag = false;
static bool connected = false;

const char *config_page = R"HTML(
<!DOCTYPE html>
<html lang="zh">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Wi-Fi Config</title>
  <style>
    body {
      margin: 0;
      padding: 0;
      background: linear-gradient(145deg, #eef2f3, #d6dbe1);
      font-family: "Helvetica Neue", sans-serif;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
    }

    .card {
      background: #ffffff;
      padding: 30px 40px;
      border-radius: 16px;
      box-shadow: 0 8px 24px rgba(0, 0, 0, 0.1);
      width: 320px;
    }

    .card h2 {
      text-align: center;
      color: #333333;
      margin-bottom: 24px;
      font-weight: normal;
    }

    label {
      display: block;
      font-size: 14px;
      margin-bottom: 6px;
      color: #555;
    }

    #switchpw {
      display: block;
      margin: -10px 0 18px auto;
      font-size: 13px;
      background: none;
      border: none;
      color: #3f51b5;
      cursor: pointer;
      text-align: right;
      padding: 0;
    }

    input[type="text"],
    input[type="password"] {
      width: 100%;
      padding: 10px 12px;
      margin-bottom: 18px;
      border: 1px solid #ccc;
      border-radius: 10px;
      font-size: 15px;
      box-sizing: border-box;
      transition: border 0.2s;
    }

    input[type="text"]:focus,
    input[type="password"]:focus {
      border-color: #3f51b5;
      outline: none;
    }

    input[type="button"] {
      width: 100%;
      padding: 10px;
      background: #3f51b5;
      border: none;
      border-radius: 10px;
      color: white;
      font-size: 16px;
      cursor: pointer;
      transition: 0.3s;
    }

    input[type="button"]:hover {
      background: #2c3ca0;
    }
  </style>
</head>
<body>
  <div class="card">
    <h2>ESP8266_Wi-Fi Config</h2>
    <div id="config">
      <label for="ssid">SSID</label>
      <input type="text" id="ssid" placeholder="input Wi-Fi SSID" />

      <label for="pw">Password</label>
      <input type="password" id="pw" placeholder="input Wi-Fi password" />
      <button type="button" id="switchpw" onclick="switch_password()">Show</button>

      <input type="button" value="connect" onclick="submit_wificonfig()" />
    </div>
  </div>

  <script>
    function submit_wificonfig() {
      var ssid = document.getElementById("ssid").value;
      var password = document.getElementById("pw").value;

      var xmlhttp = new XMLHttpRequest();
      xmlhttp.open("POST", "/wifiConfig", true);
      xmlhttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

      var wifi_info = "ssid=" + encodeURIComponent(ssid) + "&password=" + encodeURIComponent(password);
      xmlhttp.send(wifi_info);

      alert("SSID: " + ssid + "\\nPassword: " + password);
    }

    function switch_password() {
      var pw_box = document.getElementById("pw");
      var switch_button = document.getElementById("switchpw");

      if (pw_box.type === "password") {
        pw_box.type = "text";
        switch_button.textContent = "Hide";
      } else {
        pw_box.type = "password";
        switch_button.textContent = "Show";
      }
    }
  </script>
</body>
</html>
)HTML";

void printWiFiInfo()
{
  Serial.printf("SSID: %s\r\n", WiFi.SSID().c_str());
  Serial.printf("Password: %s\r\n", WiFi.psk().c_str());
  WiFi.printDiag(Serial);
}

void wifiConfiguratorBegin(const char *ap_ssid, const char *ap_password)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);

  ssid_global = String(ap_ssid);
  password_global = String(ap_password);

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid_global.c_str(), password_global.c_str());

  Serial.println("AP Standby");
  Serial.printf("IP address: %s\n", WiFi.softAPIP().toString().c_str());

  if (MDNS.begin("esp8266"))
  {
    Serial.println("mDNS responder started at: esp8266.local");
  }
  else
  {
    Serial.println("Error setting up MDNS responder!");
  }

  setupWebServer();
}

void wifiConfiguratorLoop()
{
  if (!connected)
  {
    server.handleClient();
    MDNS.update();

    if (WiFi.status() == WL_CONNECTED)
    {
      connected = true;
      printWiFiInfo();
      server.stop();
      WiFi.softAPdisconnect(true);
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }
}

void tryReconnect()
{
  WiFi.persistent(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  Serial.print("Reconnect Wi-Fi Waiting......");
  for (int i = 0; i < 10; i++)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("Reconnect Success");
      printWiFiInfo();
      connected = true;
      digitalWrite(LED_BUILTIN, HIGH);
      return;
    }
    else
    {
      Serial.print("Try...");
      ledFlag = !ledFlag;
      digitalWrite(LED_BUILTIN, ledFlag ? HIGH : LOW);
      delay(500);
    }
  }
  Serial.println("Reconnect Failed");
  digitalWrite(LED_BUILTIN, LOW);
  connected = false;
}

void sendPage()
{
  server.send(200, "text/html", config_page);
}

void handleWiFiInfo()
{
  String wifi_ssid = server.arg("ssid");
  String wifi_pw = server.arg("password");
  Serial.println("WiFi Config Received:");
  Serial.println(wifi_ssid);
  Serial.println(wifi_pw);

  ssid_global = wifi_ssid;
  password_global = wifi_pw;

  WiFi.persistent(true);
  WiFi.begin(ssid_global.c_str(), password_global.c_str());

  server.send(200, "text/plain", "WiFi config received. Trying to connect...");
}

void handleNotFound()
{
  digitalWrite(LED_BUILTIN, LOW);
  String message = "ERROR\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  server.send(404, "text/plain", message);
  digitalWrite(LED_BUILTIN, HIGH);
}

void setupWebServer()
{
  server.on("/", sendPage);
  server.on("/wifiConfig", HTTP_POST, handleWiFiInfo);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

bool wifiConnect()
{
  return connected;
}
