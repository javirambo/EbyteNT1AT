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

#pragma once
#include <Arduino.h>

class EbyteNT1AT
{
   private:
    Stream* serial;

    // Lee por ejemplo el OK:
    //  "\r\n+OK\r\n" o "\r\n+OK=AT enable\r\n"
    String ReadAT()
    {
        String s = serial->readStringUntil('\n');
        if (s.length() <= 2) s = serial->readStringUntil('\n');
        s.replace("\r", "");
        return s;
    }

    // vacia el buffer de Rx y Tx
    void Flush()
    {
        serial->flush();
        while (serial->read() != -1)  //
            ;
    }

   public:
    void Begin(Stream& serial) { this->serial = &serial; }

    // 1.2 Enter AT Commands
    bool GoIntoAT()
    {
        Flush();

        serial->print("+++");
        delay(100);
        serial->print("AT");
        // delay(100);

        // espero el OK => o \r\n+OK\r\n o \r\n+OK=AT enable\r\n
        String s = ReadAT();
        return strstr(s.c_str(), "+OK");
    }

    // envia el comando AT y retorna el resultado
    String SendAT(String cmd)
    {
        Flush();

        serial->println(cmd);
        // delay(10);
        return ReadAT();
    }

    //------------------------------------------------------------
    // 1 “The Basic Functions” AT Command
    //------------------------------------------------------------

    // 1.3 Exit AT Commands
    String ExitAT() { return SendAT("AT+EXAT"); }  //

    // 1.4 Query Model
    String QueryModel() { return SendAT("AT+MODEL"); }  // NT1-B

    // 1.5 Query / Set Name
    String QueryName() { return SendAT("AT+NAME"); }                   // admin
    String SetName(String name) { return SendAT("AT+NAME=" + name); }  //

    // 1.6 Query / Set ID
    String QueryID() { return SendAT("AT+SN"); }                   // S3203101S
    String SetID(String name) { return SendAT("AT+SN=" + name); }  //

    // 1.7 Restart
    String Restart() { return SendAT("AT+REBT"); }  //

    // 1.8 Factory Data Reset
    String FactoryDataReset() { return SendAT("AT+RESTORE"); }  //

    // 1.9 Query Version Information
    String QueryVersion() { return SendAT("AT+VER"); }  // 9013-8-16

    // 1.10 Query / Set up the serial port
    String QuerySerialPort() { return SendAT("AT+UART"); }                                   // 115200,8,1,NONE,XON/XOFF
    String SetSerialPort(String baud, String data, String stop, String parity, String flow)  //
    {
        return SendAT("AT+UART=" + baud + "," + data + "," + stop + "," + parity + "," + flow);
    }

    // 1.11 Query the MAC address
    String QueryMAC() { return SendAT("AT+MAC"); }  // 54-14-A7-86-DF-21

    // 1.12 Query / Set the network parameters
    String QueryNetwork() { return SendAT("AT+WAN"); }                                       // STATIC,192.168.3.7,255.255.255.0,192.168.3.1,114.114.114.114
    String SetNetwork(String mode, String address, String mask, String gateway, String dns)  //
    {
        return SendAT("AT+WAN=" + mode + "," + address + "," + mask + "," + gateway + "," + dns);
    }

    // 1.13 Query / set the local port number
    String QueryLocalPort() { return SendAT("AT+LPORT"); }                   // 8883
    String SetLocalPort(String port) { return SendAT("AT+LPORT=" + port); }  //

    // 1.14 Query / Set the local working mode and the target equipment
    //    Model (working mode): TCPC, TCPS, UDPC, UDPS, MQTTC, HTTPC
    //    Remote IP (target IP / domain name): a maximum of 128-character domain name
    //    Remote Port (target port): 1-65535
    String QueryNetProtocol() { return SendAT("AT+SOCK"); }  // TCPC,192.168.3.3,8888
    String SetNetProtocol(String mode, String remoteIP, String remotePort) { return SendAT("AT+SOCK=" + mode + "," + remoteIP + "," + remotePort); }

    // 1.15 Query the network link status
    String QueryLinkStatus() { return SendAT("AT+LINKSTA"); }  //

    // 1.16 Query/Set the serial port cache cleanup status
    //   ON=enable connection empty cache
    //   OFF=disable the connection to empty the cache.
    String QueryCleaningState() { return SendAT("AT+UARTCLR"); }  //
    String SetCleaningState(String status) { return SendAT("AT+UARTCLR=" + status); }

