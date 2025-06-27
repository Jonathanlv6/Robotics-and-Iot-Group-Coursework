#define BLYNK_TEMPLATE_ID "TMPL6g_hfaQCK"
#define BLYNK_TEMPLATE_NAME "24046566 Ankit Ghimire"
#define BLYNK_AUTH_TOKEN "Ap30BZLZqN4CfNcvu_T_x1nMxMuq8yHM"

// All the libraries needed for the project
#include <WiFi.h> 
#include <WiFiClient.h> 
#include <BlynkSimpleEsp32.h> 
#include <DHT.h> 
#include <DHT_U.h>  
#include <LiquidCrystal_I2C.h> 



#define DHTPIN 5 // GPIO pin where the DHT11 data line is connected
#define DHTTYPE DHT22 // DHT 22 is used in this case
#define I2C_ADDR 0x27 // Memory Address of the LCD-I2C display
#define LCD_COLUMNS 16 // number of columns in the display
#define LCD_ROWS 2 // number of columns in the display


char ssid[] = "Ankit's iphone"; // wifi ssid stored saved as a character array
char pass[] = "12345679"; // wifi password

BlynkTimer timer; // declaring a BlynkTimer object for sending real time data to Blynk in set interval

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE); // declaring DHT object for operating the dht sensor

const int trigPin = 19; // initializing trigger pin of the ultrasonic sensor
const int echoPin = 18; // inistializing echo pin of the ultrasonic sensor
long duration; // declaring a duration that stores how long did the ultrasound take after reflecting
int distance; // to store distance from the ultrasonic sensor
float waterLevel; // to store how the percentage of water left in water tank


// Create an LCD object
LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_ROWS);

#define soilPin 36 // soil moisture sensor anaglog pin which is VP
float moisture; // to store percentage of soil moisture from soil moisture sensor

const int relayPin = 23; // pin connection to the relay module.

const int buzzerPin = 2; // buzzer positive terminal pin connection
bool buzzerState = false; // to store state of buzzer being turn on or off

bool currentState = true; // dynamic state for system to act according to certain events



void mainFunction(){
  // mainFunction() is the core function that holds all sensor data reading and sending to Blynk app.
  // As well as doing certain actions like turning on or off the relay switch when necessary.
  
  /*
  * Reading Value from ultrasonic sensor through sending ultrasonic sound through trigger pin
  * and reading the data from echo pin with pulsein() funciton
  */
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH); // reading the time taken for the ultrasonic sound to come back
  distance = duration * 0.034/2; // calculating the time taken and find the distance from ultrasonic sensor
  waterLevel = ((15-(float)distance)/10)*100; // calculating water level

  // 15 is the value of ultrasonic sensor when the tank is empty in cm. 
  // 10 comes from subtracting 5 from 15 cause 5 is the value of ultrasonic sensor when the tank is full in cm. 


  // Read humidity (in %)
  float humidity = dht.readHumidity();
  // Read temperature (in °C)
  float temperature = dht.readTemperature();
  // Check if any readings faileds
  if (isnan(humidity) || isnan(temperature)) {
    lcd.clear();
    lcd.print("Fail");
    return;
  }

  // calculating the moisture reading from soil moisture sensor and calculating moisture in %
  moisture  = ((4095-(float)analogRead(soilPin))/3300)*100;
  // 4095 is the value of soil moisture sensor when the soil is driest.
  // we get 3300 when we subtract 795 from 4095 cause 
  // 795 is the value of soil moisture sensor when the soil is most damp.


  /*
  Sending all the data collected to Blynk app with virtual pins with
  correct data type that is already defined in the Blynk app.
  */
  Blynk.virtualWrite(V0, (int)waterLevel);
  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, (int)humidity);
  Blynk.virtualWrite(V3, (int)moisture);
  if (digitalRead(relayPin) == LOW){
    Blynk.virtualWrite(V4, 1); // for a virtual LED in Blynk app
  }else{
    Blynk.virtualWrite(V4, 0); // for a virtual LED in Blynk app
  }



  

  // Print values to lcd
  if (currentState == true){
    // Showing all the values of sensors in lcd
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("water lvl:");
    lcd.print(waterLevel);
    lcd.print("%");
    lcd.setCursor(0,1);
    lcd.print("H:");
    lcd.print((int)humidity);
    lcd.print("%");
    lcd.setCursor(6,1);
    lcd.print("T:");
    lcd.print(temperature,1);
    lcd.print((char) 223);
    lcd.print("C");
    currentState = false;
    // changing the state to ignore the content of the if block next time and execute else block.
  }else {
    // showing moisture level in next state
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("soil moisture: ");
    lcd.setCursor(0, 1);
    lcd.print(moisture);
    lcd.print("%");
    currentState = true;
    // changing the state to ignore the content of the else block next time and execute if block.
  }

  /*
  irrigation logics and conditions for smart farming
  */
  if ((int)waterLevel<=20){
    digitalWrite(relayPin,HIGH); // turning off the relay module
    Blynk.virtualWrite(V5, "Water Level too Low!"); // informing farmers using Blynk app
    buzzerState = true; // to turn on the buzzer if its off.
  }


  
  if ((int)moisture < 40){
    if ((int)waterLevel>=20){
      digitalWrite(relayPin,LOW); // turning on the relay module
      Blynk.virtualWrite(V5, "Dry Soil! Irrigation in progress"); // informing farmers using Blynk app
      buzzerState = false; // to turn off the buzzer if its on.
    }
  }else{
    digitalWrite(relayPin,HIGH); // turning off the relay module
    if ((int)waterLevel>=20){
      Blynk.virtualWrite(V5, "System Running Smoothly"); // informing farmers using Blynk app
      buzzerState = false; // to turn off the buzzer if its on.
    }

  }

  if (buzzerState == true){
    // creating certain sounds with buzzer
    tone(buzzerPin, 400); // play the buzzer with 400 Khz sound
    delay(100);
    tone(buzzerPin, 600); // play the buzzer with 400 Khz sound
    delay(100);
  }else{
    noTone(buzzerPin); // turning off the buzzer
  }
}

void setup() {
  pinMode(trigPin, OUTPUT); // initializing the trigger pin for output
  pinMode(echoPin, INPUT); // initializing the echo pin for input
  dht.begin();
  // Initialize the LCD
  lcd.init();
  lcd.backlight(); // Turn on LCD backlight
  pinMode(soilPin, INPUT); // initializing the soil moisture pin for input
  pinMode(relayPin, OUTPUT); // initializing the relay switch pin for output
  pinMode(buzzerPin, OUTPUT); // initializing the buzzer pin for output


  digitalWrite(relayPin,HIGH); // turning off the relay when the system first boots up

  /*
    playing cetrain sound through buzzer when the system first boots upp
  */
  tone(buzzerPin,800);
  delay(500);
  tone(buzzerPin,200);
  delay(300);
  tone(buzzerPin,700);
  delay(200);
  noTone(buzzerPin);

  lcd.print("Starting"); // informing the farmers that the system is booting up


  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); // initializing the blynk connection through Wifi
  /*
  timer will allow us to call a certain function (mainFunction in this case) in certain amount of time 
  in a loop which is 1000 milliseconds or 1 second in this case.
  */
  timer.setInterval(1000L, mainFunction); 



}



void loop() {
  Blynk.run(); // Connect with Blynk app and establish real time connection.
  timer.run(); // call the timer that was initialized in the setup that will invoke the mainFunction()
  // every 1 second
}

