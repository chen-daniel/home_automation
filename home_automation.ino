#include <RCSwitch.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

// Array of ON/OFF codes. You must replace these with your codes obtained using a sniffer. This matches the 5 outlet remote.
unsigned long rc_codes[10][2] = {
	// ON     //OFF 
	{, }, /* Outlet 1 */
	{, }, /* Outlet 2 */
	{, }, /* Outlet 3 */
	{, }, /* Outlet 4 */
	{, }, /* Outlet 5 */
  {, }, /* Outlet 6 */
  {, }, /* Outlet 7 */
  {, }, /* Outlet 8 */
  {, }, /* Outlet 9 */
  {, }, /* Outlet 10 */
};
// The physical Arduino PIN (this will be called with pinMode()).
#define RC_PIN_TX 0
#define RC_PROTOCOL 1
#define RC_PULSE_LENGTH 190 // 'Delay'
#define RC_BIT_LENGTH 24

// Wifi Constants
const char* ssid = "";
const char* password = "";
ESP8266WebServer server(301);

RCSwitch sendSwitch = RCSwitch();

// Handler for root endpoint
void handleRoot() {
  String message = "419";
  server.send(200, "text/plain", message);
}

// Handler for lights endpoint
void handleLights() {
  int totalArgs = server.args();
  String message = "Number of args received: ";
  message += totalArgs;
  message += "\n"; 
  for (int i = 0; i < totalArgs; i++) {
    message += "Arg " + (String)i + " -> ";
    message += server.argName(i) + ": ";
    message += server.arg(i) + "\n";
  }
  server.send(200, "text/plain", message);
  for (int j = 0; j < 5; j++) {
    for (int i = 0; i < totalArgs; i++) {
      int lightNum = server.argName(i).toInt();
      int command = server.arg(i).toInt();
      bool com;
      if (command == 1) {
        com = true;
      }
      else {
        com = false;
      }
      enableOutlet(lightNum, com);
      delay(50);
    }
  }
}

// Handle page not found
void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

// Set up server
void setup()
{
  Serial.begin(9600);
  sendSwitch.enableTransmit(RC_PIN_TX);
  sendSwitch.setProtocol(RC_PROTOCOL); // defaults to 1 anyway
  sendSwitch.setPulseLength(RC_PULSE_LENGTH); // this is critical
  WiFi.begin(ssid, password);
  Serial.println("");
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/lights", handleLights);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

// Outlet control function
void enableOutlet(int outletNumber, bool onOrOff)
{
  if (outletNumber < 1 || outletNumber > 10)
  {
    Serial.println("Invalid outlet number");
    return;
  }
  unsigned long *onOffCodes = rc_codes[outletNumber - 1];
  unsigned long codeToSend = onOffCodes[onOrOff ? 0 : 1];
  sendSwitch.send(codeToSend, RC_BIT_LENGTH);
	
  char outletNumberString[1];
  int retVal = snprintf(outletNumberString, 1, "%d", outletNumber);
  if (retVal < 0)
  {
    Serial.println("Log encoding error");
    return;
  }
	
  if (onOrOff)
  {
    Serial.print("Enabling");
  }
  else
  {
    Serial.print("Disabling");
  }
	
  Serial.print(" outlet ");
  Serial.println(outletNumberString);
}

// Maintain server
void loop()
{
  server.handleClient();
}
