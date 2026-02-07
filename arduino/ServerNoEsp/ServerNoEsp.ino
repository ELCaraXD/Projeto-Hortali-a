//// ----- Bibliotecas -----
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FSTools.h>
#include "FS.h"
#include "LittleFS.h"

// ----- Definições -----
#define WIFI_NAME   "João Baixa Verde"
#define WIFI_PASS   "Jonas123"

// ===== Wi-Fi do ESP (AP) =====
#define AP_SSID "ESP_CONFIG"
#define AP_PASS "12345678"


// ----- Variáveis globais -----
ESP8266WebServer server(80);


// ----- Protótipo das funções -----
void handle_pagina_inicial();

// funções do LittleFS ?//

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname,"r");
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path, "r");
    if(!file || file.isDirectory()){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, "w");
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, "a");
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path,"r");
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file && !file.isDirectory()){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, "w");
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}


// Fim//



void setup(void)
{
  // Inicializa a comunicação serial
  Serial.begin(115200);
 
  // Configura o WiFi
  WiFi.mode(WIFI_AP_STA);

  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
  
  WiFi.begin(WIFI_NAME, WIFI_PASS);
  Serial.println("Conectando à rede...");

  // Aguarda conexão
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
 
  // Printa o IP
  Serial.print("IP obtido: ");
  Serial.println(WiFi.localIP());
   
  // Configura as funções das páginas
  server.serveStatic("/", LittleFS, "/index.html");
  server.onNotFound(handle_nao_existe);

  // Inicializa o servidor
  server.begin();
  Serial.println("Web Server iniciado");
  
  //Inicializar o LittleFS
  LittleFSConfig cfg;
    cfg.setAutoFormat(true);
    LittleFS.setConfig(cfg);
    delay(3000);
    if(!LittleFS.begin()){
        Serial.println("LittleFS Mount Failed");
        return;
    }

    listDir(LittleFS, "/", 0);

}


void loop(void)
{
  // Responde às requisições feitas
  server.handleClient();
}

/*
 * Resposta de uma página que não existe
 */
void handle_nao_existe()
{
  String message = "<html><meta http-equiv=\'content-type\' content=\'text/html; charset=utf-8\'>";
  message += "<h1>Página não encontrada</h1></html>";
 
  server.send(200, "text/html", message);
}