#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

const char *ssid = "WLAN-HCKSEJ";
const char *password = "842105025012500625";
int ledPin = 2;
WiFiServer server(80);

#define PIN 4
#define NUMPIXELS 144
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
    Serial.begin(115200);
    delay(10);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, HIGH);
    // Connect to WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");

    // Start the server
    server.begin();
    Serial.println("Server started");

    // Print the IP address
    Serial.print("Use this URL to connect: ");
    Serial.print("http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");
    pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
}

int r = 255;
int g = 0;
int b = 0;

typedef struct
{
    int a;
    int b;
} AB_struct;

AB_struct *ab = NULL; //AB_struct pointer. Init to NULL(0)

void loop()
{

    // Check if a client has connected
    WiFiClient client = server.available();
    if (!client)
    {
        return;
    }

    // Wait until the client sends some data
    int timeout = 1000;
    int time = 0;
    digitalWrite(ledPin, LOW);

    while (!client.available())
    {
        time++;
        delay(1);
        if (time >= timeout)
        {
            Serial.println("client error");
            digitalWrite(ledPin, HIGH);
            return;
        }
    }

    // Read the first line of the request
    //String request = client.readString();
    while (client.available())
    {

        bool finished = false;

        while (!finished)
        {
            String data = client.readStringUntil('\n');
            if (data[0] == '{')
            {
                DynamicJsonDocument doc(16384 );
                DeserializationError error = deserializeJson(doc, data);
                client.flush();
                data = "";
                if (error)
                {
                    Serial.print(F("deserializeJson() failed: "));
                    Serial.println(error.c_str());
                    return;
                }

                if (doc["method"] == "setColor")
                {
                    int r = doc["data"][0];
                    int g = doc["data"][1];
                    int b = doc["data"][2];

                    for (int i = 0; i < NUMPIXELS; i++)
                    {
                        pixels.setPixelColor(i, pixels.Color(r, g, b));
                    }
                    pixels.show();
                    Serial.println("method: setColor");
                }
                else if (doc["method"] == "set")
                {
                    for (int i = 0; i < NUMPIXELS; i++)
                    {
                        pixels.setPixelColor(i, pixels.Color(doc["data"][i][0], doc["data"][i][1], doc["data"][i][2]));
                    }
                    doc["data"] = NULL;
                    pixels.show();
                    Serial.println("method: set");
                }
                else if (doc["method"] == "update")
                {
                    for (int i = 0; i < NUMPIXELS; i++)
                    {
                        pixels.setPixelColor(doc["data"][i][0], pixels.Color(doc["data"][i][1], doc["data"][i][2], doc["data"][i][3]));
                    }
                    pixels.show();
                    Serial.println("method: update");
                }
                else if (doc["method"] == "clear")
                {
                    pixels.clear();
                    pixels.show();
                    Serial.println("method: clear");
                }

                finished = true;
            }
        }

        client.println(200);
        digitalWrite(ledPin, HIGH);
    }
}