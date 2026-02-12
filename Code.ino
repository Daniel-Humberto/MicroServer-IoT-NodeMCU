#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>


// ========== CONFIG WIFI ==========
const char* ssid = "LOREM IPSUM";
const char* password = "123456789";




// ===== IP ESTÁTICA 192.168.1.250 =====
IPAddress local_IP(192, 168, 1, 250);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns1(8, 8, 8, 8);
IPAddress dns2(8, 8, 4, 4);




// ========== SENSORES ==========
#define DHTPIN D2        // GPIO4
#define DHTTYPE DHT11
#define PIRPIN D5        // GPIO14


const unsigned long DHT_INTERVAL = 2000; // ms
const unsigned long PIR_INTERVAL = 200;  // ms




// ========== GLOBALS ==========
ESP8266WebServer server(80);
DHT dht(DHTPIN, DHTTYPE);


float lastTemp = NAN;
float lastHum  = NAN;
bool lastMotion = false;


unsigned long lastDHTread = 0;
unsigned long lastPIRread = 0;


// Reconexión WiFi
unsigned long lastWiFiAttempt = 0;




// ========== SETUP ==========
void setup() {

  Serial.begin(115200);
  delay(10);
  pinMode(PIRPIN, INPUT);
  dht.begin();

  WiFi.mode(WIFI_STA);

  // Intentar aplicar IP estática antes de conectar
  if (!WiFi.config(local_IP, gateway, subnet, dns1, dns2)) {
    Serial.println("Warning: fallo en WiFi.config(); puede que se use DHCP.");
  } else {
    Serial.print("IP estática configurada: ");
    Serial.println(local_IP);
  }

  Serial.print("Conectando a WiFi");
  WiFi.begin(ssid, password);

  unsigned long start = millis();
  // Espera hasta 20s como timeout inicial
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("WiFi conectado. IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("No se pudo conectar a WiFi (timeout). Seguirá intentando en loop.");
  }

  server.on("/", handleRoot);
  server.on("/status", HTTP_GET, handleStatus);
  server.begin();
  Serial.println("HTTP server iniciado");
}




// ========== LOOP ==========
void loop() {

  unsigned long now = millis();

  // Reintento simple de conexión WiFi cada 10s si se desconectó
  if (WiFi.status() != WL_CONNECTED) {
    if (now - lastWiFiAttempt > 10000) {
      lastWiFiAttempt = now;
      Serial.println("WiFi desconectado, intentando reconectar...");
      WiFi.disconnect();
      WiFi.begin(ssid, password);
    }
  }

  // Leer DHT cada DHT_INTERVAL
  if (now - lastDHTread >= DHT_INTERVAL) {
    lastDHTread = now;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (!isnan(h) && !isnan(t)) {
      lastHum = h;
      lastTemp = t;
    } else {
      Serial.println("Error lectura DHT");
    }
  }

  // Leer PIR
  if (now - lastPIRread >= PIR_INTERVAL) {
    lastPIRread = now;
    lastMotion = digitalRead(PIRPIN);
  }

  server.handleClient();
}




