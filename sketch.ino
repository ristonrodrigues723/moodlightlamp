/*
 * Smart Mood Lamp - ESP32 Based Intelligent Lighting System
 *
 * try at your own risk not tested this combines 3-5 codes from instru ctable and more into 1 aurdino 
 *esp code i took those compied made my pin asign ments and deeplslep code isnt mine from scratch
 *so i mYay or nor work thwere is no wauy of knowuing 
 */

#include <WiFi.h>              // wifi connection the original had diff file for this
#include <WebServer.h>         //websevr for the color changing the original had this too seperate
#include <NTPClient.h>         //for time
#include <WiFiUdp.h>           //udp connect for ntp
#include <Adafruit_NeoPixel.h> // main neopixel strip contrl
#include <Wire.h>              // color sensor communication with esp32
#include <Adafruit_TCS34725.h> // Library for the TCS34725 RGB Color Sensor
#include <Preferences.h>       //aave colors and state to esp
#include <esp_sleep.h>         // deep sleep so it doesnt have to be on 24/7 i want the power onnlike pc for this
#include <driver/rtc_io.h>     // for wakeup


#define NEOPIXEL_PIN        18    // data pin from esp 
#define MOSFET_PIN          19    // mofst pin
#define BUTTON_PIN          23    // buton pin from esp
#define SDA_PIN             21    // data line 
#define SCL_PIN             22    //line data2

//hardware settings
#define NUM_PIXELS          30    //total leds in strip 
#define BRIGHTNESS_MIN      5     // lowest brightness
#define BRIGHTNESS_MAX      255   // max brightness
#define MAX_PRESETS         15    // fav color setting max
//this is timing control

#define BUTTON_DEBOUNCE_MS  50    
#define LONG_PRESS_MS       500   //longpress for shutdowand deep sleep
#define SENSOR_READ_INTERVAL_MS 2000  // every 2 sec sensor reads light
#define NTP_UPDATE_INTERVAL_MS 3600000  // esp updates itd time from9nternt every jhr
#define DEEP_SLEEP_TIMEOUT_MS 300000    // deepsleep


// the ssd setting for wifi without the lamp doesnt work

const char* ssid = "YOUR_WIFI_SSID";         //name and pass
const char* password = "YOUR_WIFI_PASSWORD";  


   /time settings

const long utcOffsetInSeconds = 19800;  
const char* ntpServer = "pool.ntp.org"; // The server used to get accurate time.


//global object settings

Adafruit_NeoPixel strip(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800); // Creates the NeoPixel objec
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_614MS, TCS34725_GAIN_1X); // Creates the color sensor object
WebServer server(80);         // Sets up the web server
WiFiUDP ntpUDP;               //udp for comm
NTPClient timeClient(ntpUDP, ntpServer, utcOffsetInSeconds); //ntp
Preferences prefs;            // preferance we did earlier


// GLOBAL VARIABLES

RTC_DATA_ATTR bool lampState = false;         
RTC_DATA_ATTR uint32_t currentColor = 0xFF6600;  
RTC_DATA_ATTR unsigned long lastActivityTime = 0; 
RTC_DATA_ATTR int bootCount = 0;              
RTC_DATA_ATTR uint32_t savedColors[MAX_PRESETS]; // An array to hold the saved color values.
RTC_DATA_ATTR int numSavedColors = 0;           // Keeps track of how many colors are currently saved.

float minClearValue = 10.0;    
float maxClearValue = 1000.0;  


struct TimeBasedBrightness {
  int startHour;          
  int endHour;             
  float maxBrightnessFactor; 
};

TimeBasedBrightness brightnessSchedule[] = {
  {7, 19, 1.0},    // From 7 AM to 7 PM, itll use ful brightness
  {19, 23, 0.6},   // From 7 PM to 11 PM itll  use 60% of max brightness.
  {23, 7, 0.1}     // From 11 PM to 7 AM, use 10% of max brightness .
};

