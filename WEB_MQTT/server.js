const express = require("express");
const mqtt = require("mqtt");

const app = express();

// Nếu bạn dùng thư mục public1 thì đổi thành "public1"
app.use(express.static("public"));

// ===== MQTT LOCAL BROKER (đúng theo ESP32 của bạn) =====
const MQTT_URL = "mqtt://192.168.137.1:1883";
const TOPIC = "vku/khoa/sensors/health";

let latest = { bpm: 0, spo2: 0, temp: 0, ts: 0 };
let lastMqttSeen = 0;

const client = mqtt.connect(MQTT_URL, {
  // không bắt buộc, nhưng giúp ổn định
  reconnectPeriod: 1000,
  connectTimeout: 5000,
});

client.on("connect", () => {
  console.log("MQTT connected:", MQTT_URL);
  client.subscribe(TOPIC, (err) => {
    if (err) console.log("MQTT subscribe error:", err);
    else console.log("MQTT subscribed:", TOPIC);
  });
});

client.on("reconnect", () => console.log("MQTT reconnecting..."));
client.on("error", (e) => console.log("MQTT error:", String(e)));

client.on("message", (topic, message) => {
  try {
    const s = message.toString();
    const json = JSON.parse(s);

    latest = {
      bpm: Number(json.bpm) || 0,
      spo2: Number(json.spo2) || 0,
      temp: Number(json.temp) || 0,
      ts: Number(json.ts) || Date.now(),
    };
    lastMqttSeen = Date.now();
    // debug
    console.log("MQTT recv:", latest);
  } catch (e) {
    console.log("MQTT parse error:", String(e));
  }
});

// API cho web
app.get("/data", (req, res) => {
  res.json({
    ...latest,
    mqtt_alive: Date.now() - lastMqttSeen < 5000, // thấy dữ liệu trong 5s gần nhất
  });
});

const PORT = 3000;
app.listen(PORT, () => console.log("Web server: http://localhost:" + PORT));