// ===== HANDLERS =====
void handleRoot() {
  String html = R"rawliteral(
<!doctype html>
<html>


  <head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>ESP8266 • Live Monitor</title>
  <style>
    :root{--bg:#0f1720;--card:#0b1220;--accent:#00d084;--muted:#98a0a8;}
    html,body{height:100%;margin:0;font-family:Inter,Segoe UI,Arial;background:linear-gradient(180deg,#071021 0%,#071a2a 100%);color:#e6eef6}
    .wrap{max-width:920px;margin:36px auto;padding:20px}
    header{display:flex;align-items:center;justify-content:space-between;margin-bottom:18px}
    h1{font-size:20px;margin:0;color:var(--accent)}
    .sub{color:var(--muted);font-size:13px}
    .grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(240px,1fr));gap:18px}
    .card{background:rgba(255,255,255,0.03);border:1px solid rgba(255,255,255,0.04);padding:18px;border-radius:12px;backdrop-filter: blur(6px)}
    .k{font-size:14px;color:var(--muted);margin-bottom:6px}
    .v{font-size:28px;font-weight:700;color:#fff}
    canvas{width:100%;height:80px;background:transparent;display:block;margin-top:10px}
    footer{margin-top:18px;color:var(--muted);font-size:13px;text-align:center}
  </style>
  </head>


  <body>


  <div class="wrap">

    <header>
      <div>
        <h1>ESP8266 • Live Monitor</h1>
        <div class="sub">Temperatura, Humedad y Movimiento • Actualización en vivo</div>
      </div>
      <div id="ip" class="sub"></div>
    </header>

    <div class="grid">
      <div class="card">
        <div class="k">Temperatura</div>
        <div class="v" id="temp">-- °C</div>
        <canvas id="chartTemp"></canvas>
      </div>

      <div class="card">
        <div class="k">Humedad</div>
        <div class="v" id="hum">-- %</div>
        <canvas id="chartHum"></canvas>
      </div>

      <div class="card">
        <div style="display:flex;align-items:center;justify-content:space-between">
          <div>
            <div class="k">Movimiento</div>
            <div class="v" id="motion">--</div>
          </div>
          <div style="text-align:right">
            <div style="font-size:12px;color:var(--muted)">Última detección:</div>
            <div id="lastSeen" style="font-size:13px;color:#dceef3">--</div>
          </div>
        </div>
      </div>

    </div>


    <footer>Actualización cada 1s — Sensor DHT11 limitado a cada 2s</footer>


  </div>


  <script>


  function drawLine(canvas, data) {}
    const ctx = canvas.getContext('2d');
    const w = canvas.clientWidth;
    const h = canvas.clientHeight;
    canvas.width = w;
    canvas.height = h;
    ctx.clearRect(0,0,w,h);
    ctx.lineWidth = 2;
    ctx.strokeStyle = 'rgba(0,208,132,0.95)';
    if (data.length < 2) return;
    const max = Math.max(...data);
    const min = Math.min(...data);
    const range = (max - min) || 1;
    ctx.beginPath();
    for (let i=0;i<data.length;i++){
      const x = (i/(data.length-1)) * w;
      const y = h - ((data[i]-min)/range) * h;
      if (i==0) ctx.moveTo(x,y); else ctx.lineTo(x,y);
    }
    ctx.stroke();
  }

  let tempHistory = [], humHistory = [];
  const MAX_POINTS = 40;
  let lastMotionTs = null;

  function updateUI(d){
    const tEl = document.getElementById('temp');
    const hEl = document.getElementById('hum');
    const mEl = document.getElementById('motion');
    const lastEl = document.getElementById('lastSeen');

    if (d.temp !== null) {
      tEl.innerText = d.temp.toFixed(1) + ' °C';
      tempHistory.push(d.temp);
      if (tempHistory.length>MAX_POINTS) tempHistory.shift();
    }
    if (d.hum !== null) {
      hEl.innerText = d.hum.toFixed(1) + ' %';
      humHistory.push(d.hum);
      if (humHistory.length>MAX_POINTS) humHistory.shift();
    }
    if (d.motion) {
      mEl.innerText = 'DETECTADO';
      mEl.style.color = '#ff6b6b';
      lastMotionTs = Date.now();
    } else {
      mEl.innerText = 'No';
      mEl.style.color = '#9be7c4';
    }
    if (lastMotionTs) {
      const s = Math.floor((Date.now()-lastMotionTs)/1000);
      lastEl.innerText = s + 's atrás';
    } else {
      lastEl.innerText = '--';
    }

    drawLine(document.getElementById('chartTemp'), tempHistory);
    drawLine(document.getElementById('chartHum'), humHistory);
  }

  // fetch loop
  async function fetchLoop(){
    try {
      const resp = await fetch('/status', {cache:'no-cache'});
      if (!resp.ok) throw 'err';
      const d = await resp.json();
      updateUI(d);
    } catch(e){
      console.log('fetch error', e);
    } finally {
      setTimeout(fetchLoop, 1000);
    }
  }

  // show IP if present
  window.addEventListener('load', ()=>{
    const ip = location.hostname;
    document.getElementById('ip').innerText = ip;
    fetchLoop();
  });


  </script>


  </body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}




void handleStatus() {
  // Enviar JSON con los últimos valores leídos
  String json = "{";
  if (isnan(lastTemp)) json += "\"temp\":null,"; else json += "\"temp\":" + String(lastTemp,1) + ",";
  if (isnan(lastHum))  json += "\"hum\":null,";  else json += "\"hum\":"  + String(lastHum,1)  + ",";
  json += "\"motion\":" + String(lastMotion ? 1 : 0);
  json += "}";
  server.send(200, "application/json", json);
}