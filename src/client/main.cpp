// Feather9x_TX
// -*- mode: C++ -*-

#include <SPI.h>
#include <RH_RF95.h>

//for feather32u4
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup()
{
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);

    Serial.begin(115200);

    delay(100);

    Serial.println("Feather LoRa TX Test!");

    // manual reset
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);

    while (!rf95.init())
    {
        Serial.println("LoRa radio init failed");
        while (1)
            ;
    }
    Serial.println("LoRa radio init OK!");

    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
    if (!rf95.setFrequency(RF95_FREQ))
    {
        Serial.println("setFrequency failed");
        while (1)
            ;
    }
    Serial.print("Set Freq to: ");
    Serial.println(RF95_FREQ);

    // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

    // The default transmitter power is 13dBm, using PA_BOOST.
    // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
    // you can set transmitter powers from 5 to 23 dBm:
    rf95.setTxPower(23, false);
}

int16_t packetnum = 0; // packet counter, we increment per xmission

void loop()
{

    if (!Serial)
    {
        delay(1000);                       // Wait 1 second between transmits, could also 'sleep' here!
        Serial.println("Transmitting..."); // Send a message to rf95_server

        char radiopacket[20] = "Hello World #      ";
        itoa(packetnum++, radiopacket + 13, 10);
        Serial.print("Sending ");
        Serial.println(radiopacket);
        radiopacket[19] = 0;

        Serial.println("Sending...");
        delay(10);
        rf95.send((uint8_t *)radiopacket, 20);

        Serial.println("Waiting for packet to complete...");
        delay(10);
        rf95.waitPacketSent();
    }
    else
    {

        if (Serial.available())
        {
            String in = Serial.readString();
            Serial.println("Reading:" + in);
            if (in.startsWith("Set"))
            {
                Serial.println(rf95.printRegisters());
                rf95.setModemRegisters({0x72, 0x74, 0x04})
            }
        }
        // Now wait for a reply
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t len = sizeof(buf);

        Serial.println("Waiting for reply...");
        if (rf95.waitAvailableTimeout(2000))
        {
            // Should be a reply message for us now
            if (rf95.recv(buf, &len))
            {
                Serial.print("Got reply: ");
                Serial.println((char *)buf);
                Serial.print("RSSI: ");
                Serial.println(rf95.lastRssi(), DEC);
            }
            else
            {
                Serial.println("Receive failed");
            }
        }
        else
        {
            Serial.println("No reply, is there a listener around?");
        }
    }
}