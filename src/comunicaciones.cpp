//
// Created by sait1 on 02/05/2026.
//

#include "comunicaciones.h"
#include "motor_servos.h"
#include "GLOBALES.h"
#include "sensores.h"
#include "diagnostico.h"

#include <WebServer.h>
#include "time.h"
#include <WiFi.h>
#include <HTTPClient.h>

Preferences preferences;

const char* ssid = "Matamoros";
const char* password = "";
const char* ntpServer = "pool.ntp.org";

void configurarReloj() {
    configTime(-21600, 0, ntpServer);
}

String obtenerFechaHora() {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){

        registrar_error(ERROR_TIEMPO,"");
        return "Error al obtener tiempo";

    }
    char buffer[30];
    // Formato: Año-Mes-Día Hora:Min
    strftime(buffer, 30, "%Y-%m-%d %H:%M", &timeinfo);
    return String(buffer);
}

void conectar_wifi() {


    preferences.begin("wifi-config", true); // Abre en modo lectura (true)

    String memorizadoSSID = preferences.getString("ssid", ssid);
    String memorizadoPass = preferences.getString("pass", password);
    preferences.end();

    Serial.print("Intentando conectar a: ");
    Serial.println(memorizadoSSID);

    WiFi.setTxPower(WIFI_POWER_8_5dBm); // Reduce potencia = menos pico de corriente
    WiFi.begin(memorizadoSSID.c_str(), memorizadoPass.c_str());

    int i=0;
    unsigned long tiempoInicioWiFi = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - tiempoInicioWiFi < 12000) {
        delay(500);
        Serial.print(".");

        Luz_carga(i++);
    }

    Serial.print("Estado de WiFi: ");
    switch (WiFi.status()) {
        case WL_IDLE_STATUS:     Serial.println("Inactivo"); break;
        case WL_NO_SSID_AVAIL:   Serial.println("Red no encontrada (Revisa 2.4GHz)"); break;
        case WL_CONNECT_FAILED:  Serial.println("Contraseña incorrecta"); break;
        case WL_DISCONNECTED:    Serial.println("Desconectado"); break;
        case WL_CONNECTED:       Serial.println("¡CONECTADO!"); break;
        default:                 Serial.println("Buscando..."); break;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConectado! IP: " + WiFi.localIP().toString());
        configurarReloj();
        server.begin(); // Solo inicia el servidor si hay WiFi
        configurarRutas();
    } else {
        registrar_error(ERROR_CONECCION_WIFI,"\nNo se encontro WiFi. Trabajando en modo OFFLINE.");
        sonar_alarma(2, 200);
    }
}

void guardar_wiFi(String s, String p) {
    preferences.begin("wifi-config", false); // Abre el "archivo" wifi-config
    preferences.putString("ssid", s);        // Guarda el nombre de red
    preferences.putString("pass", p);        // Guarda la contraseña
    preferences.end();                       // Cierra y asegura los datos
    Serial.println("Datos guardados en memoria permanente.");
}

void configurar_wifi_manual() {
    controlarCinta(PARAR);
    while(Serial.available() > 0) Serial.read();

    Serial.println("\n--- CONFIGURACIÓN DE NUEVA RED ---");
    Serial.println("Escribe el nombre de la red (SSID) y presiona Enter:");

    //Espera real hasta que el usuario escriba algo
    while (!Serial.available()) { delay(100); }
    String tempSSID = Serial.readStringUntil('\n');
    tempSSID.trim();

    Serial.println("SSID recibido: " + tempSSID); // Confirmación
    Serial.println("Escribe la contraseña y presiona Enter:");

    delay(500);
    while(Serial.available() > 0) Serial.read();

    //Volvemos a esperar para la contraseña
    while (!Serial.available()) { delay(100); }
    String tempPass = Serial.readStringUntil('\n');
    tempPass.trim();

    Serial.println("Cotraseña recibida: " + tempPass);
    Serial.print("Intentando conectar a: "); Serial.println(tempSSID);
    WiFi.disconnect(); // Corta la conexión actual
    WiFi.begin(tempSSID.c_str(), tempPass.c_str());

    Serial.print("Validando");
    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 15000) {
        delay(500);
        Serial.print(".");
        efecto_respirar(0,255,255);
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n¡Conexión Exitosa!");
        guardar_wiFi(tempSSID, tempPass);
        Serial.println("IP: " + WiFi.localIP().toString());
        configurarReloj();
        server.begin();
        configurarRutas();
    } else {
        registrar_error(ERROR_DATOS_WIFI_INCORRECTOS,"\nError: Datos incorrectos. No se guardó nada.");
        sonar_alarma(2, 200);
        conectar_wifi();
    }
}

