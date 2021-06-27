#include <UIPEthernet.h>

#define BUFFER_SIZE 256
#define RELE_PIN_OUT A0

const byte mac[] = {0x90, 0xA2, 0xDA, 0x0D, 0x78, 0xEE};
IPAddress ip(192,168,1,140);
EthernetServer server(80);

char request_buffer[BUFFER_SIZE];
int buffer_index = 0;
int led_state = 0;

void parse_parameter()
{
    buffer_index = 0;
    Serial.println(request_buffer);
    
    while(buffer_index < BUFFER_SIZE) {
      char c = request_buffer[buffer_index];

      if (c == '=') {
        // request has GET parameters
        c = request_buffer[++buffer_index];
        if (c == '1') {
          // Turn on
          digitalWrite(RELE_PIN_OUT, HIGH);
          led_state = 1;
          Serial.println("ON");
        } else {
          // Turn off
          digitalWrite(RELE_PIN_OUT, LOW);
          led_state = 0;
          Serial.println("OFF");
        }
        break;
      }
      
      ++buffer_index;
    }

    // Clear the buffer
    buffer_index = 0;
    for(int i=0; i < BUFFER_SIZE; i++) {
      request_buffer[i] = 0;
    }
}

void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  server.begin();

  pinMode(RELE_PIN_OUT, OUTPUT);
  digitalWrite(RELE_PIN_OUT, LOW);

  Serial.print("IP Address: ");
  Serial.print(Ethernet.localIP());
}

void loop() {
  EthernetClient client = server.available();

  if (client) {
    Serial.println("-> New connection");

    bool currentLineIsBlank = true;
    bool firstLine = true;

    while(client.connected()) {
      if (client.available()) {
        char c = client.read();
        // Serial.print(c);

        if (firstLine) {
          request_buffer[buffer_index++] = c;
        }

        if (c == '\n' && currentLineIsBlank) {
          parse_parameter();

          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json");
          client.println("Connection: close");
          client.println();
          
          if (led_state) {
            client.println("{\"status\": 1}"); 
          } else {
            client.println("{\"status\": 0}"); 
          }
          break;
        }

        if (c == '\n') {
          currentLineIsBlank = true;
          firstLine = false;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }

    delay(10);
    client.stop();
    Serial.println("   Disconnected\n"); 
  }
}
