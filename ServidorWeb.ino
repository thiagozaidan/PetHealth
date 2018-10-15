#include <SPI.h>
#include <WiFi.h>
#include <OneWire.h>

char ssid[] = "default";     //  your network SSID (name) 
//char pass[] = "wzpu2577";    // your network password

const int xPin = 4;     // X output of the accelerometer
const int yPin = 5;     // Y output of the accelerometer
const int zPin = 6;
int i = 5;

int Pino_Temperatura = 2; 
OneWire ds(Pino_Temperatura);

int status = WL_IDLE_STATUS;

boolean door_status = false;

WiFiServer server(80);

void setup() {
  // start serial port for debugging purposes
  Serial.begin(9600);

  Serial.begin(9600);

  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT);
  pinMode(zPin, INPUT);
  
  // Attach interrupt to pin 2
  attachInterrupt(0, setDoorStatus, RISING);

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid);
    // wait 10 seconds for connection:
    delay(10000);
  } 
  server.begin();
  // you're connected now, so print out the status:
  printWifiStatus();
}

// Funções Temperatura
float getTemp(){
  byte data[12];
  byte addr[8];

    if ( !ds.search(addr)) {
 //no more sensors on chain, reset search
 ds.reset_search();
 return -1000;
 }

 if ( OneWire::crc8( addr, 7) != addr[7]) {
 Serial.println("CRC is not valid!");
 return -1000;
 }
 if ( addr[0] != 0x10 && addr[0] != 0x28) {
 Serial.print("Device is not recognized");
 return -1000;
 }
 ds.reset();
 ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end
 byte present = ds.reset();
 ds.select(addr);
 ds.write(0xBE); // Read Scratchpad

 for (int i = 0; i < 9; i++) { // we need 9 bytes
 data[i] = ds.read();
  }

 ds.reset_search();

 byte MSB = data[1];
 byte LSB = data[0];
 float tempRead = ((MSB << 8) | LSB); //using two's compliment
 float TemperatureSum = tempRead / 16;

 return TemperatureSum;
}

void loop() {
  // Acelerometro
   int pulseX, pulseY, pulseZ;
  // variables to contain the resulting accelerations
  int accelerationX, accelerationY, accelerationZ;

  // read pulse from x- and y-axes:
  pulseX = pulseIn(xPin,HIGH);  
  pulseY = pulseIn(yPin,HIGH);
  pulseZ = pulseIn(zPin,HIGH);

  // convert the pulse width into acceleration
  // accelerationX and accelerationY are in milli-g's:
  // earth's gravity is 1000 milli-g's, or 1g.
  accelerationX = (((pulseX / 10) )) /*-500) * 8) -716*/; //I added the last value (-716) to try and calibrate the milli-g reading to 0
  accelerationY = (((pulseY / 10) ));/*- 500) * 8)*/ //I added the last value (-720) to try and calibrate the milli-g reading to 0
  accelerationZ = (((pulseZ / 10) )); /* - 500) * 8)*/ //I added the last value (-712) to try and calibrate the milli-g reading to 0

  // Temperatura
  float temperature = getTemp();
  
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connnection: close");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<head>");
          client.println("<title>PetHealth</title>");
          // ******* TITULO ******
          client.println("</head>");
          client.println("<body>");
          client.println("PetHealth");
        //client.println("<p><img alt=\"\" src=\"http://forum.arduino.cc/Themes/arduinoWide/images/home_icon.gif\" style=\"width: 167px; height: 62px\" /></p>");
          //client.println("<p><img alt=\"\" src=\"https://www.google.com.br/url?sa=i&rct=j&q=&esrc=s&source=imgres&cd=&cad=rja&uact=8&ved=2ahUKEwiC_r270fzdAhWHTJAKHfjwA7MQjRx6BAgBEAU&url=http%3A%2F%2Fvisao.sapo.pt%2Fvisaomais%2F2017-11-05-Porque-sentimosmais-empatia-com-caes-do-que-com-pessoas&psig=AOvVaw1Ia1mf_2sToeH6de3odlmc&ust=1539287107538215\" style =\"width: 167px; height: 62px;\" /></p>");
          // ******* Bloco 1 ******
          client.println("</head>");
          client.println("<body>");
          client.println("A temperatura eh:");
          client.println(temperature);
          if(temperature >= 30.5){
            client.print("\n"); 
            client.println("A temperatura ambiente não está adequada para o seu petzinho");
          }
          // ******* Bloco 2 *****
          client.println("</head>");
          client.println("<body>");
          client.print("\n"); 
             if(accelerationX != 0 || accelerationY != 0 || accelerationZ != 0){
             client.println("O seu petzinho esta se movendo! :)\n"); 
              i=5;
              } else {
            client.println("Parado!\n");
              i--;
          if(i == 0){
          client.println("Cheque seu petzinho ele está ha 12 s parado!\n");
          i=5;
      }
    }
          client.println("<meta http-equiv=\"refresh\" content=\"5\">");
          
            if (door_status == false){
              client.print("Everything is ok");
            }
            else {
              // client.print("Alert ! The door has been opened");
            }       
          client.println("<br />");
          client.println("</html>");
           break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
      // close the connection:
      client.stop();
      Serial.println("client disonnected");
  }
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void setDoorStatus() {
  door_status = true;
}





