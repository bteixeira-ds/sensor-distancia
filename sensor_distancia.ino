#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// Conexões do TFT ST7735S
#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  8

// Sensor HC-SR04
#define TRIG_PIN 2
#define ECHO_PIN 3

// Buzzer
#define BUZZER_PIN 7

// Limiares de distância (cm)
#define DISTANCIA_PERIGO  10  // <10cm: Vermelho + bip contínuo
#define DISTANCIA_ALERTA  35  // 10-35cm: Amarelo + bips rápidos
#define DISTANCIA_SEGURA  80  // >35cm: Verde + silêncio

// Cores corrigidas (valores hexadecimais garantidos)
#define VERMELHO 0xF800  // Vermelho puro
#define AMARELO  0xFFE0  // Amarelo puro
#define VERDE    0x07E0  // Verde puro

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(9600);
  
  // 1. Reset robusto do display
  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, LOW);
  delay(100);
  digitalWrite(TFT_RST, HIGH);
  delay(150);

  // 2. Inicialização correta para ST7735S
  tft.initR(INITR_GREENTAB);
  
  // Correção de cores alternativa (usando comandos padrão)
  tft.sendCommand(ST77XX_MADCTL, 0xC0);  // Ajuste de formato de cor
  
  tft.setRotation(1);
  
  // 3. Configuração de hardware
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // 4. Teste inicial (remova após verificar as cores)
  testarCores();
}

void loop() {
  // 1. Medir distância
  float distancia = medirDistancia();
  
  // 2. Atualizar interface
  if(distancia >= 0) {
    atualizarInterface(distancia);
    controlarBuzzer(distancia);
  }
  
  delay(100);
}

// Função de teste de cores (remova após verificação)
void testarCores() {
  tft.fillScreen(VERMELHO);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("VERMELHO");
  delay(3000);

  tft.fillScreen(AMARELO);
  tft.setTextColor(ST7735_BLACK);
  tft.println("AMARELO");
  delay(3000);

  tft.fillScreen(VERDE);
  tft.setTextColor(ST7735_WHITE);
  tft.println("VERDE");
  delay(3000);
}

float medirDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duracao = pulseIn(ECHO_PIN, HIGH, 30000);
  
  if(duracao <= 0) {
    Serial.println("Erro no sensor!");
    return -1;
  }
  
  return duracao * 0.034 / 2;
}

void atualizarInterface(float dist) {
  // Muda cor de fundo conforme distância
  if(dist < DISTANCIA_PERIGO) {
    tft.fillScreen(VERMELHO);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("PERIGO!");
  } 
  else if(dist < DISTANCIA_ALERTA) {
    tft.fillScreen(AMARELO);
    tft.setTextColor(ST7735_BLACK);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("ATENCAO");
  } 
  else {
    tft.fillScreen(VERDE);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("SEGURO");
  }

  // Mostra distância
  tft.setTextSize(3);
  tft.setCursor(10, 40);
  if(dist > 400 || dist < 0) {
    tft.println("---");
  } else {
    tft.print(dist, 0);
    tft.println(" cm");
  }
}

void controlarBuzzer(float dist) {
  static unsigned long ultimoBip = 0;
  
  if(dist < DISTANCIA_PERIGO && dist > 0) {
    tone(BUZZER_PIN, 2000); // Bip contínuo
  } 
  else if(dist < DISTANCIA_ALERTA) {
    int intervalo = map(dist, DISTANCIA_PERIGO, DISTANCIA_ALERTA, 50, 250);
    if(millis() - ultimoBip > intervalo) {
      tone(BUZZER_PIN, 1500, 50);
      ultimoBip = millis();
    }
  } 
  else {
    noTone(BUZZER_PIN);
  }
}