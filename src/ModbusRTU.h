#pragma once
#include <Arduino.h>
#include "driver/uart.h"

inline uint16_t lowWord(uint32_t ww) { return (uint16_t)((ww) & 0xFFFF); }
inline uint16_t highWord(uint32_t ww) { return (uint16_t)((ww) >> 16); }

inline uint32_t makeDWord(uint32_t w) { return w; }
inline uint32_t makeDWord(uint16_t h, uint16_t l) { return (h << 16) | l; }

#define dword(...)            makeDWord(__VA_ARGS__)

#define READ_HOLDING_REGISTER 0x03
#define BUF_SIZE              (127)

static uint16_t crc16_update(uint16_t crc, uint8_t a)
{
    int i;
    crc ^= a;
    for (i = 0; i < 8; ++i)
    {
        if (crc & 1)
            crc = (crc >> 1) ^ 0xA001;
        else
            crc = (crc >> 1);
    }
    return crc;
}

class ModbusRTU
{
   private:
    int uartNum;
    uint8_t ADU[10];
    uint32_t timeout;
    bool bigEndian;
    bool swapRegs;
    int tx_enabled;

   public:
    static const uint8_t Status_OK                = 0;
    static const uint8_t Status_NotInitialized    = 0xF0;
    static const uint8_t Status_IncorrectSlaveID  = 0xF1;
    static const uint8_t Status_IncorrectFunction = 0xF2;
    static const uint8_t Status_ModbusException   = 0xF3;
    static const uint8_t Status_Timeout           = 0xF4;
    static const uint8_t Status_CRCError          = 0xF5;

    bool initialized;
    uint8_t status;  // estado de la transaccion (leer antes de usar el resultado!)

    ModbusRTU(int UartNum) : uartNum(UartNum), initialized(false) {}

    void Setup(int baud, int bits, int parity, int stops, int rx_pin, int tx_pin, int tx_enabled, bool bigEndian, uint32_t timeoutMs, bool swapRegs)
    {
        this->bigEndian  = bigEndian;
        this->timeout    = timeoutMs;
        this->swapRegs   = swapRegs;
        this->tx_enabled = tx_enabled;  // si es RS485, aca viene el pin de TX-ENABLE del modulo 485. Sino es -1.

        if (!uart_is_driver_installed(uartNum))
        {
            if (uart_driver_install(uartNum, BUF_SIZE * 2, 0, 0, NULL, 0) != ESP_OK)
            {
                Serial.println("Failed to install UART driver\n");
            }
        }

        // acomodo los seteos para el IDF:
        uart_word_length_t bts = (bits == 8 ? UART_DATA_8_BITS : (bits == 7 ? UART_DATA_7_BITS : (bits == 6 ? UART_DATA_6_BITS : UART_DATA_5_BITS)));
        uart_parity_t par      = (parity == 'N' ? UART_PARITY_DISABLE : (parity == 'E' ? UART_PARITY_EVEN : UART_PARITY_ODD));
        uart_stop_bits_t stpb  = (stops == 1 ? UART_STOP_BITS_1 : UART_STOP_BITS_2);

        Serial.printf("settings: %d %x %x %x\n", baud, bts, par, stpb);

        uart_config_t uart_config = {
            .baud_rate = baud,
            .data_bits = bts,
            .parity    = par,
            .stop_bits = stpb,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        };
        uart_param_config(uartNum, &uart_config);

        if (tx_enabled != -1)
        {
            pinMode(tx_enabled, OUTPUT);
            digitalWrite(tx_enabled, 0);
        }

        uart_set_pin(uartNum, tx_pin, rx_pin, -1, -1);
        initialized = true;
    }

    /*
        http://www.tolaemon.com/docs/modbus.htm#func_3_4
        Función 3 o 4 ( 3 Read Holding Registers – 4 Read Input Registers ) :

        Petición del máster (modo RTU):
            NºEsclavo                                    | 1 byte
            Código Operación: 0x03 o 0x04                | 1 byte
            Dirección del registro :                     | 2 bytes
            Nº de datos que se desea leer: max 128 datos | 2 bytes
            CRC(16): H L                                 | 2 bytes

        Respuesta del esclavo (modo RTU):
            NºEsclavo                      | 1 byte
            Código Operación: 0x03 o 0x04  | 1 byte
            Nº de bytes leidos:            | 1 byte
            Datos: ¿ max 128 datos ?       | ^ Nº de bytes leidos (2 * cant de registros)
            CRC(16): H L                   | 2 bytes
    */

