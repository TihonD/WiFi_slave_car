#ifndef Sakura_ezWiFi.h
#define Sakura_ezWiFi.h


//ESP pins
#ifndef WPRG
#define WPRG 25
#endif
#ifndef WRST
#define WRST 26
#endif

#define w_debug//USART debug messages

#define TIMEOUT 10000 //ESP response timeout, ms

class ezWiFi
{
	public:
	int local_IP[2];
  int conn_type;
  String SSID;
  String SSID_pwd;
  int port;

  char buff[50];
  unsigned int buff_p;
  

  //init
	ezWiFi(long int br);
  //br - USART baudrate (0 = 115200)

  //reset ESP
	boolean reset();

  //connection to the router in client mode
	boolean net_connect(String w_ssid, String w_pwd);
  boolean net_connect(String w_ssid, String w_pwd, int mode);
  //w_ssid and w_pwd - net SSID and password
  //mode - ESP mode (0 - off; 1 - client; 2 - AP; 3 - AP+client) (availible is 1 and 3, default is 1)
  
  //GET request
  boolean get_req(String tcp_domain, String tcp_path, int tcp_port);
  boolean get_req(String tcp_domain, String tcp_path);
  //tcp_domain, tcp_path - domain and path (example: "http://TihonD.RU/iot/upload.php?t=20&h=740" -> "http://TihonD.RU" is the domain, "/iot/upload.php?t=20&h=740" is the path)
  //tcp_port - TCP port (default is 80)

  //get local IP in String
  String localIPstring();

  //send startup message to TihonD.RU/connect (or the same) IoT server
  boolean sendIP(String user, String pwd, String dev);
	boolean sendIP(String user, String pwd, String dev, String link);
	//default IoT server is www.TihonD.RU/connect
	//user - user name on IoT server
  //id, pwd - device ID and access code (from IoT server)
  //link - default device local link 
	
	
	private:
	long int wifi_USART_br=115200;
  int xIP[4];
  
  boolean SendCommand(String cmd, String ack, boolean halt_on_fail);
	void errorHalt(String msg);
	boolean echoFind(String keyword);
	int connFind();
  boolean get_local_IP();
  boolean contentFind();
	
};

#endif
