/*
 * Este archivo es parte del proyecto EbyteNT1AT.
 *
 * Este trabajo ha sido dedicado al dominio público bajo la licencia CC0 1.0 Universal.
 * Para ver una copia de esta licencia, visite:
 * https://creativecommons.org/publicdomain/zero/1.0/
 *
 * Renunciamos a todos los derechos de autor y derechos conexos en la mayor medida
 * permitida por la ley aplicable.
 *
 * Autor: Javier Rambaldo
 * Fecha: 21 de junio de 2024
 */

#include <Arduino.h>
#include "EbyteNT1AT.h"
#include "ModbusRTU.h"

#define TX_PIN 42
#define RX_PIN 41

ModbusRTU ModbusConn(UART_NUM_1);
EbyteNT1AT Nt1(UART_NUM_1);

void setup()
{
    Serial.begin(115200);
    pinMode(0, INPUT_PULLUP);
    delay(1000);

    ModbusConn.Setup(115200, 8, 'N', 1, RX_PIN, TX_PIN, -1, 0, 1000, 0);

    Serial.println("Listo");
}

void loop()
{
    if (digitalRead(0) != LOW)
    {
        uint32_t result = ModbusConn.ReadHoldingRegister(0, 1, 2);
        if (ModbusConn.status == 0)
        {
            Serial.printf("%04X\n", result);
        }
        else
        {
            Serial.printf("Error status=%X\n", ModbusConn.status);
        }
        delay(1000);
    }

    if (digitalRead(0) == LOW)
    {
        Serial.println("\n----------------------\nConfigurando el cosito...");
        if (Nt1.GoIntoAT())
        {
            // 1) Local IP/port/GW:
            Serial.println(Nt1.QueryNetwork());  //+OK=STATIC,192.168.0.7,255.255.255.0,192.168.0.1,114.114.114.114
            Serial.println(Nt1.SetNetwork(Nt1.STATICNetwork, "192.168.0.7", "255.255.255.0", "192.168.0.1", "114.114.114.114"));

            // 2) Target IP: (remote IP...la del PLC porque es MODO CLIENTE)
            Serial.println(Nt1.QueryWorkingMode());  //+OK=TCPC,192.168.0.2,502
            Serial.println(Nt1.SetWorkingMode(Nt1.WorkModeTcpClient, "192.168.0.2", 502));

            // 3) modo MODBUS Simple (conversion RTU<>TCP):
            Serial.println(Nt1.QueryModbusMode());                                            //+OK=SIMPL,1000
            Serial.println(Nt1.SetModbusMode(Nt1.ModbusModeSimpleProtocolConversion, 1000));  //+OK=Modbus TCP to RTU is ON

            //Nt1.Restart();  // tengo que resetear sino no anda...

            // salgo del modo AT...
            Serial.println(Serial.println(Nt1.ExitAT()));  //+OK
            Serial.println("sali del modo AT, ahora mando algo al PLC..");

            delay(500);
        }
        else
            Serial.println("Error");
    }
}
