#include <ESP8266WiFi.h>
#include <ESP8266Firebase.h>
// List of allowed SSIDs and their corresponding passwords
const char* allowedSSIDs[] = {"Room1", "Room2","Parking","Terrace"};
const char* allowedPasswords[] = {"12345678", "12345678","12345678","12345678"};
const int allowedNetworksCount = sizeof(allowedSSIDs) / sizeof(allowedSSIDs[0]);

#define REFERENCE_URL "https://woman-safety-project-default-rtdb.firebaseio.com/" //Do not include https:// in FIREBASE_HOST
Firebase firebase(REFERENCE_URL);

int ConnectionStatus=0;
int Nodeid=2;

int triggerThreshold=130;

const int pulsePin = A0; // Pulse sensor connected to analog pin A0
int signal;               // Holds the incoming raw data
int filteredSignal;       // Holds the filtered signal value
bool pulseDetected = false;       // True when pulse wave is high
unsigned long lastBeatTime = 0; // Time at which the last beat occurred
unsigned long intervalStart = 0; // Start of the 5-second interval

const float alpha = 0.75; // Constant for low-pass filter
int beatIntervals[100];   // Array to store intervals between beats
int beatCount = 0;        // Count of beats detected in the current interval

const int buttonPin = 4;     // Pin connected to the push button

int buttonState = HIGH;      // Current state of the button
int lastButtonState = HIGH;  // Previous state of the button

unsigned long lastDebounceTime = 0;  // Last time the button state changed
unsigned long debounceDelay = 50;    // Debounce delay time in milliseconds
unsigned long longPressThreshold = 1000;  // Define the time threshold for a long press

String HeartTrig="0";
String EmerTrig="0";
String HelpTrig="0";
void buttonTracker();

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  ConnectionStatus=checkAndConnectToStrongestNetwork(true,ConnectionStatus); // Force initial connection
  intervalStart = millis();
  pinMode(buttonPin, INPUT_PULLUP); // Set pin mode to INPUT_PULLUP
  firebase.setString("Database/Node"+String(Nodeid)+"/MasterSignal", String(HeartTrig+EmerTrig+HelpTrig));
}

void loop() {
  // Periodically check for a stronger network and switch if necessary
  static unsigned long lastCheckTime = 0;
  if (millis() - lastCheckTime > 5000) { // Every 3 seconds
    lastCheckTime = millis();
    ConnectionStatus=checkAndConnectToStrongestNetwork(false,ConnectionStatus); // Do not force reconnection if already on the strongest network
  }
  if(ConnectionStatus==1){
    //Serial.println("Execute..");
    PulseRateMonitor();
    buttonTracker();

  }
  else{
    Serial.println("Connection Lost");
  }
  // Your main code here
  delay(10); // Adjust according to your needs
}

int checkAndConnectToStrongestNetwork(bool forceReconnect,int ConnectionStatus) {
  
  Serial.println("Scanning for networks...");
  int n = WiFi.scanNetworks();
  Serial.println("Scan complete");
  if (n == 0) {
    Serial.println("No networks found");
    ConnectionStatus=0;
    return 0;
  }

  String currentSSID = WiFi.SSID();
  long currentSignalStrength = WiFi.RSSI();
  int strongestSignalIndex = -1;
  long strongestSignal = -10000; // Start with a very low signal

  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < allowedNetworksCount; ++j) {
      buttonTracker();
      if (WiFi.SSID(i) == String(allowedSSIDs[j]) && WiFi.RSSI(i) > strongestSignal) {
        strongestSignalIndex = i;
        strongestSignal = WiFi.RSSI(i);
        break; // Found an allowed network with a strong signal
      }
    }
  }

  // Check if we need to switch networks
  if (strongestSignalIndex != -1 && (forceReconnect || (WiFi.SSID(strongestSignalIndex) != currentSSID && strongestSignal > currentSignalStrength))) {
    String selectedSSID = WiFi.SSID(strongestSignalIndex);
    const char* password = "";

    // Match the selected SSID to its password
    for (int i = 0; i < allowedNetworksCount; ++i) {
      buttonTracker();
      if (selectedSSID == String(allowedSSIDs[i])) {
        password = allowedPasswords[i];
        break;
      }
    }

    Serial.print("Switching to ");
    Serial.println(selectedSSID);
    
    WiFi.begin(selectedSSID.c_str(), password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      Serial.print(".");
      buttonTracker();
    }

    Serial.println("");
    Serial.println("WiFi connected");
    ConnectionStatus=1;
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    firebase.setString("Database/Node"+String(Nodeid)+"/Located", selectedSSID);
  } else if (strongestSignalIndex == -1) {
    Serial.println("No allowed networks found with stronger signal.");
    ConnectionStatus=0;
  } else {
    Serial.println("Already connected to the strongest allowed network.");
    ConnectionStatus=1;

  }
  return ConnectionStatus;
}