// Button handling variables: Used to manage button presses and debouncing.
volatile bool buttonPressed = false;
unsigned long buttonPressTime = 0;  
bool buttonState = false;            
bool lastButtonState = false;        

// Sensor readings and update timers.
float currentClearValue = 0;     // Stores the most recent clear light reading from the sensor.
unsigned long lastSensorRead = 0; // Stores the last time the ambient light sensor was read.
unsigned long lastNTPUpdate = 0;  // Stores the last time NTP client updated the time.


// SETUP FUNCTION- start when esp riuns

void setup() {
  Serial.begin(115200); 
  
 
  bootCount++;
  Serial.println("Boot count: " + String(bootCount));
  
  // Initialize Preferences library to read/write data from flash.
  prefs.begin("moodlamp", false); // "moodlamp" is the namespace, 'false' means read-write mode.
  
  // Load saved state and presets from flash memory.
  loadState();
  
  // Initialize GPIO pins.
  pinMode(MOSFET_PIN, OUTPUT);      // Set MOSFET control pin as an output.
  pinMode(BUTTON_PIN, INPUT_PULLUP); 
  
  // Initializes  communication for the color sensor.
  Wire.begin(SDA_PIN, SCL_PIN); 
  
  // Initialize NeoPixel strip.
  strip.begin(); 
  strip.clear(); 
  strip.show(); 
  
  // Initialize color sensor.
  if (!tcs.begin()) { // Tries to start the TCS34725 sensor.
    Serial.println("Failed to find TCS34725 chip"); // If it can't find the sensor, print an error.
  } else {
    Serial.println("TCS34725 initialized successfully"); // Otherwise, confirm it's working.
  }
  
  // Determine why the ESP32 woke up.
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.println("Woken up by button press");
    lampState = !lampState;
  } else if (bootCount == 1) { 
    Serial.println("Initial boot");
    lampState = false;  // Start the lamp in the OFF state.
  }
  
  // Control the MOSFET based on the lamp's current state.
  digitalWrite(MOSFET_PIN, lampState ? HIGH : LOW); 
  
  if (lampState) {
    // Initialize Wi-Fi and web services.
    initializeWiFi();
    initializeWebServer();
    timeClient.begin(); // Start the NTP client.
    updateNTP();        // Gets the current time.
    
    // Set the LEDs to the last saved color.
    setStripColor(currentColor);
    
    Serial.println("Lamp is ON - All systems initialized");
  } else {
    Serial.println("Lamp is OFF - Entering deep sleep");
    enterDeepSleep(); // Go to deep sleep immediately to save power.
  }
  

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);
  
  lastActivityTime = millis(); // Record the current time as the last activity.
}


// MAIN LOOP of the coide

void loop() {
  if (!lampState) { 
  
    enterDeepSleep();
    return; // Exit the loop immediately.
  }
  
  // Handle incoming web requests from phone redmi in my case
  server.handleClient();
  
  // Check and process button presses.
  handleButton();
  
  // Update sensors periodically to adjust brightness.
  if (millis() - lastSensorRead > SENSOR_READ_INTERVAL_MS) { 
    readAmbientLight(); 
    updateBrightness(); 
    lastSensorRead = millis(); 
  }
  
  // Update NTP periodically to keep time accurate.
  if (millis() - lastNTPUpdate > NTP_UPDATE_INTERVAL_MS) { 
    updateNTP(); 
    lastNTPUpdate = millis(); 
  }
  
  // Check for inactivity to decide if the lamp should go to deep sleep.
  if (millis() - lastActivityTime > DEEP_SLEEP_TIMEOUT_MS) { // If no activity for a while.
    Serial.println("Inactivity timeout - entering deep sleep");
    lampState = false; 
    saveState();       // Save the OFF state.
    enterDeepSleep();  // Go to deep sleep.
  }
  
  delay(10);  // A small pause to prevent the CPU from running at 100% constantly.
}


// INITIALIZATION FUNCTIONS


void initializeWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password); // Tries to connect to your specified Wi-Fi network.
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) { // Keep trying for a limited number of attempts.
    delay(500);
    Serial.print("."); 
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected!");
    Serial.println("IP address: " + WiFi.localIP().toString()); 
  } else { 
    Serial.println("Failed to connect to WiFi");
  }
}

void initializeWebServer() {
  server.on("/", handleRoot);         
  server.on("/setcolor", handleSetColor);
  server.on("/status", handleStatus);   
  server.on("/savepreset", handleSavePreset);
  server.on("/getpresets", handleGetPresets); 
  server.on("/clearpresets", handleClearPresets); 
  server.begin(); // Starts the web server.
  Serial.println("Web server started");
}


// SENSOR FUNCTIONS

void readAmbientLight() {
  uint16_t clear, red, green, blue; // Variables to store the raw color sensor readings.
  
  if (tcs.getRawData(&red, &green, &blue, &clear)) { 
    currentClearValue = clear; // Store the 'clear' channel value (overall light intensity).
    Serial.println("Ambient light (clear): " + String(currentClearValue)); // Print for debugging.
  }
}

void updateBrightness() {
  if (!lampState) return; // If the lamp is off, no need to update brightness.
  

           timeClient.update(); 
  int currentHour = timeClient.getHours(); 
  
  // Calculate brightness based on time of day.
  float timeBrightnessFactor = getTimeBrightnessFactor(currentHour); 
  

 
  float ambientBrightnessFactor = map(currentClearValue, minClearValue, maxClearValue, 0.3, 1.0);
  ambientBrightnessFactor = constrain(ambientBrightnessFactor, 0.1, 1.0); 
  

  float finalBrightnessFactor = timeBrightnessFactor * ambientBrightnessFactor;
  

  int brightness = (int)(BRIGHTNESS_MAX * finalBrightnessFactor)
  brightness = constrain(brightness, BRIGHTNESS_MIN, BRIGHTNESS_MAX); 
  
  strip.setBrightness(brightness);
  strip.show(); // Apply the brightness change to the LEDs.
  
  Serial.println("Brightness updated: " + String(brightness) +  //debugging
                  " (Time factor: " + String(timeBrightnessFactor) + 
                  ", Ambient factor: " + String(ambientBrightnessFactor) + ")");
}

float getTimeBrightnessFactor(int hour) {
  for (int i = 0; i < sizeof(brightnessSchedule) / sizeof(brightnessSchedule[0]); i++) {
    TimeBasedBrightness schedule = brightnessSchedule[i];
    
    if (schedule.startHour <= schedule.endHour) {

      if (hour >= schedule.startHour && hour < schedule.endHour) {
        return schedule.maxBrightnessFactor;
      }
    } else {

      if (hour >= schedule.startHour || hour < schedule.endHour) {
        return schedule.maxBrightnessFactor;
      }
    }
  }
  return 0.5; 
}

=
// NEOPIXEL control

