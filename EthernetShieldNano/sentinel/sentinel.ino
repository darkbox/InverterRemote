#include <UIPEthernet.h>

#define BUFFER_SIZE 128
#define BUTTON_PIN PIN_A0
#define LED_STATUS_PIN PIN_A1
#define REFRESH_INTERVAL_MS 2000
#define WORKER_IP "192.168.1.140"

const byte mac[] = {0x90, 0xA2, 0xDA, 0x0D, 0x78, 0xEF};
IPAddress ip(192,168,1,141);
EthernetClient client;

int lastWorkerState = 0;
unsigned int bufferIndex = 0;
char responseBuffer[BUFFER_SIZE];
char requestBuffer[50];
static unsigned long lastRefreshTime = 0;

/**
 * Sends a http resquest to the worker arduino.
 * 
 * @param status -1 check status; 0 off; 1 on;
 */
void sendRequest(int status)
{
  if (client.connect(WORKER_IP, 80)) {
    if (status < 0) {
      client.println("GET / HTTP/1.1");
    } else {
      sprintf(requestBuffer, "GET /?s=%d HTTP/1.1", status);
      client.println(requestBuffer);
    }
    sprintf(requestBuffer, "Host: %s", WORKER_IP);
    client.println(requestBuffer);
    client.println();
    Serial.println(requestBuffer);
  }
}

/**
 * Send a http request and check the status 
 * of the worker every X time periodicaly.
 */
void checkWorker()
{
  if (millis() - lastRefreshTime >= REFRESH_INTERVAL_MS) {
    lastRefreshTime += REFRESH_INTERVAL_MS;
    sendRequest(-1);
    //Serial.println("Probing...");
  }
}

/**
 * Checks if the physical button has been press.
 * If so, send a http request to the worker
 */
void checkButton()
{
  if (digitalRead(BUTTON_PIN)) {
    Serial.println("Sending on...");
    sendRequest(1);
    delay(1000);
    Serial.println("Sending off...");
    sendRequest(0);
    delay(500);
  }
}

/**
 * Update the local led indicator to show the current
 * state of the arduino worker.
 */
void updateLedIndicator()
{
  if (lastWorkerState) {
    digitalWrite(LED_STATUS_PIN, HIGH);
  } else {
    digitalWrite(LED_STATUS_PIN, LOW);
  }
}

/**
 * Find string in another string
 * 
 * @param s     the string to search for
 * @param sSize the size of the string
 * @param t     the text
 * @param tSize the size of the text
 * @return      the position found or -1 if not found
 */
int findString(char* s, int sSize, char* t, int tSize)
{
  int posSearch = 0;
  int posText = 0;
  sSize -= 1;
  tSize -= 1;

  for (;posText < tSize - sSize; ++posText) {
    if (t[posText] == s[posSearch]) {
      ++posSearch;
      if (posSearch == sSize) {
        // Found!
        return posText - sSize;
      }
    } else {
      posText -= posSearch;
      posSearch = 0;
    }
  }
  
  return -1; // Not found
}

/**
 * Checks the worker response and parse the body.
 */
void checkResponse()
{
  // Reading
  bufferIndex = 0;
  while(client.connected()) {
    if (client.available()) {
      responseBuffer[bufferIndex++] = client.read();
    }
  }

  // Parse JSON response for status
  bufferIndex = 0;
  int bodyFound = 0;
  while(bufferIndex < BUFFER_SIZE) {
    char c = responseBuffer[bufferIndex++];
  }

  int posFound = findString("status", 6, responseBuffer, BUFFER_SIZE);

  if (posFound > -1 && posFound < BUFFER_SIZE) {
    int valuePos = posFound + 10;
    lastWorkerState = responseBuffer[valuePos] - '0';
    sprintf(requestBuffer, "Status: %d at %d with %c", lastWorkerState, valuePos, responseBuffer[valuePos]);
    Serial.println(requestBuffer);
  }
  
  //Serial.println(responseBuffer);

  // Clear the buffer
  memset(responseBuffer, 0, BUFFER_SIZE * sizeof(*responseBuffer));
  bufferIndex = 0;
}

void setup()
{
  pinMode(BUTTON_PIN, INPUT);
  pinMode(PIN_A1, OUTPUT);
  
  Serial.begin(9600);
  Ethernet.begin(mac, ip);

  Serial.print("IP Address: ");
  Serial.println(Ethernet.localIP());

  sendRequest(-1);
}

void loop()
{
  checkButton();
  checkResponse();
  updateLedIndicator();
  checkWorker();
}
