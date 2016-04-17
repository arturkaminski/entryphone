#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

const char* ssid = "_HackWimbledon";
const char* password = "<Hack@Compton>";
//IPAddress ip(192,168,0,100);
//IPAddress gateway(192,168,0,1);
//IPAddress subnet(255,255,255,0);
ESP8266WebServer server(80);

//Check if header is present and correct
bool is_authentified() {
  Serial.println("Enter is_authentified");
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
      Serial.println("Authentification Successful");
      return true;
    }
  }
  Serial.println("Authentification Failed");
  return false;
}

//login page, also called for disconnect
void handleLogin() {
  String msg;
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
  }
  if (server.hasArg("DISCONNECT")) {
    Serial.println("Disconnection");
    String header = "HTTP/1.1 301 OK\r\nSet-Cookie: ESPSESSIONID=0\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    pinMode(2, OUTPUT);
    digitalWrite(2, 1);
    pinMode(4, OUTPUT);
    digitalWrite(4, 1);
    return;
  }
  if (server.hasArg("USERNAME") && server.hasArg("PASSWORD")) {
    if (server.arg("USERNAME") == "admin" &&  server.arg("PASSWORD") == "wimbledon" ) {
      String header = "HTTP/1.1 301 OK\r\nSet-Cookie: ESPSESSIONID=1\r\nLocation: /\r\nCache-Control: no-cache\r\n\r\n";
      server.sendContent(header);
      Serial.println("Log in Successful");
      pinMode(2, OUTPUT);
      digitalWrite(2, 0);
      pinMode(4, OUTPUT);
      digitalWrite(4, 0);
    
      return;
    }
    msg = "Wrong username/password! try again.";
    Serial.println("Log in Failed");
  }
  String content = "<html><title>Door Entry System</title><body><form action='/login' method='POST'>To open door please log in<br>";
  content += "<H1 style='position: absolute; top:80px; left:200px; width:240px; height:100px'> User: <input type='text' name='USERNAME' placeholder='user name'></H1><br>";
  content += "<H1 style='position: absolute; top:180px; left:200px; width:240px; height:100px'> Password:<input type='password' name='PASSWORD' placeholder='password'></H1><br>";
  content += "<p style='position: absolute; top:280px; left:200px; width:400px; height:100px '> <input type='submit' style='background-color:blue' name='SUBMIT' value='Submit'></form>" + msg + "</p><br>";
  content += "<p style='position: absolute; top:80px; left:500px'> Camera <img src='http://192.168.1.129:8081' alt='http://192.168.1.129:8081/'></img></p></body></html>";
  server.send(200, "text/html", content);
}

//root page can be accessed only if authentification is ok
void handleRoot() {
  Serial.println("Enter handleRoot");
  String header;
  if (!is_authentified()) 
  {
    String header = "HTTP/1.1 301 OK\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
  }
  else
  {
    String content = "<html><head><title>Door Entry System</title><META http-equiv='refresh' content='5;URL=/login?DISCONNECT=YES'></head><body onload='secs()'><H2>Door Open</H2><br>";

    /*if (server.hasHeader("User-Agent")){
      content += "the user agent used is : " + server.header("User-Agent") + "<br><br>";
    }
    */
    content += "Door will be close in <p id='txt' style='display:inline'>5</p><script>function secs(){var i=6; setInterval(function(){i--;txt.innerHTML = i}, 1200)}</script> second </body></html>";
    
    server.send(200, "text/html", content);
  }
}

//no need authentification
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void) {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  // WiFi.config(ip, gateway, subnet);
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
  server.on("/login", handleLogin);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works without need of authentification");
  });

  server.onNotFound(handleNotFound);
  //here the list of headers to be recorded
  const char * headerkeys[] = {"User-Agent", "Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
  //ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize );
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}