void obtenerValores() {
    Serial.println("\n--- REPORTE DE PAQUETE ---");
    Serial.print("Tiempo de entrada: "); Serial.print(tiempo_entrada);                     Serial.println(" ms");
    Serial.print("Tiempo de salida:  "); Serial.print(tiempo_entrada+tiempo_en_pasar);     Serial.println(" ms");
    Serial.print("Tiempo total:   "); Serial.print(tiempo_en_pasar);          Serial.println(" ms");
    Serial.println("--------------------------");
    Serial.print("Grosor máximo: "); Serial.print(grosor_maximo);                   Serial.println(" mm");
    Serial.print("Largo total: "); Serial.print(tamanio);                           Serial.println(" mm");
    Serial.print("Peso total: "); Serial.print(peso);                               Serial.println(" g");
    Serial.println("--------------------------");
    Serial.print("Voltaje de 5V: ");      Serial.print(voltaje_5V);                  Serial.println(" V");
    Serial.print("Corriente de 5V: ");    Serial.print(corriente_5V ,4);             Serial.println(" A");
    Serial.print("Resistencia de 5V: ");  Serial.print(resistencia_5V);              Serial.println(" R");
    Serial.print("Voltaje de 12V: ");      Serial.print(voltaje_12V);                Serial.println(" V");
    Serial.print("Corriente de 12V: ");    Serial.print(corriente_12V ,4);           Serial.println(" A");
    Serial.print("Resistencia de 12V: ");  Serial.print(resistencia_12V);            Serial.println(" R");
    Serial.print("Amperaje de el motor: "); Serial.print(motor_mA);                  Serial.println(" mA");
    Serial.println("--------------------------");
    Serial.print("Galletas chicas: "); Serial.println(contador_chicas);
    Serial.print("Galletas medianas: ");  Serial.println(contador_medianas);
    Serial.print("Galletas grandes: ");  Serial.println(contador_grandes);
    Serial.print("No validas: ");        Serial.println(no_validas);
    Serial.println("--------------------------");
    Serial.print("Fecha: ");        Serial.println(obtenerFechaHora());
    Serial.println();

}

String generarJSON() {
    char buffer[512]; // Buffer fijo en el stack, sin fragmentación de heap

    snprintf(buffer, sizeof(buffer),
        "{"
        "\"tiempo_entrada\":%lu,"
        "\"tiempo_salida\":%lu,"
        "\"tiempo_total\":%lu,"
        "\"grosor_max\":%.2f,"
        "\"largo\":%.2f,"
        "\"peso\":%.2f,"
        "\"voltaje_5v\":%.3f,"
        "\"corriente_5v\":%.4f,"
        "\"resistencia_5v\":%.2f,"
        "\"voltaje_12v\":%.3f,"
        "\"corriente_12v\":%.4f,"
        "\"resistencia_12v\":%.2f,"
        "\"motor_mA\":%.2f,"
        "\"galletas_chicas\":%d,"
        "\"galletas_medianas\":%d,"
        "\"galletas_grandes\":%d,"
        "\"no_validas\":%d,"
        "\"categoria\":\"%s\","
        "\"fecha\":\"%s\""
        "}",
        // Tiempos
        tiempo_entrada,
        tiempo_entrada + tiempo_en_pasar,
        tiempo_en_pasar,
        // Dimensiones
        grosor_maximo,
        tamanio,
        peso,
        // Telemetría 5V
        voltaje_5V,
        corriente_5V,
        resistencia_5V,
        // Telemetría 12V
        voltaje_12V,
        corriente_12V,
        resistencia_12V,
        // Motor
        motor_mA,
        // Contadores
        contador_chicas,
        contador_medianas,
        contador_grandes,
        no_validas,
        // Categoría y fecha
        categoria_actual.c_str(),
        obtenerFechaHora().c_str()
    );

    return String(buffer); // Solo UNA String al final
}


void enviar_a_base_de_datos() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        // La IP del Servidor Web
        http.begin("http://190.198.200");
        http.addHeader("Content-Type", "application/json");

        String datosJSON = generarJSON();
        int httpResponseCode = http.POST(datosJSON);

        if (httpResponseCode > 0) {
            Serial.print("Dato guardado en Servidor: ");
            Serial.println(httpResponseCode);
            color_led(3, tira.Color(0,0,255));
            serverActivo = true;
        } else {
            registrar_error(ERROR_ENVIAR_DATOS_SERVIDOR,"Error al enviar bace de datos: ");
            Serial.println(httpResponseCode);
            color_led(3, tira.Color(255,0,0));
            sonar_alarma(1, 200);
            serverActivo = false;
        }
        http.end();
    }
    else {
        color_punto_led(3, tira.Color(255,0,0));
    }
}

void enviar_error_a_base_de_datos(int codigo, String mensaje) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        // Dirección de tu servidor o API donde reside la base de datos
        http.begin("http://tu-servidor-api.com/errores"); // Asegúrate de apuntar al endpoint correcto
        http.addHeader("Content-Type", "application/json");

        // Usamos snprintf para armar el JSON de forma segura sin saturar la RAM
        char buffer[384];
        snprintf(buffer, sizeof(buffer),
            "{"
            "\"codigo_error\":%d,"
            "\"desc_error\":\"%s\""
            "}",
            codigo,
            mensaje.c_str()
        );

        // Enviamos la petición POST real
        int httpResponseCode = http.POST(String(buffer));

        if (httpResponseCode > 0) {
            Serial.print("-> Alerta de falla enviada. Servidor responde: ");
            Serial.println(httpResponseCode);
            serverActivo = true;
        } else {
            Serial.print("-> Error de enlace con el servidor de planta: ");
            Serial.println(http.errorToString(httpResponseCode).c_str());
            serverActivo = false;
        }
        http.end();
    }
}

