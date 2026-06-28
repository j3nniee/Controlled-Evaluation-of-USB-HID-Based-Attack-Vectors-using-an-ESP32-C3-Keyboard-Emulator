
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED CONFIG Details
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDR     0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Uses LED_BUILTIN if defined; otherwise falls back to GPIO 2 (common blue LED)
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif
const int LED_PIN = LED_BUILTIN;

// AP CONFIG 
const char* AP_SSID = "ESP32-LED-AP";
const char* AP_PASSWORD = "esp32led123"; // >= 8 chars (WPA2); set your own

// Web Server Config
WebServer server(80);
volatile bool ledOn = false;

// oled helpers part
void oledMultiline(const String &a, const String &b = "", const String &c = "", const String &d = "") {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  if (a.length()) display.println(a);
  if (b.length()) display.println(b);
  if (c.length()) display.println(c);
  if (d.length()) display.println(d);
  display.display();
}

void showStatus() {
  IPAddress ip = WiFi.softAPIP();
  oledMultiline(
    "AP Running",
    String("SSID: ") + AP_SSID,
    String("IP: ") + ip.toString(),
    String("LED: ") + (ledOn ? "ON" : "OFF")
  );
}

// LED control part
void setLED(bool on) {
  ledOn = on;
  digitalWrite(LED_PIN, on ? HIGH : LOW);
  showStatus();
}

// The nice little webapp part
// HTML page stored in PROGMEM to save RAM
// (C++11 raw string literal for convenience LOL)
const char INDEX_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="en">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP32 LED Control</title>
<style>
  body { font-family: system-ui, Arial, sans-serif; margin: 24px; }
  .card { max-width: 420px; margin:auto; padding:20px; border:1px solid #ccc; border-radius:12px; }
  h1 { font-size: 1.25rem; margin:0 0 12px; }
  .status { margin: 12px 0; font-weight: 600; }
  button { padding: 10px 16px; margin-right: 8px; border-radius: 8px; border: 1px solid #aaa; cursor: pointer; }
</style>
<div class="card">
  <h1>Welcome to this neat webapp I built LOL</h1>
  <h1>ESP32 LED Control</h1>
  <div class="status">Status: <span id="s">...</span></div>
  <button onclick="setLed('on')">Turn ON</button>
  <button onclick="setLed('off')">Turn OFF</button>
</div>
<script>
async function refresh() {
  const r = await fetch('/api/status'); 
  const j = await r.json();
  document.getElementById('s').textContent = j.led ? 'ON' : 'OFF';
}
async function setLed(state) {
  await fetch('/api/led?state=' + state, {method:'POST'});
  refresh();
}
refresh();
</script>
</html>
)HTML";

void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleStatus() {
  String json = String("{\"led\":") + (ledOn ? "true" : "false") + "}";
  server.send(200, "application/json", json);
}

void handleLed() {
  String state = server.arg("state");
  if (state == "on") {
    setLED(true);
    server.send(200, "application/json", "{\"ok\":true,\"led\":true}");
  } else if (state == "off") {
    setLED(false);
    server.send(200, "application/json", "{\"ok\":true,\"led\":false}");
  } else {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"state must be on/off\"}");
  }
}

void setup() {
  // Serial optional
  Serial.begin(115200);
  delay(200);

  // OLED init
  Wire.begin(21, 22); // SDA, SCL
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    // If OLED fails, continue anyway
  } else {
    oledMultiline("Booting...", "Init OLED");
  }

  // LED init
  pinMode(LED_PIN, OUTPUT);
  setLED(false);  // default OFF

  // Wi-Fi AP
  WiFi.mode(WIFI_MODE_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  delay(200);

  // Web routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/led", HTTP_POST, handleLed);
  server.begin();

  showStatus();
}

void loop() {
  server.handleClient();
}