void setStripColor(uint32_t color) {
  // Sets all NeoPixels to a single specified color.
  for (int i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();     
  currentColor = color;
}

uint32_t parseColor(String colorStr) {

  if (colorStr.startsWith("#")) {
    colorStr = colorStr.substring(1);
  }
  
  // Convert the hexadecimal string to a long integer.
  long colorValue = strtol(colorStr.c_str(), NULL, 16);
  

  uint8_t r = (colorValue >> 16) & 0xFF; // Red component
  uint8_t g = (colorValue >> 8) & 0xFF;  // Green component
  uint8_t b = colorValue & 0xFF;         // Blue component
  
  return strip.Color(r, g, b); // Return the color in NeoPixel's internal format.
}


// BUTTON HANDLING-

void IRAM_ATTR buttonISR() {



  buttonPressed = true;
}

void handleButton() {
 
  bool currentButtonState = digitalRead(BUTTON_PIN) == LOW;
  

  if (currentButtonState && !buttonState) {
    buttonPressTime = millis(); 
    buttonState = true;      
    lastActivityTime = millis();
  }
  
  // Detects when the button is released.
  if (!currentButtonState && buttonState) {
    unsigned long pressDuration = millis() - buttonPressTime; 
    buttonState = false;
    
    if (pressDuration < LONG_PRESS_MS) {

      Serial.println("Short press - toggling lamp");
      lampState = !lampState; 
      digitalWrite(MOSFET_PIN, lampState ? HIGH : LOW); // Turn MOSFET ON/OFF based on new lamp state.
      
      if (lampState) { 
        setStripColor(currentColor);
        updateBrightness();         
      } else { 
        strip.clear(); 
        strip.show();  
        saveState();  
        enterDeepSleep();
      }
    } else {
  
      Serial.println("Long press - forcing off");
      lampState = false; // Force the lamp state to OFF.
      strip.clear();     // Turn all LEDs off.
      strip.show();      // Update LEDs.
      digitalWrite(MOSFET_PIN, LOW); // Ensuresm MOSFET is off
      saveState();       // Save the OFF state.
      enterDeepSleep();  // Puts the ESP32 into deep sleep.
    }
  }
  

  if (buttonPressed) {
    buttonPressed = false;
  }
}


// WEB SERVER handlers handels comm bewtwen web server and esp

void handleRoot() {

  String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Smart Mood Lamp</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            font-family: 'Inter', sans-serif; /* Changed font to Inter for a modern look */
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); /* A nice gradient background */
            margin: 0;
            padding: 20px;
            min-height: 100vh; /* Ensures the background covers the whole screen */
            display: flex;
            justify-content: center; /* Centers content horizontally */
            align-items: center; /* Centers content vertically */
            color: #333; /* Default text color */
        }
        .container {
            background: rgba(255, 255, 255, 0.95); /* Slightly transparent white background */
            padding: 40px;
            border-radius: 20px; /* Rounded corners for the main container */
            box-shadow: 0 15px 35px rgba(0, 0, 0, 0.1); /* Soft shadow for depth */
            text-align: center;
            max-width: 450px; /* Increased max-width for better layout */
            width: 100%;
            box-sizing: border-box; /* Ensures padding doesn't increase total width */
        }
        h1 {
            color: #333;
            margin-bottom: 30px;
            font-size: 2.5em; /* Larger heading */
        }
        h2 {
            color: #555;
            margin-top: 30px;
            margin-bottom: 15px;
            font-size: 1.8em;
        }
        .status {
            font-size: 1.2em;
            margin-bottom: 30px;
            padding: 15px;
            border-radius: 10px;
            background: #f0f0f0;
            box-shadow: inset 0 1px 3px rgba(0,0,0,0.1); /* Inner shadow for status box */
        }
        .color-picker {
            margin-bottom: 20px;
            display: flex;
            flex-direction: column; /* Stacks color picker and hex input vertically */
            align-items: center; /* Centers them */
            gap: 15px; /* Space between elements */
        }
        input[type="color"] {
            width: 120px; /* Slightly larger color wheel */
            height: 120px;
            border: none;
            border-radius: 50%; /* Makes it circular */
            cursor: pointer;
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2);
            -webkit-appearance: none; /* Removes default browser styling for color input */
            -moz-appearance: none;
            appearance: none;
            padding: 0;
            overflow: hidden; /* Hides any default borders */
        }
        /* Specific styles to ensure the color swatch is also circular and borderless */
        input[type="color"]::-webkit-color-swatch-wrapper { padding: 0; }
        input[type="color"]::-webkit-color-swatch { border: none; border-radius: 50%; }
        input[type="color"]::-moz-color-swatch-wrapper { padding: 0; }
        input[type="color"]::-moz-color-swatch { border: none; border-radius: 50%; }

        input[type="text"] {
            width: 150px;
            padding: 10px 15px;
            border: 1px solid #ccc;
            border-radius: 8px;
            font-size: 1em;
            text-align: center;
            box-shadow: inset 0 1px 3px rgba(0,0,0,0.1); /* Inner shadow for text input */
        }
        button {
            background: linear-gradient(45deg, #667eea, #764ba2); /* Gradient button background */
            color: white;
            border: none;
            padding: 15px 30px;
            font-size: 1.1em;
            border-radius: 25px; /* Pill-shaped buttons */
            cursor: pointer;
            margin: 10px;
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2); /* Soft shadow */
            transition: transform 0.2s, box-shadow 0.2s; /* Smooth transitions for hover effects */
            outline: none; /* Removes default focus outline */
        }
        button:hover {
            transform: translateY(-2px); /* Lifts button slightly on hover */
            box-shadow: 0 8px 20px rgba(0, 0, 0, 0.3); /* Deeper shadow on hover */
        }
        button:active {
            transform: translateY(0); /* Pushes button down on click */
            box-shadow: 0 3px 10px rgba(0, 0, 0, 0.2); /* Flatter shadow on click */
        }
        .preset-colors, .saved-colors {
            display: flex;
            flex-wrap: wrap; /* Allows color circles to wrap to the next line */
            justify-content: center; /* Centers them */
            gap: 10px; /* Space between color circles */
            margin: 20px 0;
            padding: 10px;
            border: 1px dashed #ddd; /* A dashed border to visually group them */
            border-radius: 10px;
            min-height: 50px; /* Ensures the container is visible even if no colors are saved */
        }
        .preset-color, .saved-color-item {
            width: 40px;
            height: 40px;
            border-radius: 50%; /* Makes them circular */
            border: 2px solid #fff; /* White border around each color */
            cursor: pointer;
            box-shadow: 0 3px 10px rgba(0, 0, 0, 0.2);
            transition: transform 0.2s, border-color 0.2s;
            display: inline-block; /* Ensures they behave like block elements for sizing, but flow inline */
        }
        .preset-color:hover, .saved-color-item:hover {
            transform: scale(1.1); /* Enlarges slightly on hover */
            border-color: #667eea; /* Changes border color on hover for feedback */
        }
        /* Responsive adjustments for smaller screens */
        @media (max-width: 600px) {
            .container {
                padding: 20px;
                margin: 10px;
            }
            h1 {
                font-size: 2em;
            }
            button {
                padding: 12px 20px;
                font-size: 1em;
            }
            .preset-colors, .saved-colors {
                gap: 8px;
            }
            .preset-color, .saved-color-item {
                width: 35px;
                height: 35px;
            }
        }
    </style>
    <script src="https://cdn.tailwindcss.com"></script> <!-- Tailwind CSS for utility classes (though mostly custom CSS here) -->
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;600&display=swap" rel="stylesheet"> <!-- Imports the 'Inter' font from Google Fonts -->
</head>
<body>
    <div class="container">
        <h1>my Smart Mood Lamp</h1>
        <div class="status" id="status">
            Status: <span id="lampStatus">Loading...</span><br>
            Current Color: <span id="currentColor">#FF6600</span>
        </div>
        
        <div class="color-picker">
            <input type="color" id="colorPicker" value="#FF6600"> <!-- The HTML color wheel input -->
            <input type="text" id="hexInput" value="#FF6600" onkeyup="updateColorPicker(this.value)" onchange="applyColor()"> <!-- Text input for hex codes -->
        </div>
        
        <button onclick="applyColor()">Apply Color</button> <!-- Button to apply the chosen color -->
        <button onclick="saveCurrentColorAsPreset()">Save Current Color</button> <!-- Button to save the current color as a preset -->
        <button onclick="updateStatus()">Refresh Status</button> <!-- Button to manually refresh lamp status -->

        <h2>Saved Colors</h2> <!-- Heading for the saved colors section -->
        <div class="saved-colors" id="savedColorsContainer">
            <!-- Saved colors will be loaded here dynamically by JavaScript -->
        </div>
        <button onclick="clearSavedColors()">Clear Saved Colors</button> <!-- Button to clear all saved presets -->

    </div>

    <script>

        function updateColorPicker(hex) {
            // Basic validation to ensure the input looks like a hex color.
            if (/^#?([0-9A-F]{3}){1,2}$/i.test(hex)) {
                document.getElementById('colorPicker').value = hex; // Set the color picker's value.
            }
        }

    
        function updateHexInput(color) {
            document.getElementById('hexInput').value = color;
        }


        document.getElementById('colorPicker').addEventListener('input', function() {
            updateHexInput(this.value);
        });

        function setColor(color) {
            document.getElementById('colorPicker').value = color;
            updateHexInput(color); 
            applyColor(); // Sends the color to the ESP32 to i send to din of neopixel.
        }
        
       
        function applyColor() {
            const color = document.getElementById('colorPicker').value;
    
               fetch('/setcolor?color=' + encodeURIComponent(color))
                .then(response => response.text()) 
                .then(data => {
                    console.log('Color set:', data);
                    updateStatus(); 
                })
                .catch(error => {
                    console.error('Error:', error); 
                });
        }
        
                   function updateStatus() {
            fetch('/status')
                .then(response => response.json()) 
                  .then(data => {
                    document.getElementById('lampStatus').textContent = data.state ? 'ON' : 'OFF'; 
                    document.getElementById('currentColor').textContent = data.color; 
                    document.getElementById('colorPicker').value = data.color;
                    document.getElementById('hexInput').value = data.color; 
                })
                .catch(error => {
                    console.error('Error:', error);
                });
            loadSavedColors();
        }
        function saveCurrentColorAsPreset() {
            fetch('/savepreset') 
                .then(response => response.text())
                .then(data => {
                    console.log('Preset saved:', data);
                    loadSavedColors(); 
                })
                .catch(error => {
                    console.error('Error saving preset:', error);
                });
        }

        // Function to clear all saved color presets.
        function clearSavedColors() {
 
            if (confirm('Are you sure you want to clear all saved colors?')) { 
                fetch('/clearpresets') 
                    .then(response => response.text())
                    .then(data => {
                        console.log('Presets cleared:', data);
                        loadSavedColors(); 
                    })
                    .catch(error => {
                        console.error('Error clearing presets:', error);
                    });
            }
        }

  
        function loadSavedColors() {
            fetch('/getpresets')
                .then(response => response.json()) 
                .then(data => {
                    const container = document.getElementById('savedColorsContainer');
                    container.innerHTML = ''; 
                    if (data && data.length > 0) { 
                        data.forEach(color => { 
                            const colorDiv = document.createElement('div');
                            colorDiv.className = 'saved-color-item'; 
                            colorDiv.style.background = color; 
                            colorDiv.onclick = () => setColor(color); 
                            container.appendChild(colorDiv); 
                        });
                    } else { // If no colors are saved.
                        container.innerHTML = '<p style="color: #666; font-size: 0.9em;">No saved colors yet. Click "Save Current Color"!</p>';
                    }
                })
                .catch(error => {
                    console.error('Error loading presets:', error);
                    document.getElementById('savedColorsContainer').innerHTML = '<p style="color: red; font-size: 0.9em;">Failed to load saved colors.</p>';
                });
        }
        
      
        updateStatus();
        
        setInterval(updateStatus, 10000);
    </script>
