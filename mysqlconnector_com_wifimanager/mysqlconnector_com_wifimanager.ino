#include <DHT.h>            // DHT sensor library por adafruit (Foi utilizado essa biblioteca para que fosse possível usar float)
#define DHTPIN 2
#define DHTTYPE DHT11
#include <WiFiManager.h>    // WiFiManager por tzapu
#define MYSQL_DEBUG_PORT      Serial
#define _MYSQL_LOGLEVEL_      1
#include <MySQL_Generic.h>  //MySQL_MariaDB_Generic por Dr. Charles

DHT dht(DHTPIN, DHTTYPE);

IPAddress internal_server(10, 16, 1, 20);       //ip para acesso interno
IPAddress external_server(200, 239, 65, 28);    //ip para acesso externo

uint16_t server_port = 3306;

char default_database[] = "topicos";
char default_table[]    = "dadosTU";
char user[]         = "aluno";
char password[]     = "aula";

MySQL_Connection conn((Client *)&client);

MySQL_Query *query_mem;

// Configuração do Wifi Manager
void configWiFi(){
  WiFiManager wm;

  bool res;

  res = wm.autoConnect("Topicos Especiais", "12345678"); // anonymous ap

  if(!res) {
      Serial.println("Falha na conexão!");
  } 
  else {
    Serial.println("Conectado a rede! :)");
  }
}

// Inserção no banco de dados Topicos
void insertRow()
{
  float hum = dht.readHumidity();
  float temp = dht.readTemperature(); 

  String INSERT_SQL = String("INSERT INTO ") + default_database + "." + default_table 
                 + " (usuario, idSensor, tempCelsius, umidade) VALUES ('Pedro e Jonathan', 66," + temp +", "+ hum +")";

  MySQL_Query query_mem = MySQL_Query(&conn);

  if (conn.connected())
  {
    MYSQL_DISPLAY(INSERT_SQL);
    
    if ( !query_mem.execute(INSERT_SQL.c_str()) )
    {
      MYSQL_DISPLAY("Erro na inserção");
    }
    else
    {
      query_mem.show_results();
    }
  }
  else
  {
    MYSQL_DISPLAY("Desconectado do Servidor.");
  }
}

// Mostra os dados da tabela topicos.dadosTU
void queryTable(){

  String consultaQuery = "SELECT * FROM topicos.dadosTU dt;";  
  MySQL_Query query_mem = MySQL_Query(&conn);

  if (conn.connected())
  {
    MYSQL_DISPLAY(consultaQuery);
    
    if ( !query_mem.execute(consultaQuery.c_str()) )
    {
      MYSQL_DISPLAY("Erro na consulta");
    }
    else
    {
      query_mem.show_results();
    }
  }
  else
  {
    MYSQL_DISPLAY("Desconectado do Servidor.");
  }  
}

void setup()
{
  Serial.begin(115200);
  while (!Serial && millis() < 5000);

  MYSQL_DISPLAY1("\nStarting Basic_Insert_ESP on", ARDUINO_BOARD);
  MYSQL_DISPLAY(MYSQL_MARIADB_GENERIC_VERSION);

  // Chama a função de conexão à rede Wi-Fi utilizando o wifi-manager
  configWiFi();
  dht.begin(); // inicializa o dht sensor
}

void loop()
{
  MYSQL_DISPLAY("Conectando...");
  // Se o ip local da máquina for 10.16.1.*, tenta conectar a partir do IP interno no servidor de BD
  if(String(WiFi.localIP()[0]) == "10" && String(WiFi.localIP()[1]) == "16" && String(WiFi.localIP()[2]) == "1"){
    MYSQL_DISPLAY3("Conectando ao servidor SQL @", internal_server, ", Port =", server_port);
    MYSQL_DISPLAY5("Usuário =", user, ", Senha =", password, ", DB =", default_database);
    
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
      MYSQL_DISPLAY("\nFalha na conexão. Tentando novamente na próxima interação");
    }
  }else{
      // Se o ip local da máquina for diferente de 10.16.1.*, tenta conectar a partir do IP externo no servidor de BD
      MYSQL_DISPLAY3("Conectando ao servidor SQL @", external_server, ", Port =", server_port);
      MYSQL_DISPLAY5("Usuário =", user, ", Senha =", password, ", DB =", default_database);
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
        MYSQL_DISPLAY("\nFalha na conexão. Tentando novamente na próxima interação");
      }
  }

  MYSQL_DISPLAY("\nSleeping...");
  MYSQL_DISPLAY("================================================");
 
  delay(120000); // Intervalo de 2 minutos (solicitado na tarefa) para a próxima execução do programa
}