    // 1.17 Query/Set the registration package mode
    //   Status: OFF-Disable
    //   OLMAC-First connection Send MAC
    //   OLCSTM-First Connection Send Custom
    //   EMBMAC-Send a MAC per package
    //   EMBCSTM-Send a custom per package
    String QueryRegistrationMode() { return SendAT("AT+REGMOD"); }  //
    static const uint8_t PackageModeDisabled             = 0;
    static const uint8_t PackageModeFirstSendMAC         = 1;
    static const uint8_t PackageModeFirstSendCustom      = 2;
    static const uint8_t PackageModeSendMAcPerPackage    = 3;
    static const uint8_t PackageModeSendCustomPerPackage = 4;
    String SetRegistrationMode(uint8_t mode)
    {
        switch (mode)
        {
            case PackageModeDisabled:
                return SendAT("AT+REGMOD=OFF");
            case PackageModeFirstSendMAC:
                return SendAT("AT+REGMOD=OLMAC");
            case PackageModeFirstSendCustom:
                return SendAT("AT+REGMOD=OLCSTM");
            case PackageModeSendMAcPerPackage:
                return SendAT("AT+REGMOD=EMBMAC");
            case PackageModeSendCustomPerPackage:
                return SendAT("AT+REGMOD=EMBCSTM");
            default:
                return "Error: Invalid package mode.";
        }
    }

    // 1.18 Query / Set up the custom registration package content
    //   Mode: Data Format (HEX) 16 decimal system, (STR) string.
    //   Data data: ASCII limit 40 bytes, HEX limit 20 bytes.
    String QueryCustomRegistration() { return SendAT("AT+REGINFO"); }  // Received: \r\n+OK=STR,regist msg\r\n
    static const uint8_t DataFormatHEX = 0;
    static const uint8_t DataFormatSTR = 0;
    String SetCustomRegistration(uint8_t mode, String data)
    {
        String sMode;
        if (mode == DataFormatHEX)
            sMode = "HEX";
        else
            sMode = "STR";
        return SendAT("AT+REGINFO=" + sMode + "," + data);
    }

    // 1.19 Query / Set the heartbeat packet mode
    //   Mode: NONE (off), UART (serial port heartbeat), NET (network heartbeat)
    //   Time: Time 0-65535s, 0 (close the heartbeat)
    String QueryHeartbeat() { return SendAT("AT+HEARTMOD"); }  //
    String SetHeartbeat(String mode, String time) { return SendAT("AT+HEARTMOD=" + mode + "," + time); }

    // 1.20 Query / Set up the heartbeat data
    // Mode: Data Format (H EX) 16 decimal system, (S TR) string
    // Data data: ASCII limit 40 bytes, HEX limit 20 bytes
    String QueryHeartbeatData() { return SendAT("AT+HEARTINFO"); }  //
    String SetHeartbeatData(String mode, String data) { return SendAT("AT+HEARTINFO=" + mode + "," + data); }

    // 1.21 Query / Set the short connection time
    //   Time: limit of 2-255s, 0 is closed
    String QueryShortConnection() { return SendAT("AT+SHORTM"); }  //
    String SetShortConnection(String time) { return SendAT("AT+SHORTM=" + time); }

    // 1.22 Query / Set the timeout restart time
    //  Time: 60-65535s, 0 for closed
    String QueryRestartTimeout() { return SendAT("AT+TMORST"); }  //
    String SetRestartTimeout(String time) { return SendAT("AT+TMORST=" + time); }

    // 1.23 Query / Set the time and number of disconnected reconnections
    //  Times (disconnection reconnection time): limit 1-255,0 is closed;
    //  Num (number of reconnection): 1-60 times;
    String QueryReconnection() { return SendAT("AT+TMOLINK"); }  //
    String SetReconnection(String times, String num) { return SendAT("AT+TMOLINK=" + times + "," + num); }

    // 1.24 Web configuration port
    //  PORT: 2-65535
    String QueryWebPort() { return SendAT("AT+WEBCFGPORT"); }  //
    String SetWebPort(String port) { return SendAT("AT+WEBCFGPORT=" + port); }

    //------------------------------------------------------------
    // 2 The Modbus Functions AT Commands
    //------------------------------------------------------------

