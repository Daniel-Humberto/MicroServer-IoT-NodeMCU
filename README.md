<div align="center">


---


![Banner](https://capsule-render.vercel.app/api?type=waving&color=0:020617,50:0a192f,100:020617&height=180&section=header&text=MicroServer%20IoT%20NodeMCU&fontSize=34&fontColor=60a5fa&animation=fadeIn&fontAlignY=38&desc=NodeMCU%20%7C%20WebServer%20%7C%20IoT%20Sensor&descAlignY=60&descAlign=50)


</div>


---


## 🌡️ Introducción


Microservidor IoT basado en un NodeMCU que integra un sensor DHT (temperatura y humedad) y un sensor PIR de movimiento.
Recolecta y actualiza en tiempo real datos de temperatura, humedad, y detección de movimiento.

Mediante un Dashboard Web embebido se pueden consultar los datos en tiempo real.
El Dashboard Web es accesible desde cualquier dispositivo en la red local.


---


## 📋 Características

Este proyecto convierte un NodeMCU ESP8266 en un pequeño servidor web embebido que:

- Lee temperatura y humedad con un sensor **DHT11**
- Detecta movimiento con un sensor **PIR (HC-SR501)**
- Despliega un **Dashboard Web** con actualización cada 1 segundo
- Expone un endpoint **`/status`** en formato JSON para integrarse con otros sistemas IoT
- Mantiene una **IP estática** (192.168.1.250) en la red local
- Reconecta automáticamente al WiFi si pierde la conexión


---


<div align="center">


## 🛠️ Hardware requerido

| Componente | Descripción |
|---|---|
| NodeMCU ESP8266 | Microcontrolador con WiFi integrado |
| DHT11 | Sensor de temperatura y humedad |
| PIR HC-SR505 | Sensor infrarrojo pasivo de movimiento |
| Resistencia 10kΩ | Pull-up para el pin de datos del DHT11 |
| Protoboard + cables | Conexiones |


---


## 🔌 Conexiones (Pinout)

| Sensor | Pin del sensor | Pin NodeMCU |
|---|---|---|
| DHT11 — Data | DATA | D2 (GPIO4) |
| DHT11 — VCC | + | 3.3V o 5V |
| DHT11 — GND | - | GND |
| PIR HC-SR505 — Signal | OUT | D5 (GPIO14) |
| PIR HC-SR505 — VCC | + | 5V |
| PIR HC-SR505 — GND | - | GND |

> ⚠️ El sensor PIR HC-SR505 requiere 5V para funcionar correctamente. Conectarlo al pin **VIN** del NodeMCU cuando esté alimentado por USB.


</div>


---


## 📦 Dependencias (librerías Arduino)

Instalar desde el **Library Manager** de Arduino IDE:

- `ESP8266WiFi` — incluida en el core de ESP8266
- `ESP8266WebServer` — incluida en el core de ESP8266
- `DHT sensor library` — de **Adafruit**
- `Adafruit Unified Sensor` — dependencia de la anterior


---


## ⚙️ Configuración

Antes de compilar, editar las siguientes líneas en el archivo `.ino`:

```cpp
// Credenciales WiFi
const char* ssid     = "TU_RED_WIFI";
const char* password = "TU_CONTRASEÑA";

// IP estática (ajustar según tu red)
IPAddress local_IP(192, 168, 1, 250);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
```


---


## 🚀 Uso

1. Conectar el hardware según el pinout.
2. Configurar credenciales WiFi en el código.
3. Compilar y subir al NodeMCU desde Arduino IDE.
4. Abrir el **Monitor Serie** (115200 baudios) para confirmar la IP asignada.
5. Navegar en el navegador a la ip seleccionada.

---


<div align="center">

## 🌐 Endpoints del servidor web

| Ruta | Método | Descripción |
|---|---|---|
| `/` | GET | Dashboard web con gráficas en vivo |
| `/status` | GET | Datos en formato JSON |

</div>


### Ejemplo de respuesta `/status`

```json
{
  "temp": 24.5,
  "hum": 58.0,
  "motion": 0
}
```


<div align="center">

| Campo | Tipo | Descripción |
|---|---|---|
| `temp` | `float` / `null` | Temperatura en °C. `null` si hay error de lectura |
| `hum` | `float` / `null` | Humedad relativa en %. `null` si hay error de lectura |
| `motion` | `0` / `1` | `1` = movimiento detectado, `0` = sin movimiento |

</div>


---


## 📊 Dashboard web

El dashboard muestra:

- **Temperatura** en °C con gráfica histórica de los últimos 40 puntos
- **Humedad** en % con gráfica histórica de los últimos 40 puntos
- **Estado de movimiento** con indicador de tiempo desde la última detección
- Se actualiza automáticamente cada **1 segundo** vía fetch al endpoint `/status`

> 📌 El sensor DHT11 se lee cada **2 segundos** por limitación del sensor. La UI se actualiza igual cada 1s mostrando el último valor válido.


---


## 📝 Licencia

Este proyecto está licenciado bajo la [Licencia GNU](LICENSE).


---


<p align="center">
  <sub>© 2026 Ing. Daniel Humberto Reyes Rocha.</sub>
</p>


---
