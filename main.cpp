/*
    Project/Script: ESP8266-01S Basic Connection
    Version: 1.1
    Hardware: NUCLEO L152RE und ESP826-01S
*/
#include "mbed.h"

#include "ESP8266Interface.h"
ESP8266Interface wifi(PC_12, PD_2);

#include "TCPSocket.h"
SocketAddress socket_address;
TCPSocket tcp_socket;

#define SLEEP_TIME 100ms

DigitalIn t1(BUTTON1);
const int PRESSED = 0;
bool wlan_connected = false;

// development/ debugging
#define LOG_DEBUG print_message_to_serial
// production, no console output
//#define LOG_DEBUG no_nothing


void no_nothing(const char* message_with_args="", ...){ return; }
void print_message_to_serial(const char* message_with_args="", ...) {
    va_list arg;
    va_start(arg, message_with_args);
    char message[250];
    vsprintf(message, message_with_args, arg);
    va_end(arg);
    printf("%s\n", message);
}


int connect_to_wifi() {
    // derzeitige Status des WLAN-Verbindung abfragen
    nsapi_connection_status wifi_status = wifi.get_connection_status();

    int status = -1;

    // wenn WLAN ist schon verbunden, dann nichts tun, ggf. dei Meldung an Serial schicken
    if (wifi_status == NSAPI_STATUS_LOCAL_UP || wifi_status == NSAPI_STATUS_GLOBAL_UP) {
        LOG_DEBUG("Already connected to Wifi");
        status = 0;
    } else{ // Wenn WLAN war nicht verbunden, dann versuche eine Verbindung herzustellen
        status = wifi.connect( // FEHLER in die nächste Zeilen ignorieren
            MBED_CONF_NSAPI_DEFAULT_WIFI_SSID, // WLAN-AP aus mbed_app.json
            MBED_CONF_NSAPI_DEFAULT_WIFI_PASSWORD, // WLAN-Passwort mbed_app.json
            NSAPI_SECURITY_WPA_WPA2 // WLAN-Sicherheit aus mbed_app.json
            );

        if(status == 0) { // eine Status '0' bedeutet, die Verbindung ist erfolgreicht hergestellt
            LOG_DEBUG("Connected to Wifi Network Successfully.");
            wifi.get_ip_address(&socket_address);
            LOG_DEBUG("IP: %s", socket_address.get_ip_address() );
            wifi.set_as_default(); // die WLAN-Verbindung als Standard Verbindungsmethode setzen
        }else{ // WLAN Verbindung war nicht erfolgreich
            LOG_DEBUG("wifi connection error: %d", status);
        }
    }
    return status;
}

int disconnect_from_wifi() {
    nsapi_connection_status wifi_status = wifi.get_connection_status();
    if (wifi_status == NSAPI_STATUS_LOCAL_UP || wifi_status == NSAPI_STATUS_GLOBAL_UP) {
        wifi.disconnect();
        LOG_DEBUG("Wifi disconnected");
    } else{
        LOG_DEBUG("Wifi doesn't seem to be connected, why disconnect?");
    }
    
    return 0;
}


// main() runs in its own thread in the OS
int main()
{

    LOG_DEBUG("Ready, press Blue Button L152RE to connect to WIFI");

    while (true) {

        if(t1 == PRESSED){
            while(t1 == PRESSED){}

            if(wlan_connected) { // wenn wlan ist verbunden, dann die verbindung trennen
                LOG_DEBUG("Button pressed! will now try to disconnect from WIFI");
                disconnect_from_wifi();
                wlan_connected = !wlan_connected;
            } else { // wenn wlan ist nicht verbunden, versuche die Verbindung herzustellen
                LOG_DEBUG("Button pressed! will now try to connect to WIFI");
                if( connect_to_wifi() == 0) { // wenn Verbindung erfolgreich war, die Meldung anzeigen
                    wlan_connected = !wlan_connected;
                    LOG_DEBUG("You can Press the Button again to disconnect from WIFI");
                }
            } // if(wlan_connected)
        } // if(t1 == PRESSED)

        ThisThread::sleep_for(SLEEP_TIME);
    }
}

