#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>

#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>
#include <SPIFFS.h> // bíblioteca própria para conectar outros arquivos de código

// Entradas (sensores)
#define ENTRADA_VP 36
#define ENTRADA_VN 39
#define ENTRADA_D34 34
#define ENTRADA_D32 32
#define ENTRADA_D33 33
#define ENTRADA_D25 25
#define ENTRADA_D26 26
#define ENTRADA_D27 27

// Saídas
#define PINO_TRAVA 23
#define PINO_ESTEIRA 22
#define PINO_SEPARADOR 21
#define PINO_MAGAZINE 19
#define SAIDA_D18 18 // Medidor

// Wi-Fi
const char *ssid = "FESTO4";

const char *password = "festo4123";
WebServer server(80);

bool modoAutomatico = false;

enum Estado
{
  ESTADO_1,
  ESTADO_2,
  ESTADO_3
};
Estado estadoAtual = ESTADO_1;

unsigned long tempoInicioCiclo = 0;
unsigned long tempoFimCiclo = 0;
bool cicloEmAndamento = false;
unsigned long contadorPecas = 0;

unsigned long ultimoTempoCiclo = 0; // Tempo do último ciclo em ms

void setup()
{

  Serial.begin(9600);

  SPIFFS.begin(true);

  // Configuração de pinos
  pinMode(ENTRADA_VP, INPUT);
  pinMode(ENTRADA_VN, INPUT);
  pinMode(ENTRADA_D34, INPUT);
  pinMode(ENTRADA_D32, INPUT);
  pinMode(ENTRADA_D33, INPUT);
  pinMode(ENTRADA_D25, INPUT);
  pinMode(ENTRADA_D26, INPUT);
  pinMode(ENTRADA_D27, INPUT);

  pinMode(PINO_TRAVA, OUTPUT);
  pinMode(PINO_ESTEIRA, OUTPUT);
  pinMode(PINO_SEPARADOR, OUTPUT);
  pinMode(PINO_MAGAZINE, OUTPUT);
  pinMode(SAIDA_D18, OUTPUT);

  // Reset de atuadores
  digitalWrite(PINO_TRAVA, LOW);
  digitalWrite(PINO_ESTEIRA, LOW);
  digitalWrite(PINO_SEPARADOR, LOW);
  digitalWrite(PINO_MAGAZINE, LOW);
  digitalWrite(SAIDA_D18, LOW);

  // WiFi Access Point
  WiFi.softAP(ssid, password);
  Serial.print("Acesse: http://");
  Serial.println(WiFi.softAPIP());

  // Servindo os arquivos da interface via SPIFFS
  // server.serveStatic("/", SPIFFS, "/index.html").setDefaultFile("index.html");
  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/style.css", SPIFFS, "/style.css");
  server.serveStatic("/script.js", SPIFFS, "/script.js");

  // Rotas para tempo e contador — ESSENCIAIS FORA do /comando
  server.on("/tempo", HTTP_GET, []()
            { server.send(200, "text/plain", String(ultimoTempoCiclo)); });
  server.on("/contador", HTTP_GET, []()
            { server.send(200, "text/plain", String(contadorPecas)); });

  // Rota nova para enviar os dados juntos em JSON
  server.on("/dados", HTTP_GET, []()
            {
    String json = "{";
    json += "\"tempo\":" + String(ultimoTempoCiclo) + ",";
    json += "\"contador\":" + String(contadorPecas);
    json += "}";
    server.send(200, "application/json", json); });

  // Rota para comandos
  server.on("/comando", HTTP_GET, []()
            {
    String acao = server.arg("acao");

    if (acao == "ligarSistema") {
      modoAutomatico = true;
      server.send(200, "text/plain", "Modo automático ativado");
    } else if (acao == "desligarSistema") {
      modoAutomatico = false;
      server.send(200, "text/plain", "Modo manual ativado");
    }

    if (!modoAutomatico) {
      if (acao == "ligarEsteira")        digitalWrite(PINO_ESTEIRA, HIGH);
      else if (acao == "desligarEsteira") digitalWrite(PINO_ESTEIRA, LOW);

      else if (acao == "ligarSeparador")  digitalWrite(PINO_SEPARADOR, HIGH);
      else if (acao == "desligarSeparador") digitalWrite(PINO_SEPARADOR, LOW);

      else if (acao == "ligarTrava")      digitalWrite(PINO_TRAVA, HIGH);
      else if (acao == "desligarTrava")   digitalWrite(PINO_TRAVA, LOW);

      else if (acao == "ligarMedidor")    digitalWrite(SAIDA_D18, HIGH);
      else if (acao == "desligarMedidor") digitalWrite(SAIDA_D18, LOW);

      else if (acao == "ligarMagazine")   digitalWrite(PINO_MAGAZINE, HIGH);
      else if (acao == "desligarMagazine") digitalWrite(PINO_MAGAZINE, LOW);

      server.send(200, "text/plain", "Comando manual executado: " + acao);
    } else {
      if (acao.startsWith("ligar") || acao.startsWith("desligar")) {
        server.send(200, "text/plain", "Comando ignorado: modo automático ativado");
      }
    } });

  server.begin();
}

void loop()
{

  server.handleClient();

  if (modoAutomatico)
  {
    switch (estadoAtual)
    {
    case ESTADO_1:
      if (digitalRead(ENTRADA_VP) == HIGH)
      {
        tempoInicioCiclo = millis(); // Início do ciclo
        cicloEmAndamento = true;

        digitalWrite(PINO_ESTEIRA, HIGH);
        estadoAtual = ESTADO_2;
      }
      break;

    case ESTADO_2:
      if (digitalRead(ENTRADA_D33) == HIGH)
      {
        digitalWrite(PINO_MAGAZINE, HIGH);
        digitalWrite(PINO_TRAVA, HIGH);

        if (digitalRead(ENTRADA_VN) == HIGH)
        {
          digitalWrite(PINO_SEPARADOR, LOW);
          estadoAtual = ESTADO_3;
        }

        else
        {
          digitalWrite(PINO_SEPARADOR, HIGH);
          estadoAtual = ESTADO_3;
        }
      }
      break;

    case ESTADO_3:
      if (digitalRead(ENTRADA_D34) == LOW)
      {
        digitalWrite(PINO_ESTEIRA, LOW);
        digitalWrite(PINO_MAGAZINE, LOW);
        digitalWrite(PINO_TRAVA, LOW);
        digitalWrite(PINO_SEPARADOR, LOW);

        tempoFimCiclo = millis(); // Fim do ciclo
        if (cicloEmAndamento)
        {
          unsigned long duracao = tempoFimCiclo - tempoInicioCiclo;
          ultimoTempoCiclo = duracao; // Salva tempo do ciclo
          contadorPecas++;            // Incrementa contador de peças
                                      //
          Serial.print("Ciclo concluído. Tempo de execução: ");
          Serial.print(duracao);
          Serial.println(" ms");
          cicloEmAndamento = false;
        }

        estadoAtual = ESTADO_1;
      }
      break;
    }
  }
}