void configurarRutas() {
    // Definimos una función lambda auxiliar para inyectar CORS de golpe a cualquier respuesta
    auto enviarCORS = []() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    };

    // --- RUTA COMPLETA DE TELEMETRÍA (JSON) ---
    server.on("/datos", HTTP_GET, [enviarCORS]() {
        obtenerValores();
        enviarCORS();
        server.send(200, "application/json", generarJSON());
    });

    // --- CONTROLES OPERATIVOS ---
    server.on("/encender", HTTP_GET, [enviarCORS]() {
        sistemaActivo = true;
        controlarCinta(ADELANTE);
        enviarCORS();
        server.send(200, "text/plain", "Sistema Activo");
    });

    server.on("/detener", HTTP_GET, [enviarCORS]() {
        sistemaActivo = false;
        controlarCinta(PARAR);
        enviarCORS();
        server.send(200, "text/plain", "Sistema Detenido");
    });

    server.on("/avanzar_motor", HTTP_GET, [enviarCORS]() {
        controlarCinta(ADELANTE);
        enviarCORS();
        server.send(200, "text/plain", "Motor Avanzando");
    });

    server.on("/detener_motor", HTTP_GET, [enviarCORS]() {
        controlarCinta(PARAR);
        enviarCORS();
        server.send(200, "text/plain", "Motor Detenido");
    });

    server.on("/retroceder_motor", HTTP_GET, [enviarCORS]() {
        controlarCinta(ATRAS);
        enviarCORS();
        server.send(200, "text/plain", "Motor Retrocediendo");
    });

    // --- ACTUADORES (SERVO BRAZOS) ---
    server.on("/brazo1", HTTP_GET, [enviarCORS]() {
        controlarBrazo(PRIMER_BRAZO);
        enviarCORS();
        server.send(200, "text/plain", "Brazo 1 probado");
    });

    server.on("/brazo2", HTTP_GET, [enviarCORS]() {
        controlarBrazo(SEGUNDO_BRAZO);
        enviarCORS();
        server.send(200, "text/plain", "Brazo 2 probado");
    });

    server.on("/brazo3", HTTP_GET, [enviarCORS]() {
        controlarBrazo(TERCER_BRAZO);
        enviarCORS();
        server.send(200, "text/plain", "Brazo 3 probado");
    });

    // --- SENSORES Y PERIFÉRICOS ---
    server.on("/tarar", HTTP_GET, [enviarCORS]() {
        bascula.tare();
        enviarCORS();
        server.send(200, "text/plain", "Bascula puesta en 0");
    });

    server.on("/obtener_peso", HTTP_GET, [enviarCORS]() {
        float p = obtenerPeso();
        enviarCORS();
        server.send(200, "text/plain", String(p) + " g");
    });

    server.on("/medir_distancia", HTTP_GET, [enviarCORS]() {
        uint16_t dist = usar_laser();
        enviarCORS();
        server.send(200, "text/plain", "Distancia: " + String(dist) + " mm");
    });

    server.on("/amperaje", HTTP_GET, [enviarCORS]() {
        enviarCORS();
        server.send(200, "text/plain", "Amperaje de el motor: " + String(motor_mA) + " mA");
    });

    server.on("/fecha", HTTP_GET, [enviarCORS]() {
        enviarCORS();
        server.send(200, "text/plain", "Fecha: " + obtenerFechaHora());
    });

    // --- CONFIGURACIÓN DE RED ---
    server.on("/conectar_wifi", HTTP_GET, [enviarCORS]() {
        conectar_wifi();
        enviarCORS();
        if (WiFi.status() == WL_CONNECTED) {
            server.send(200, "text/plain", "Conectado a Internet");
        } else {
            server.send(500, "text/plain", "Error de conexion");
        }
    });

    server.on("/cambiar_wifi", HTTP_GET, [enviarCORS]() {
        enviarCORS();
        server.send(200, "text/plain", "Configurando WiFi Manualmente");
        configurar_wifi_manual(); // Se ejecuta después de responder para no colgar la petición HTTP
    });

    server.on("/offline", HTTP_GET, [enviarCORS]() {
        online = false;
        enviarCORS();
        server.send(200, "text/plain", "Modo offline activado");
    });

    server.on("/alarma_critica", HTTP_GET, [enviarCORS]() {
        enviarCORS();
        server.send(200, "text/plain", "Alarma Critica Activada");
        alarma_critica(0, 0, 0);
    });

    // --- MANEJO DE PETICIONES PREFLIGHT (OPTIONS) ---
    // Los navegadores modernos hacen un disparo en 'OPTIONS' antes de mandar el comando real
    server.onNotFound([enviarCORS]() {
        if (server.method() == HTTP_OPTIONS) {
            enviarCORS();
            server.send(200, "text/plain", "OK");
        } else {
            server.send(404, "text/plain", "Not Found");
        }
    });
}

