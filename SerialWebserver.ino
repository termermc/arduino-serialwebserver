#include <MemoryFree.h>

#define HTTP_MAX_HEADERS 6
#define BAUD 115200

String readString(bool skipReturn, bool doTrim) {
  String buf = "";
  auto done = false;
  
  while(!done) {
    while(Serial.available() > 0) {
      auto raw = Serial.read();
      auto ch = (char)raw;
      if(ch == '\n') {
        done = true;
        break;
      } else if(!skipReturn || ch != '\r') {
        buf+=ch;
      }
    }
  }
  
  if(doTrim)
    buf.trim();
  
  return buf;
}
String readString() {
  return readString(true, true);
}
int readInt() {
  while(Serial.available() < 1) {}
  return Serial.parseInt();
}

void sendHttpResponse(int status, String statusMsg, String contentType, String content) {
  String statusStr = F("HTTP/1.1 ");
  statusStr+=String(status);
  statusStr+=F(" ");
  statusStr+=statusMsg;
  Serial.println(statusStr);
  Serial.println(F("Connection: close"));
  String typeStr = F("Content-Type: ");
  typeStr+=contentType;
  Serial.println(typeStr);
  String lenStr = F("Content-Length: ");
  lenStr+=String(content.length());
  Serial.println(lenStr);
  Serial.println(F(""));
  Serial.print(content);
}

struct HttpHeader {
  String name;
  String value;
};
struct HttpRequest {
  String method;
  String path;
  unsigned char headersCount;
  struct HttpHeader headers[HTTP_MAX_HEADERS];
  String body;
};

// Returns the value of the header with the provided name, or an empty String if it cannot be found
String getHeader(struct HttpRequest *req, String name) {
  auto key = String(name);
  key.toLowerCase();
  
  for(unsigned char i = 0; i < (*req).headersCount; i++) {
    auto header = (*req).headers[i];
    
    if(header.name == key)
      return header.value;
  }

  return "";
}

struct HttpRequest parseHttpRequest() {
  // Read and parse first line
  auto ln = readString();
  struct HttpRequest req;
 
  auto mtdIdx = ln.indexOf(" ");
  req.method = ln.substring(0, mtdIdx);
  ln = ln.substring(mtdIdx+1);
  req.path = ln.substring(0, ln.indexOf(" "));

  // Parse headers
  req.headersCount = 0;
  String lastLn = ".";
  while(true) {
    auto headerLn = readString();

    // End request if blank
    if(headerLn == "") {
      break;
    } else if(req.headersCount < HTTP_MAX_HEADERS) {
      // Parse header
      struct HttpHeader header;
      auto idx = headerLn.indexOf(":");
      header.name = headerLn.substring(0, idx);
      header.name.toLowerCase();
      header.name.trim();
      header.value = headerLn.substring(idx+1);
      header.value.trim();

      // Put header
      req.headers[req.headersCount] = header;

      req.headersCount++;
    }
  }

  return req;
}

void setup() {
  Serial.begin(BAUD);
}

// Count page hits
int pageHits = 0;

void loop() {
  // Get HTTP request
  struct HttpRequest req = parseHttpRequest();

  // Pages
  if(req.path == "/") {
    String res = F("<title>SerialWebserver Home</title><h1>Welcome</h1><p>You've found yourself at the SerialWebserver homepage.</p><p>This website is hosted on an <a href=\"https://www.arduino.cc/en/Main/arduinoBoardUno&amp;gt;\">Arduino Uno</a> microcontroller.</p><p><a href=\"/memory\">Memory info</a> - <a href=\"/headers\">HTTP headers</a></p><p>Page hits: ");
    res+=String(pageHits);
    res+=F("</p>");
    sendHttpResponse(
      200, F("OK"),
      F("text/html"),
      res
    );
  } else if(req.path == "/memory") {
    String res = F("<title>Memory Info</title><h1>Memory Info</h1><p>The server currently has ");
    res+=String(freeMemory());
    res+=F(" bytes of memory free. There may be more used on more content-heavy pages.</p><p><a href=\"/\">Back</a></p>");
    sendHttpResponse(
      200, F("OK"),
      F("text/html"),
      res
    );
  } else if(req.path == "/headers") {
    String res = F("<title>HTTP headers</title><h1>HTTP headers</h1><p>Received the following headers (capped at ");
    res+=String(HTTP_MAX_HEADERS);
    res+=F(" headers):</p><pre>");
    for(unsigned char i = 0; i < req.headersCount; i++) {
      res+=req.headers[i].name;
      res+=F(": ");
      res+=req.headers[i].value;
      res+=F("\n");
    }
    res+=F("</pre><p>The more headers that are read, the more memory is consumed per-request, which means less memory for page rendering and processing. (");
    res+=String(freeMemory());
    res+=F(" bytes free right now)</p><p><a href=\"/\">Back</a></p>");
    sendHttpResponse(
      200, F("OK"),
      F("text/html"),
      res
    );
  } else {
    String res = F("<title>Not Found</title><h1>Not Found</h1><p>The page \"");
    res+=req.path;
    res+=F("\" does not exist.</p>");
    sendHttpResponse(
      404, F("Not Found"),
      F("text/html"),
      res
    );
  }

  pageHits++;
}