void PulseRateMonitor(){
  signal = analogRead(pulsePin);
  filteredSignal = lowPassFilter(signal);
  detectBeat(filteredSignal);

  if (millis() - intervalStart >= 10000) { // 5-second interval
    if (beatCount > 0) { // Ensure at least one beat is detected
      float averageBPM = calculateAverageBPM();

      if (averageBPM >= 60 && averageBPM <= 170) { // Check if BPM is within the acceptable range
        Serial.print("Average BPM over 5 seconds: ");
        Serial.println(averageBPM);
        firebase.setString("Database/Node"+String(Nodeid)+"/Heartrate", String(averageBPM));
        if(averageBPM>triggerThreshold){
        HeartTrig="1";
        EmerTrig="0";
        HelpTrig="0";
        firebase.setString("Database/Node"+String(Nodeid)+"/MasterSignal", String(HeartTrig+EmerTrig+HelpTrig));  
        }

      } else {
        Serial.println("Detected BPM out of normal range, possibly noise.");
      }
    } else {
      Serial.println("No beats detected.");
    }

    // Reset for the next interval
    beatCount = 0;
    intervalStart = millis();
  }

  delay(10); // Delay for stability
  //Serial.print("Monitoring");

}

int lowPassFilter(int signal) {
  static int prevFilteredSignal = 0;
  int filteredSignal = alpha * prevFilteredSignal + (1 - alpha) * signal;
  prevFilteredSignal = filteredSignal;
  return filteredSignal;
}

void detectBeat(int signal) {
  unsigned long currentTime = millis();
  const int threshold = 550; // Adjust based on your sensor signal
  const int maxInterval = 60000 / 60;  // Maximum interval for 60 BPM
  const int minInterval = 60000 / 150; // Minimum interval for 150 BPM

  if (signal > threshold && !pulseDetected) {
    pulseDetected = true;
    unsigned long pulseInterval = currentTime - lastBeatTime;
    lastBeatTime = currentTime;

    // Only accept the beat if it's within the expected range
    if (pulseInterval >= minInterval && pulseInterval <= maxInterval) {
      beatIntervals[beatCount++] = pulseInterval;
    }
  }

  if (signal < threshold && pulseDetected) {
    pulseDetected = false;
  }
}

float calculateAverageBPM() {
  int totalInterval = 0;
  for (int i = 0; i < beatCount; i++) {
    totalInterval += beatIntervals[i];
  }
  return (60000.0 * beatCount) / totalInterval; // Calculate average BPM
}


void buttonTracker() {
  int reading = digitalRead(buttonPin);
  //Serial.println(reading);

  // Check if the button state has changed
  if (reading != lastButtonState) {
    // Reset the debounce timer
    lastDebounceTime = millis();
  }

  // Check if a certain amount of time has passed since the last button state change
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If the button state has changed
    if (reading != buttonState) {
      buttonState = reading;

      // If the button is pressed down
      if (buttonState == LOW) {
        Serial.println("Button pressed!");
        //firebase.setString("Database/Node"+String(Nodeid)+"/Help", "1");

        // Wait for the button to be released or for the long press threshold to be reached
        unsigned long startTime = millis();
        while (digitalRead(buttonPin) == LOW) {
          if ((millis() - startTime) > longPressThreshold) {
            Serial.println("Long press detected! -- EMERGENCY");
            firebase.setString("Database/Node"+String(Nodeid)+"/Emergency", "1");
            
            HeartTrig="0";
            EmerTrig="1";
            HelpTrig="0";
            firebase.setString("Database/Node"+String(Nodeid)+"/MasterSignal", String(HeartTrig+EmerTrig+HelpTrig));
            return;
          }
          
        }
        firebase.setString("Database/Node"+String(Nodeid)+"/Help", "1");
        HeartTrig="0";
        EmerTrig="0";
        HelpTrig="1";
        firebase.setString("Database/Node"+String(Nodeid)+"/MasterSignal", String(HeartTrig+EmerTrig+HelpTrig));
      }
    }
  }

  // Save the current button state for comparison in the next iteration
  lastButtonState = reading;
}