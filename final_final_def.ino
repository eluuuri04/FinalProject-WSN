#include <Wire.h>            // Biblioteca per comunicació I2C (no usada directament aquí)
#include "rgb_lcd.h"         // Biblioteca per controlar la pantalla LCD RGB
#include <DHT.h>             // Biblioteca per sensor DHT (temperatura i humitat)
#include <SoftwareSerial.h>  // Biblioteca per crear un port sèrie extra via software

// Definició pins i tipus sensors DHT22
#define DHTPIN1 2       // Pin del sensor d'aire
#define DHTPIN2 4       // Pin del sensor de sorra
#define DHTTYPE DHT22   // Tipus del sensor DHT

// Pins per comunicació UART per software amb un altre dispositiu
#define RX_PIN 10
#define TX_PIN 11
SoftwareSerial uart(RX_PIN, TX_PIN);  // Inicialització de port sèrie software

// Creem objectes dels sensors i la pantalla
DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);
rgb_lcd lcd;

// Pin del relé per controlar el reg
int pinRelay = 7;

// Variables per controlar temporitzadors i intervals
unsigned long lastMeasureTime = 0;    // Últim moment que es van fer les mesures i enviaments
unsigned long lastDisplayTime = 0;    // Últim moment que es va actualitzar la pantalla

const unsigned long intervalMesura = 1800000;  // 30 minuts en mil·lisegons
const unsigned long intervalPantalla = 180000; // 3 minuts en mil·lisegons

// Variables globals per guardar les últimes dades llegides
float hum_aire = 0;
float temp_aire = 0;
float hum_sorra = 0;
float temp_sorra = 0;

bool hasWatered = false;  // Indica si s'ha regat en l'última mesura
bool mostrarSorra = true; // Control per alternar dades a la pantalla

void setup() {
  Serial.begin(9600);     // Inicialització port sèrie hardware per debug
  uart.begin(9600);       // Inicialització port sèrie software per comunicar amb altre dispositiu
  dht1.begin();           // Inicialització sensor d’aire
  dht2.begin();           // Inicialització sensor de sorra

  lcd.begin(16, 2);       // Inicialitza pantalla LCD 16x2
  lcd.setRGB(0, 0, 255);  // Posa color blau (RGB) de fons
  lcd.print("Inicialitzant...");
  delay(2000);            // Espera 2 segons per mostrar missatge inicial
  lcd.clear();

  pinMode(pinRelay, OUTPUT);  // Configura el pin del relé com a sortida
  digitalWrite(pinRelay, LOW); // S'assegura que el relé estigui apagat
}

void loop() {
  unsigned long currentTime = millis(); // Guarda el temps actual des d'inici del programa

  // 🔁 MESURA I ENVIAMENT DE DADES CADA 30 MINUTS
  if (currentTime - lastMeasureTime >= intervalMesura || lastMeasureTime == 0) {
    lastMeasureTime = currentTime;   // Actualitza el moment de la darrera mesura
    hasWatered = false;              // Reinicia flag de reg

    // Llegeix sensors (humitat i temperatura de l’aire i sorra)
    hum_aire = dht1.readHumidity();
    temp_aire = dht1.readTemperature();
    hum_sorra = dht2.readHumidity();
    temp_sorra = dht2.readTemperature();

    // Comprova si alguna lectura no és vàlida
    if (isnan(hum_aire) || isnan(temp_aire) || isnan(hum_sorra) || isnan(temp_sorra)) {
      // Si error, mostra missatge vermell a la pantalla i espera 3 segons
      lcd.setRGB(255, 0, 0);
      lcd.clear();
      lcd.print("Error sensors!");
      delay(3000);
      return; // Surt del loop per evitar més codi
    }

    // 🚿 CONTROL AUTOMÀTIC DEL REG
    if (hum_sorra < 50 && !hasWatered) {  // Si humitat sorra baixa i no s'ha regat encara
      digitalWrite(pinRelay, HIGH);       // Activa relé per engegar el reg
      uart.println("WATERING...");        // Envia missatge per UART software
      lcd.setRGB(0, 0, 255);              // Color blau a la pantalla
      lcd.clear();
      lcd.print("REGANT...");
      delay(20000);                       // Reg durant 20 segons
      digitalWrite(pinRelay, LOW);        // Apaga el relé
      hasWatered = true;                  // Marca que ja s'ha regat

      // Envia dades actuals just després de regar
      enviarDadesUART();
    }

    // Enviament regular de dades cada 30 minuts (tot i que no regui)
    enviarDadesUART();
  }

  // 🖥️ ACTUALITZACIÓ DE PANTALLA CADA 3 MINUTS
  if (currentTime - lastDisplayTime >= intervalPantalla || lastDisplayTime == 0) {
    lastDisplayTime = currentTime;  // Actualitza temps d’última actualització pantalla

    // Canvia color de la pantalla segons humitat de la sorra
    if (hum_sorra > 50) {
      lcd.setRGB(0, 255, 0);  // Verd si humitat alta
    } else {
      lcd.setRGB(255, 0, 0);  // Vermell si humitat baixa
    }

    lcd.clear();
    lcd.setCursor(0, 0);

    if (mostrarSorra) {  // Mostra dades sorra
      lcd.print("Sorra H:");
      lcd.print(hum_sorra, 1);
      lcd.print("%");
      lcd.setCursor(0, 1);
      lcd.print("T:");
      lcd.print(temp_sorra, 1);
      lcd.print((char)223); // Símbol grau °
      lcd.print("C");
    } else {  // Mostra dades aire
      lcd.print("Aire  H:");
      lcd.print(hum_aire, 1);
      lcd.print("%");
      lcd.setCursor(0, 1);
      lcd.print("T:");
      lcd.print(temp_aire, 1);
      lcd.print((char)223); // Símbol grau °
      lcd.print("C");
    }

    mostrarSorra = !mostrarSorra;  // Alterna per a la propera actualització
  }
}

// 📤 Funció que envia les dades per UART software separades per comes
void enviarDadesUART() {
  uart.print(hum_aire);
  uart.print(",");
  uart.print(temp_aire);
  uart.print(",");
  uart.print(hum_sorra);
  uart.print(",");
  uart.print(temp_sorra);
  uart.println();
}
