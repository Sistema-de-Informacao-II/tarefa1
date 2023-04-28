#if ! (ESP8266 || ESP32 )
  #error This code is intended to run on the ESP8266/ESP32 platform! Please check your Tools->Board setting
#endif
#include "Credentials.h"
#include <SimpleDHT.h>
#include <WiFiManager.h>
#define MYSQL_DEBUG_PORT      Serial
#define _MYSQL_LOGLEVEL_      1
#include <MySQL_Generic.h>

IPAddress internal_server(10, 16, 1, 20);       //ip para acesso interno
IPAddress external_server(200, 239, 65, 28);    //ip para acesso externo
//IPAddress local_server(127, 0, 0, 1);         //ip local para testes
uint16_t server_port = 3306;    //3306 valor padrão

int pinDHT11 = 2;
SimpleDHT11 dht11(pinDHT11);

char default_database[] = "topicos";           //"test_arduino";
char default_table[]    = "dadosTU";          //"test_arduino";


MySQL_Connection conn((Client *)&client);

MySQL_Query *query_mem;

// configuração do wifi manager
void configWiFi(){
  WiFiManager wm;

  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  res = wm.autoConnect("Topicos"); // anonymous ap
  // res = wm.autoConnect("Topicos","1234"); // password protected ap

  if(!res) {
      Serial.println("Failed to connect");
      // ESP.restart();
  } 
  else {
    //if you get here you have connected to the WiFi    
    Serial.println("connected...yeey :)");
  }
}

void setup()
{
  Serial.begin(115200);
  while (!Serial && millis() < 5000); // wait for serial port to connect

  MYSQL_DISPLAY1("\nStarting Basic_Insert_ESP on", ARDUINO_BOARD);
  MYSQL_DISPLAY(MYSQL_MARIADB_GENERIC_VERSION);

  // Configura o Wi-Fi
  configWiFi();  
}

// função de inserção na
void insertRow()
{
  float temp = 20;
  float hum = 20;

  // Initiate the query class instance

  String INSERT_SQL = String("INSERT INTO ") + default_database + "." + default_table 
                 + " (usuario, idSensor, tempCelsius, umidade) VALUES ('Pedro e Jonathan', 66," + temp +", "+ hum +")";

  MySQL_Query query_mem = MySQL_Query(&conn);


  if (conn.connected())
  {
    MYSQL_DISPLAY(INSERT_SQL);
    
    if ( !query_mem.execute(INSERT_SQL.c_str()) )
    {
      MYSQL_DISPLAY("Insert error");
    }
    else
    {
      query_mem.show_results();
    }
  }
  else
  {
    MYSQL_DISPLAY("Disconnected from Server. Can't insert.");
  }
}

void queryTable(){

  String consultaQuery = "SELECT * FROM topicos.dadosTU dt;";  
  MySQL_Query query_mem = MySQL_Query(&conn);

  if (conn.connected())
  {
    MYSQL_DISPLAY(consultaQuery);
    
    // Execute the query
    // KH, check if valid before fetching
    if ( !query_mem.execute(consultaQuery.c_str()) )
    {
      MYSQL_DISPLAY("Consulta error");
    }
    else
    {
      query_mem.show_results();
    }
  }
  else
  {
    MYSQL_DISPLAY("Disconnected from Server. Can't insert.");
  }  
}

void loop()
{
  MYSQL_DISPLAY("Connecting...");
  if(String(WiFi.localIP()[0]) == "10" && String(WiFi.localIP()[1]) == "16" && String(WiFi.localIP()[2]) == "1"){
    MYSQL_DISPLAY3("Connecting to SQL Server @", internal_server, ", Port =", server_port);
    MYSQL_DISPLAY5("User =", user, ", PW =", password, ", DB =", default_database);
    
    if (conn.connectNonBlocking(internal_server, server_port, user, password) != RESULT_FAIL)
    {
      insertRow(); 
      delay(500); 
      queryTable();
      delay(2000);
      conn.close();
    } 
    else 
    {
      MYSQL_DISPLAY("\nConnect failed. Trying again on next iteration.");
    }
  }else{
      MYSQL_DISPLAY3("Connecting to SQL Server @", external_server, ", Port =", server_port);
      MYSQL_DISPLAY5("User =", user, ", PW =", password, ", DB =", default_database);
      if (conn.connectNonBlocking(external_server, server_port, user, password) != RESULT_FAIL)
      {
        insertRow(); 
        delay(500); 
        queryTable();
        delay(2000);
        conn.close();
      } 
      else 
      {
        MYSQL_DISPLAY("\nConnect failed. Trying again on next iteration.");
      }
  }

  MYSQL_DISPLAY("\nSleeping...");
  MYSQL_DISPLAY("================================================");
 
  delay(40000);
}