    // 2.2 Query the Modbus mode, and the commands timeout time
    //   Mode: NONE (disable MODBUS)
    //   SIMPL (simple protocol conversion)
    //   MULIT (Multihost mode)
    //   STORE (storage-type gateway)
    //   CONFIG (a configurable gateway)
    //   AUTOUP (active upload mode)
    //   Timeout :0-65535；

    String QueryModbusMode() { return SendAT("AT+MODWKMOD"); }

    static const uint8_t ModbusDisabled                     = 0;
    static const uint8_t ModbusModeSimpleProtocolConvertion = 1;
    static const uint8_t ModbusModeMultihostMode            = 2;
    static const uint8_t ModbusModeStorageType              = 3;
    static const uint8_t ModbusModeConfigurableGateway      = 4;
    static const uint8_t ModbusModeActiveUploadMode         = 5;
    String SetModbusMode(uint8_t mode, String timeout)
    {
        String sMode;
        switch (mode)
        {
            case ModbusDisabled:
                return SendAT("AT+MODWKMOD=NONE," + timeout);
            case ModbusModeSimpleProtocolConvertion:
                return SendAT("AT+MODWKMOD=SIMPL," + timeout);
            case ModbusModeMultihostMode:
                return SendAT("AT+MODWKMOD=MULIT," + timeout);
            case ModbusModeStorageType:
                return SendAT("AT+MODWKMOD=STORE," + timeout);
            case ModbusModeConfigurableGateway:
                return SendAT("AT+MODWKMOD=CONFIG," + timeout);
            case ModbusModeActiveUploadMode:
                return SendAT("AT+MODWKMOD=AUTOUP," + timeout);
            default:
                return "Error: unknown mode";
        }
    }

    // 2.3 Turn on the Modbus TCP to Modbus RTU protocol conversion
    // Mode: ON  (Enable protocol conversion)
    //       OFF (disable protocol conversion)
    String QueryModbusProtocolConvertion() { return SendAT("AT+MODPTCL"); }
    String EnableModbusProtocolConvertion() { return SendAT("AT+MODPTCL=ON"); }
    String DisableModbusProtocolConvertion() { return SendAT("AT+MODPTCL=OFF"); }

    // 2.4 Set the Modbus Gateway instruction storage time and the automatic query interval
    // Time 1: instruction storage time (1-255 seconds)
    // Time 2: Automatic query interval time (1-65,535 ms)
    String QueryModbusGatewayTimes() { return SendAT("AT+MODGTWYTM"); }
    String SetModbusGatewayTimes(String time1, String time2) { return SendAT("AT+MODGTWYTM=" + time1 + "," + time2); }

    // 2.5 Modbus configuration gateway prememory instruction query and edit
    // Mode:
    //  ADD add instruction;
    //  DEL delete instruction;
    //  CLR emptying instruction;
    //  CMD: Modbus instruction (only support the standard Modbus RTU instruction, do not need to fill
    //       in the verification, only can configure the read instruction 01,02,03,04 function code),
    //       and can not store the same instruction whether to return +ERR= -4;
    String QueryModbusGateway() { return SendAT("AT+MODCMDEDIT"); }
    String AddModbusGateway(String cmd) { return SendAT("AT+MODCMDEDIT=ADD," + cmd); }
    String DelModbusGateway(String cmd) { return SendAT("AT+MODCMDEDIT=DEL," + cmd); }
    String ClearModbusGateway(String cmd) { return SendAT("AT+MODCMDEDIT=CLR," + cmd); }

    //------------------------------------------------------------
    // 3 The "Internet of Things Function" AT Commands
    //------------------------------------------------------------

    // 3.3 Query / Set the HTTP request mode
    // Method: GET\POST
    String QueryHttpMode() { return SendAT("AT+HTPREQMODE"); }
    String SetHttpMode(String method) { return SendAT("AT+HTPREQMODE=" + method); }

    // 3.4 Query / Set the HTTP URL path
    //   Path: HTTP Request U RL Resource Address (Length limit of 0-128 characters)
    String QueryHttpURL() { return SendAT("AT+HTPURL"); }
    String SetHttpURL(String path) { return SendAT("AT+HTPURL=" + path); }

