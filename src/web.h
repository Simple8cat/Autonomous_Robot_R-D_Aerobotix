#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// ======================== WIFI ACCESS POINT ========================
#define AP_SSID     "ESP32-AP"
#define AP_PASSWORD "12345678"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

unsigned long lastSend = 0;
unsigned long sendInterval = 100; // send encoder & odometry every 100 ms

// ======================== WEBSOCKET HANDLERS ========================
void handle_received_msg(String message) {
  Serial.println("Received: " + message);
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("Client #%u connected\n", client->id());
      break;

    case WS_EVT_DISCONNECT:
      Serial.printf("Client #%u disconnected\n", client->id());
      break;

    case WS_EVT_DATA: {
      AwsFrameInfo *info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->opcode == WS_TEXT) {
        data[len] = 0;
        handle_received_msg((char*)data);
      }
    } break;

    default:
      break;
  }
}

// Send message to all websocket clients
void send_msg(String msg) {
  ws.textAll(msg);
}

// ======================== SETUP ========================
void setup() {
  Serial.begin(115200);

  // ---- WIFI ACCESS POINT ----
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  Serial.println("AP Started.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // ---- WEBSOCKET ----
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  server.begin();
  Serial.println("WebSocket server ready!");
}

// ======================== LOOP ========================
void loop() {
  ws.cleanupClients();
  // Send encoder counts + odometry every 100ms
  if (millis() - lastSend >= sendInterval) {
    lastSend = millis();

    String json = "{";
    json += "\"E1\":" + String(delta_left) + ",";
    json += "\"E2\":" + String(delta_right) + ",";
    json += "\"x\":"  + String(current_pose.x, 4) + ",";
    json += "\"y\":"  + String(current_pose.y, 4) + ",";
    json += "\"theta\":" + String(current_pose.theta, 4);
    json += "}";

    send_msg(json);
  }
}