</body>
</html>
)";
  
  server.send(200, "text/html", html);
  lastActivityTime = millis();
}

void handleSetColor() {
  if (server.hasArg("color")) {
    String colorStr = server.arg("color"); 
    uint32_t newColor = parseColor(colorStr);
    
    if (lampState) {
      setStripColor(newColor);
      updateBrightness(); 
    
    currentColor = newColor; 
    saveState(); 
    
    server.send(200, "text/plain", "Color set to: " + colorStr); 
    Serial.println("Color changed to: " + colorStr); 
  } else {
    server.send(400, "text/plain", "Missing color parameter");
  }
  
  lastActivityTime = millis();
}

   void handleStatus() {
  // This function builds a JSON string containing the lamp's current status.
  String json = "{";
     json += "\"state\":" + String(lampState ? "true" : "false") + ",";
  json += "\"color\":\"#" + String(currentColor, HEX) + "\","; 
   json += "\"brightness\":" + String(strip.getBrightness()) + ","; 
  json += "\"ambient_light\":" + String(currentClearValue) + ","; 
  json += "\"time\":\"" + timeClient.getFormattedTime() + "\""; 
   json += "}";
  
       server.send(200, "application/json", json); 
  lastActivityTime = millis();
}

// New Web Server Handlers for Presets
    void handleSavePreset() {
  if (numSavedColors < MAX_PRESETS) {
    savedColors[numSavedColors] = currentColor; 
    numSavedColors++; 
    saveState(); 
    server.send(200, "text/plain", "Preset saved successfully."); 
    Serial.println("Preset saved: #" + String(currentColor, HEX)); 
  } else {
    server.send(400, "text/plain", "Maximum number of presets reached."); 
    Serial.println("Failed to save preset: Max reached."); 
  }
  lastActivityTime = millis(); 
}

void handleGetPresets() {
  // This function builds a JSON array of all saved color presets.
  String json = "[";
  for (int i = 0; i < numSavedColors; i++) {
    json += "\"#" + String(savedColors[i], HEX) + "\""; 
    if (i < numSavedColors - 1) { 
      json += ",";
    }
  }
  json += "]";
  server.send(200, "application/json", json); 
  lastActivityTime = millis(); 
}

void handleClearPresets() {
  numSavedColors = 0; // Resets the count of saved colors to zero.
  
  for (int i = 0; i < MAX_PRESETS; i++) {
    char key[10];
    sprintf(key, "color%d", i); 
    prefs.remove(key); 
  }
  saveState();
  server.send(200, "text/plain", "All presets cleared."); 
  Serial.println("All presets cleared."); 
  lastActivityTime = millis(); 
}



// STATE MANAGEMENT

void saveState() {
  prefs.putBool("lampState", lampState);     // Saves the current ON/OFF state.
  prefs.putUInt("currentColor", currentColor); // Saves the last set color.
  
  // Saves the number of currently saved presets.
  prefs.putInt("numColors", numSavedColors);
  for (int i = 0; i < numSavedColors; i++) {
    char key[10];
    sprintf(key, "color%d", i); 
    prefs.putUInt(key, savedColors[i]); 
  }
  Serial.println("State and presets saved."); 
}

void loadState() {
  lampState = prefs.getBool("lampState", false); 
  currentColor = prefs.getUInt("currentColor", 0xFF6600);
  
  // Load the number of saved presets.
  numSavedColors = prefs.getInt("numColors", 0);
  for (int i = 0; i < numSavedColors; i++) {
    char key[10];
    sprintf(key, "color%d", i);
    savedColors[i] = prefs.getUInt(key, 0); //it will Load the color value.
  }
  Serial.println("State and presets loaded - Lamp: " + String(lampState) + ", Current Color: #" + String(currentColor, HEX) + ", Saved Presets: " + String(numSavedColors)); // Print confirmation.
}


// Power management

void enterDeepSleep() {
  Serial.println("Entering deep sleep...");
  

  strip.clear(); // Turns off all NeoPixels.
  strip.show();  // Updates the strip.
  digitalWrite(MOSFET_PIN, LOW); // will Turn off power to NeoPixels via MOSFET.
  
  // Saves the current state before sleep
  saveState();
  


  esp_sleep_enable_ext0_wakeup(GPIO_NUM_23, 0);  // Matches BUTTON_PIN definition.
  
  // Start deep sleep. The ESP32 will consume very little power until the button is pressed like laptop no idea if this will ever work.
  esp_deep_sleep_start();
}


// Helper functions.

void updateNTP() {
  if (WiFi.status() == WL_CONNECTED) { 
    timeClient.update(); // Fetchs the latest time from the NTP server.
    Serial.println("NTP updated: " + timeClient.getFormattedTime()); 
  }
}