    // 3.5 Query / Set HTTP Packet Header
    //   Para (whether HTTP returns the serial port data with a header):
    //     DEL: No Baotou
    //     ADD: with Baotou
    //   Head (HTTP Request Ptou): Length limit 128 characters
    String QueryHttpHeader() { return SendAT("AT+HTPHEAD"); }
    String AddHttpHeader(String head) { return SendAT("AT+HTPHEAD=ADD," + head); }
    String DelHttpHeader(String head) { return SendAT("AT+HTPHEAD=DEL," + head); }

    // 3.6 Query / Set up the MQTT target platform
    //  Server (MQTT Target Platform):
    //  STANDARD (MQTT3.1.1 standard protocol server)
    //  ONENET (O ne NET-MQTT server)
    //  ALI (AliCloud M QTT Server)
    //  BAIDU (Baidu Cloud M QTT Server)
    //  HUAWEI (Huawei Cloud M QTT Server)
    static const uint8_t StandarMqttServer = 0;
    static const uint8_t OneNetMqttServer  = 1;
    static const uint8_t AliMqttServer     = 2;
    static const uint8_t BaiduMqttServer   = 3;
    static const uint8_t HuaweiMqttServer  = 4;
    String QueryMQTTServer() { return SendAT("AT+MQTTCLOUD"); }
    String SetMQTTServer(uint8_t server)
    {
        switch (server)
        {
            case StandarMqttServer:
                return SendAT("AT+MQTTCLOUD=STANDARD");
            case OneNetMqttServer:
                return SendAT("AT+MQTTCLOUD=ONENET");
            case AliMqttServer:
                return SendAT("AT+MQTTCLOUD=ALI");
            case BaiduMqttServer:
                return SendAT("AT+MQTTCLOUD=BAIDU");
            case HuaweiMqttServer:
                return SendAT("AT+MQTTCLOUD=HUAWEI");
            default:
                return "Error unknown server";
        }
    }

    // 3.7 Query / Set the MQTT Active Packet Header Package Delivery Cycle
    // Time: MQTT active heartbeat time (limited to 1-255 seconds, default to 60s, modification is not recommended);
    String QueryMQTTPacket() { return SendAT("AT+QTKPALIVE"); }
    String SetMQTTPacket(String time) { return SendAT("AT+QTKPALIVE=" + time); }

    // 3.8 Query / Set the MQTT Device Name (Client ID)
    // Client ID: The MQTT Device Name (C lient ID) limits a length of 128 characters;
    String QueryMQTTClientID() { return SendAT("AT+MQTDEVID"); }
    String SetMQTTClientID(String clientID) { return SendAT("AT+MQTDEVID=" + clientID); }

    // 3.9 Query / Set the MQTT Username (User Name / Device Name)
    // User Name: MQTT product I D (User N ame / device name) limit length of 128 characters;
    String QueryMQTTUserName() { return SendAT("AT+MQTUSER"); }
    String SetMQTTUserName(String user) { return SendAT("AT+MQTUSER=" + user); }

    // 3.10 Query / Set the MQTT Password (MQTT Password / Device Secret)
    //  Password: MQTT Login password (MQTT P assword / Device Secret) length limit of 128 characters;
    String QueryMQTTPassword() { return SendAT("AT+MQTPASS"); }
    String SetMQTTPassword(String password) { return SendAT("AT+MQTPASS=" + password); }

    // 3.11 Query / Set the Product Key of AliCloud MQTT
    // Product Key: Product Ke y (64 characters)
    String QueryMQTTProductKeyForAliCloud() { return SendAT("AT+MQTTPRDKEY"); }
    String SetMQTTProductKeyForAliCloud(String password) { return SendAT("AT+MQTTPRDKEY=" + password); }

    // 3.12 Query / Set the MQTT subscription topic
    // Qos: Only levels 0,1 are supported;
    // Topic: MQTT subscription theme (length limit of 128 characters)
    String QueryMQTTSubTopic() { return SendAT("AT+MQTSUB"); }
    String SetMQTTSubTopic(uint8_t QoS, String topic) { return SendAT("AT+MQTSUB=" + String(QoS) + "," + topic); }

    // 3.13 Query / Set the MQTT release topic
    // Qos: Only levels 0,1 are supported;
    // Topic: MQTT release theme (128 characters)
    String QueryMQTTReleaseTopic() { return SendAT("AT+MQTPUB"); }
    String SetMQTTReleaseTopic(uint8_t QoS, String topic) { return SendAT("AT+MQTPUB=" + String(QoS) + "," + topic); }
};