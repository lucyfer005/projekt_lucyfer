#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "FS.h"

const byte DNS_PORT = 53;
IPAddress apIP(8, 8, 8, 8);
DNSServer dnsServer;
ESP8266WebServer webServer(80);


void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println("Configuring access point...");
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("KOSCIOL_SZATANA");

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);


  MDNS.begin("lucyfer");
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
  Serial.println("mDNS responder started");

  SPIFFS.begin(); 
  cleanLogToFile();

 webServer.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(webServer.uri()))                  // send it if it exists
      handleFileRead("/index.html");
  });
  

  webServer.begin();                           // Actually start the server
  Serial.println("HTTP server started");
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Get the MIME type
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = webServer.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    logToFile("handleFileRead: " + path);
    return true;
  }
  logToFile("File Not Found: " + path);
  return false;                                         // If the file doesn't exist, return false
}

void logToFile(String value)
{   
  File file = SPIFFS.open("/log.txt", "a");
  String logString = value;
  Serial.println(logString);
  file.println(logString);
  file.close();
}


void cleanLogToFile()
{   
  File file = SPIFFS.open("/log.txt", "w");
  file.println("");
  file.close();
}
