/*
PINOUT:
RC522 MODULE    Uno/Nano     MEGA
SDA             D10          D9
SCK             D13          D52
MOSI            D11          D51
MISO            D12          D50
IRQ             N/A          N/A
GND             GND          GND
RST             D9           D8
3.3V            3.3V         3.3V
*/
/* Include the standard Arduino SPI library */
#include <SPI.h>
/* Include the RFID library */
#include <RFID.h>

/* Define the DIO used for the SDA (SS) and RST (reset) pins. */
#define SDA_DIO 9
#define RESET_DIO 8
/* Create an instance of the RFID library */
RFID RC522(SDA_DIO, RESET_DIO); 

const int led = 13;
const int buzzer = 40; //buzzer to arduino pin 
bool server_is_connected = false;
/*RFID VALUES*/
String readID;

String request = "POST /api/v1/entries/access/?card=";
String request_type =" HTTP/1.1";

#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

char server[] = "192.168.0.102";    // name address for Google (using DNS)

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 103);
IPAddress myDns(192, 168, 0, 1);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

// Variables to measure the speed
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;
bool printWebData = true;  // set to false for better speed measurement

void init_connection()
{
   // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.print("connecting to ");
  Serial.print(server);
  Serial.println("...");
}

bool connectToServer()
{
  bool is_connected = false;
  init_connection();

  // if you get a connection, report back via serial:
  if (client.connect(server, 8080)) {
    Serial.print("connected to ");
    Serial.println(server);
    is_connected = true;
    server_is_connected = true;
    beginMicros = micros();
  }
  return is_connected;
}

void count_bytes()
{
  // if there are incoming bytes available
  // from the server, read them and print them:
  int len = client.available();
  if (len > 0) {
    byte buffer[80];
    if (len > 80) len = 80;
    client.read(buffer, len);
    if (printWebData) {
      Serial.write(buffer, len); // show in the serial monitor (slows some boards)
    }
    byteCount = byteCount + len;
  }
}

void finish_and_calculate_data_send()
{
    endMicros = micros();
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    Serial.print("Received ");
    Serial.print(byteCount);
    Serial.print(" bytes in ");
    float seconds = (float)(endMicros - beginMicros) / 1000000.0;
    Serial.print(seconds, 4);
    float rate = (float)byteCount / seconds / 1000.0;
    Serial.print(", rate = ");
    Serial.print(rate);
    Serial.print(" kbytes/second");
    Serial.println();
}

void setup()
{ 
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  /* Enable the SPI interface */
  SPI.begin(); 
  /* Initialise the RFID reader */
  RC522.init();
  pinMode(led, OUTPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(led, LOW);
}

void loop()
{
  /* Has a card been detected? */
  if (RC522.isCard())
  {
    /* If so then get its serial number */
    RC522.readCardSerial();
    Serial.println("Card detected:");
       
      /*Clearing*/
      readID = ("");
  
    for(int i=0;i<5;i++)
    {
      Serial.print(RC522.serNum[i],DEC);
      readID+=RC522.serNum[i];
    }
     Serial.print(readID);
     
    tone(buzzer, 1000);
    digitalWrite(led, HIGH);
    delay(500);
    noTone(buzzer); 
    Serial.println();
    Serial.println();
    digitalWrite(led, LOW);

       if (connectToServer())
      {
        // Make a HTTP request:
        //client.println("GET /search?q=arduino HTTP/1.1");
        String req = request + readID + request_type;
        Serial.println(req);
        client.println(req);
        client.println("Host: 192.168.0.102");
        client.println("Connection: close");
        client.println();
        Serial.println("wait for response");
        delay(20000);
        count_bytes();
        //if(!client.connected())
          finish_and_calculate_data_send();
      } else {
        // if you didn't get a connection to the server:
       Serial.println("connection failed");
      }     
      
 
  }
  delay(1000);
}
