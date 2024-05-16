#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266Firebase.h>
#include <ArduinoJson.h>

#include <Base64.h>

#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

#define REFERENCE_URL "https://woman-safety-project-default-rtdb.firebaseio.com/" //Do not include https:// in FIREBASE_HOST
Firebase firebase(REFERENCE_URL);

const char* twilioAccountSid = "AC19bcacffd13d450737cb04a7a8364f9f";
const char* twilioAuthToken = "122f5070752f0197152c64afd1cd655e";
const char* twilioNumber = "+19149158335"; // Include country code
const char* toNumber = "+918792373343"; // Include country code

//https://woman-safety-project-default-rtdb.firebaseio.com/
const char* ssid = "Room1";
const char* password = "12345678";

LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 columns and 2 rows
String Enable="111";

bool Node1msgSent=false;
bool Node2msgSent=false;

const int buzzerPin = 2;

void generateTone(int frequency, int duration) {
  for (long i = 0; i < duration * frequency; i++) {
    digitalWrite(buzzerPin, HIGH);
    delayMicroseconds(500000 / frequency); // Half period in microseconds
    digitalWrite(buzzerPin, LOW);
    delayMicroseconds(500000 / frequency); // Half period in microseconds
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin,LOW);
  lcd.init();                      // Initialize LCD
  lcd.backlight();                 // Turn on backlight

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  lcd.setCursor(0, 0);
  lcd.print("  WOMAN SAFETY");
  lcd.setCursor(0, 1);
  lcd.print("     SYSTEM");
  
    String Node1data = firebase.getString("/Database/Node1/MasterSignal");
    String Node2data = firebase.getString("/Database/Node2/MasterSignal");
    Serial.print("Node1data:");
    Serial.println(Node1data);
    Serial.print("Node2data:");
    
    if(Node1data[0]=='1'){
      lcd.clear();
      Serial.print("HeartRate1");
      AlarmAndDisplay("Node1","HEARTRATE!",Node1msgSent);
      Node1msgSent=true;
    }
    else if(Node1data[1]=='1'){
      lcd.clear();
      Serial.print("Emergency1");
      AlarmAndDisplay("Node1","EMERGENCY!",Node1msgSent);
      Node1msgSent=true;
    }
    else if(Node1data[2]=='1'){
      lcd.clear();
      Serial.print("Help1");
      AlarmAndDisplay("Node1","HELP!",Node1msgSent);
      Node1msgSent=true;
    }
    else{
      Node1msgSent=false;
    }

    if(Node2data[0]=='1'){
      lcd.clear();
      Serial.print("HeartRate2");
      AlarmAndDisplay("Node2","HEARTRATE!",Node2msgSent);
      Node2msgSent=true;
    }
    else if(Node2data[1]=='1'){
      lcd.clear();
      Serial.print("Emergency2");
      AlarmAndDisplay("Node2","EMERGENCY!",Node2msgSent);
      Node2msgSent=true;
    }
    else if(Node2data[2]=='1'){
      lcd.clear();
      Serial.print("Help2");
      AlarmAndDisplay("Node2","HELP!",Node2msgSent);
      Node2msgSent=true;
    }
    
  delay(100); // Update every second
}
 // Use any digital pin, such as D1

// Function to generate a tone of a specific frequency for a certain duration
void AlarmAndDisplay(String NodeID,String msg,bool smssent){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Alert! - "+String(NodeID));
  lcd.setCursor(0,1);
  lcd.print("   "+msg);
  for(int i=0;i<3;i++){
digitalWrite(buzzerPin,HIGH);
delay(250);
digitalWrite(buzzerPin,LOW);
delay(250);
  }
lcd.clear();
if(!smssent){
  Serial.println("sending Alert Message");
  //SendTwilio(NodeID ,msg);
  //sendSMS("Hi,Im ESP");
}
}

// void sendSMS(const char* message) {
//   HTTPClient http;

//   // Construct the Twilio API URL
//   String url = "https://api.twilio.com/2010-04-01/Accounts/";
//   url += String(twilioAccountSid);
//   url += "/Messages.json";

//   // Construct the authentication header
//   String auth = String(twilioAccountSid) + ":" + String(twilioAuthToken);
//   String encodedAuth = base64::encode(auth);

//   // Set up HTTP request headers
//   http.begin(url);
//   http.addHeader("Content-Type", "application/x-www-form-urlencoded");
//   http.addHeader("Authorization", "Basic " + encodedAuth);

//   // Construct the message body
//   String body = "To=" + String(toNumber);
//   body += "&From=" + String(twilioNumber);
//   body += "&Body=" + String(message);

//   // Send HTTP POST request with the message body
//   int httpResponseCode = http.POST(body);

//   if (httpResponseCode > 0) {
//     Serial.print("SMS sent! Response code: ");
//     Serial.println(httpResponseCode);
//     String payload = http.getString();
//     Serial.println("Response payload: " + payload);
//   } else {
//     Serial.print("Failed to send SMS. Error code: ");
//     Serial.println(httpResponseCode);
//     String error = http.errorToString(httpResponseCode);
//     Serial.println("Error: " + error);
//   }

//   http.end();
// }