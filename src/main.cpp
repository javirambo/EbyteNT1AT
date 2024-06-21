#include <Arduino.h>
#include "EbyteNT1AT.h"

#define TX_PIN 1
#define RX_PIN 2

EbyteNT1AT Nt1;

void setup()
{
    pinMode(0, INPUT_PULLUP);

    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
    Nt1.Begin(Serial2);

    delay(1000);
    Serial.println("Listo");
}

void loop()
{
    if (digitalRead(0) == LOW)
    {
        Serial.println("\n---------------\nenviando +++AT al otro COM");
        if (Nt1.GoIntoAT())
        {
            // envio comandos:
            Serial.println(Nt1.SendAT("AT+MAC"));
            Serial.println(Nt1.SendAT("AT+VER"));
            Serial.println(Nt1.SendAT("AT+MODEL"));
            Serial.println(Nt1.SendAT("AT+SN"));
            Serial.println(Nt1.SendAT("AT+NAME"));
            Serial.println(Nt1.SendAT("AT+UART"));
            Serial.println(Nt1.SendAT("AT+WAN"));
            Serial.println(Nt1.SendAT("AT+LINKSTA"));

            Serial.println(Nt1.ExitAT());  // salgo del modo AT...
        }
        else
            Serial.println("Error");
        delay(1000);
    }
}
