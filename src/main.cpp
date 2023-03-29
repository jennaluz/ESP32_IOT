#include <WiFi.h>
#include <Arduino.h>


// network credentials
const char *ssid = "redacted";
const char *password = "redacted";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

unsigned long current_time = millis(); // current time
unsigned long previous_time = 0;       // previous time
const long timeout_time = 2000;        // define timeout time in milliseconds (example: 2000ms = 2s)


void ui_handler(void *parameters);


void setup() {
    Serial.begin(115200);

    // connect to Wi-Fi network with SSID and password
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    // print local IP address and start web server
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    server.begin();

    xTaskCreate(ui_handler, "User Interface", 2048, nullptr, 4, nullptr);
}


void loop() {}


void ui_handler(void *parameters) {
    while (true) {
        WiFiClient client = server.available();   // listen for incoming clients

        // new client connects
        if (client) {
            current_time = millis();
            previous_time = current_time;
            Serial.println("New Client.");
            String current_line = "";

            // loop while the client is connected
            while (client.connected() && current_time - previous_time <= timeout_time) {
                current_time = millis();

                if (client.available()) {
                    char c = client.read();
                    Serial.write(c);
                    header += c;
                    if (c == '\n') {
                        if (current_line.length() == 0) {
                            client.println("HTTP/1.1 200 OK");
                            client.println("Content-type:text/html");
                            client.println("Connection: close");
                            client.println();

                            client.println(
                                "<!DOCTYPE html>"
                                "<html>"
                                    "<head>"
                                        "<meta charset=\"utf-8\">"
                                        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
                                        "<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.0-alpha1/dist/css/bootstrap.min.css\" integrity=\"sha384-GLhlTQ8iRABdZLl6O3oVMWSktQOp6b7In1Zl3/Jr59b6EGGoI1aFkw7cmDA6j6gD\" crossorigin=\"anonymous\">"
                                        "<title>FreeRTOS</title>"
                                    "</head>"
                                    "<body>"
                                        "<div class=\"container mt-5 text-center\">"
                                            "<h1 class=\"display-1\">Jenna-Luz Pura</h1>"
                                            "<h1 class=\"display-6\">Tuesday, March 28, 2023</h1>"
                                        "<div>"
                                    "</body>"
                                "</html>"
                                );

                            // the HTTP response ends with another blank line
                            client.println();
                            // break out of the while loop
                            break;
                        } else { // if you got a newline, then clear currentLine
                            current_line = "";
                        }
                    } else if (c != '\r') {  // if you got anything else but a carriage return character,
                        current_line += c;      // add it to the end of the currentLine
                    }
                }
            }

            // clear the header variable
            header = "";
            // close the connection
            client.stop();
            Serial.println("Client disconnected.");
            Serial.println("");
        }
    }
}
