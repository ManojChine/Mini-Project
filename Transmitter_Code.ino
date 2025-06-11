#include <SPI.h>
#include <LoRa.h>

#define FLOW_SENSOR_PIN 15  
#define LORA_NSS 5          
#define LORA_RST 2          
#define LORA_DIO0 4         
#define LORA_FREQUENCY 433E6 

volatile int pulseCount = 0; 
float flowRate = 0.0;        
float totalVolume = 0.0;     

void IRAM_ATTR countPulse() {
    pulseCount++;  
}

void setup() {
    Serial.begin(115200);
    
    pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), countPulse, RISING);

    Serial.println("📡 Initializing LoRa...");
    LoRa.setPins(LORA_NSS, LORA_RST, LORA_DIO0);
    if (!LoRa.begin(LORA_FREQUENCY)) {
        Serial.println("❌ LoRa Initialization Failed!");
        while (1);
    }

    LoRa.setSpreadingFactor(10);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setTxPower(20);
    LoRa.enableCrc();

    Serial.println("✅ LoRa Transmitter Ready!");
}

void loop() {
    static unsigned long lastTime = 0;
    unsigned long currentTime = millis();

    if (currentTime - lastTime >= 1000) {  
        lastTime = currentTime;

        noInterrupts();
        flowRate = (pulseCount / 7.5);  
        totalVolume += (flowRate / 60);  
        pulseCount = 0;  
        interrupts();  

        String message = "FLOW:" + String(flowRate, 2) + ",VOLUME:" + String(totalVolume, 2);
        
        Serial.println("📤 Sending: " + message);
        LoRa.beginPacket();
        LoRa.print(message);
        LoRa.endPacket();  
        Serial.println("✅ Message Sent!");

        delay(2000);
    }
}