    // solo puedo leer 1 o 2 registros y el resultado será de 32 bits.
    uint32_t ReadHoldingRegister(uint8_t slaveID, uint16_t regAddress, uint8_t cantReg)
    {
        if (!initialized)
        {
            Serial.println("No iniciado!");
            status = Status_NotInitialized;
            return 0xFFFFFFFF;
        }

        //-------------------------------
        // PETICION:

        ADU[0] = slaveID;
        ADU[1] = READ_HOLDING_REGISTER;
        ADU[2] = highByte(regAddress);
        ADU[3] = lowByte(regAddress);
        ADU[4] = highByte(cantReg);
        ADU[5] = lowByte(cantReg);
        // append CRC
        uint16_t u16CRC = 0xFFFF;
        for (int i = 0; i < 6; i++) u16CRC = crc16_update(u16CRC, ADU[i]);
        ADU[6] = lowByte(u16CRC);
        ADU[7] = highByte(u16CRC);

        if (tx_enabled != -1) digitalWrite(tx_enabled, 1);
        // uart_flush_input(uartNum);
        uart_write_bytes(uartNum, ADU, 8);
        uart_wait_tx_done(uartNum, 100);
        if (tx_enabled != -1) digitalWrite(tx_enabled, 0);

        //-------------------------------
        // RESPUESTA:

        uint8_t c, bytesRestan = 5, index = 0;
        uint32_t startTime = millis();

        status = Status_OK;

        // espero recibir durante unos milisegundos...
        while (bytesRestan && !status)
        {
            if (uart_read_bytes(uartNum, &c, 1, 20 / portTICK_PERIOD_MS) == 1)
            {
                ADU[index++] = c;
                bytesRestan--;
                // Serial.printf("%02X ", c);
            }
            if (index == 5)
            {
                if (ADU[0] != slaveID) status = Status_IncorrectSlaveID;                          // incorrect Modbus slave
                if ((ADU[1] & 0x7F) != READ_HOLDING_REGISTER) status = Status_IncorrectFunction;  // incorrect Modbus function code (mask exception bit 7)
                if (bitRead(ADU[1], 7)) status = Status_ModbusException;                          // Modbus exception occurred; return Modbus Exception Code (ADU[2] contiene el error)
                bytesRestan = ADU[2] & 0x7F;                                                      // cant de bytes de datos (2 * cant de registros) max 128
            }
            if ((millis() - startTime) > timeout) status = Status_Timeout;  // timeout
        }

        //-------------------------------
        // check CRC
        if (!status && index >= 5)
        {
            uint16_t u16CRC = 0xFFFF;
            for (int i = 0; i < (index - 2); i++) u16CRC = crc16_update(u16CRC, ADU[i]);
            if (lowByte(u16CRC) != ADU[index - 2] || highByte(u16CRC) != ADU[index - 1]) status = Status_CRCError;
        }

        //----------------------------------------
        // Solo si está OK, obtengo el resultado:

        uint32_t result = 0xFFFFFFFF;

        if (status == 0)
        {
            // resultado crudo de 32 bits (puede ser short,long o float) se castea en otro lado.
            if (cantReg == 1)
            {
                result = bigEndian ? word(ADU[4], ADU[3]) : word(ADU[3], ADU[4]);
            }
            else
            {
                uint16_t H = bigEndian ? word(ADU[4], ADU[3]) : word(ADU[3], ADU[4]);
                uint16_t L = bigEndian ? word(ADU[6], ADU[5]) : word(ADU[5], ADU[6]);
                result     = swapRegs ? dword(H, L) : dword(L, H);
            }
            // Serial.printf("Modbus read ok:  regAddress:%d   Value: (%d)  [%04X][%04X]  (%f)\n", regAddress, result, highWord(result), lowWord(result), (*(float*)&result));
        }

        // OJO! verificar que el status sea 0 para usar el resultado!
        return result;
    }
};
