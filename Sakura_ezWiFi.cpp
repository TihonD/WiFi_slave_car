#include "Arduino.h"
#include "Sakura_ezWiFi.h"

ezWiFi::ezWiFi(long int br)
{
	if(br==0) wifi_USART_br=115200;
	else wifi_USART_br=br;

  //ESP USART init
  //Serial1 must be defined!
  //Example: #define Serial1 Serial1
  //Serial1(wifi_USART_br);
  Serial1.begin(wifi_USART_br);
  reset();
}

boolean ezWiFi::reset()
{
	//ESP pins init
	pinMode(WPRG,OUTPUT);
	pinMode(WRST,OUTPUT);
	digitalWrite(WPRG,HIGH);
	digitalWrite(WRST,HIGH);
	
	//ESP hard reset
	//default pulse lenght: 50 ms.
	digitalWrite(WRST,LOW);///WIFI HARD RESET
	delay(50);
	digitalWrite(WRST,HIGH);
	echoFind("ready");
  
	//ESP soft reset
	SendCommand("AT+RST", "Ready", true);
  echoFind("WIFI");
  conn_type = connFind();
  if(conn_type==1) get_local_IP();
}

boolean ezWiFi::net_connect(String w_ssid, String w_pwd)
{
	//mode->client
	SendCommand("AT+CWMODE=1", "OK", true);

  //join to AP with SSID and pwd
	SendCommand("AT+CWJAP=\"" + w_ssid + "\",\"" + w_pwd + "\"", "DISCONNECT", true);//SSID, PWD
	SSID=w_ssid;
	SSID_pwd=w_pwd;

  //connection checking
	conn_type = connFind();
  if(conn_type==1) get_local_IP();
	echoFind("OK");
}

boolean ezWiFi::net_connect(String w_ssid, String w_pwd, int mode)
{
  //mode->client
  SendCommand("AT+CWMODE="+String(mode), "OK", true);

  //join to AP with SSID and pwd
  SendCommand("AT+CWJAP=\"" + w_ssid + "\",\"" + w_pwd + "\"", "DISCONNECT", true);//SSID, PWD
  SSID=w_ssid;
  SSID_pwd=w_pwd;

  //connection checking
  conn_type = connFind();
  if(conn_type==1) get_local_IP();
  echoFind("OK");
}

boolean ezWiFi::get_local_IP()
{
  int p,k;
  
  //get IP and MAC
  SendCommand("AT+CIFSR", "AT", true);

  //waiting for IP
  echoFind("CIFSR:STAIP,\"");

  //grabbing dogits
  for(k=0;k<4;k++)
  {
    p=0;
    while(1)
    {
      if (Serial1.available())
      {
        char ch = Serial1.read();
        if ((ch<'0')||(ch>'9')) break;
        p=p*10+ch-'0';
      }
    }
    xIP[k]=p;
  }

  //checking IP
  if((xIP[0]!=192)||(xIP[1]!=168))//not valid local IP
  {
    local_IP[0]=0;
    local_IP[1]=0;
    return 1;
  }
  else
  {
    local_IP[0]=xIP[2];
    local_IP[1]=xIP[3];
    return 0;
  }
  
}

boolean ezWiFi::get_req(String tcp_domain, String tcp_path, int tcp_port=0)
{
  if(tcp_port==0) tcp_port=80;

  String tcp_str="GET http://" + tcp_domain + tcp_path + " HTTP/1.0\r\n\r\n";
  String tcp_strlen=String(tcp_str.length());
  SendCommand("AT+CIPSTART=\"TCP\",\""+tcp_domain+"\","+String(tcp_port),"OK",true);
  delay(2000);
  SendCommand("AT+CIPSEND="+tcp_strlen,">",true);
  delay(50);
  Serial.println(tcp_str);
  //Serial1.println(tcp_str);
  SendCommand(tcp_str,"OK",true);
  echoFind("+IPD");
  echoFind("CLOSED");
 
}

boolean ezWiFi::get_req(String tcp_domain, String tcp_path)
{
  int tcp_port=80;
  int p=0;

  byte ch;

  String tcp_str="GET http://" + tcp_domain + tcp_path + " HTTP/1.0\r\n\r\n";
  String tcp_strlen=String(tcp_str.length());
  SendCommand("AT+CIPSTART=\"TCP\",\""+tcp_domain+"\","+String(tcp_port),"OK",true);
  delay(2000);
  SendCommand("AT+CIPSEND="+tcp_strlen,">",true);
  delay(50);
  Serial.println(tcp_str);
  //Serial1.println(tcp_str);
  SendCommand(tcp_str,"OK",true);
  //delay(20000);
  //SendCommand("AT+CIPCLOSE","OK",true);
  /*while(1)///////////////////////////////////////
  {
    if(Serial1.available()) Serial.write(Serial1.read());
  }*/
  echoFind("+IPD,");
  p=0;
  while(Serial1.available())
  {
    ch=Serial1.read();
    if((ch<'0')||(ch>'9')) break;
    p=p*10+ch-'0';
  }
  echoFind("Content-Length: ");
  p=0;
  while(Serial1.available())
  {
    ch=Serial1.read();
    if((ch<'0')||(ch>'9')) break;
    p=p*10+ch-'0';
  }
  buff_p=p;
  echoFind("\r\n\r\n\r\n");
  for(int i_i=0;i_i<p;i_i++)
  {
    while(!Serial1.available());
    buff[i_i]=Serial1.read();
  }
  //if(Serial1.available()) buff[i_i]=Serial1.read();
  echoFind("CLOSED");//contentFind();
 
}

boolean ezWiFi::SendCommand(String cmd, String ack, boolean halt_on_fail)
{
	Serial1.println(cmd); // Send command to module
	// Otherwise wait for ack.
	if (!echoFind(ack)) // timed out waiting for ack string
	{
		if (halt_on_fail) errorHalt(cmd+" failed");// Critical failure halt.
		else return false; // Let the caller handle it.
	}
	return true; // ack blank or ack found
}

boolean ezWiFi::echoFind(String keyword)
{
	byte current_char = 0;
	byte keyword_length = keyword.length();
	// Fail if the target string has not been sent by deadline.
	long deadline = millis() + TIMEOUT;
	while(millis() < deadline)
	{
		if (Serial1.available())
		{
			char ch = Serial1.read();
			#ifdef w_debug
			Serial.write(ch);
      #endif
			if (ch == keyword[current_char])
			{
				if (++current_char == keyword_length)
				{
					delay(10);
					return true;
				}
			}
		}
	}
	return false; // Timed out
}

boolean ezWiFi::contentFind()
{
  String keyword="CLOSED";
  byte current_char = 0;
  byte keyword_length = keyword.length();
  int p=0;
  // Fail if the target string has not been sent by deadline.
  long deadline = millis() + TIMEOUT;
  while(millis() < deadline)
  {
    if (Serial1.available())
    {
      char ch = Serial1.read();
      buff[p]=ch;
      p++;
      #ifdef w_debug
      Serial.write(ch);
      #endif
      if (ch == keyword[current_char])
      {
        if (++current_char == keyword_length)
        {
          delay(10);
          buff_p=p-6;
          Serial.println("->"+String(buff_p)+"<-");
          return true;
        }
      }
    }
  }
  return false; // Timed out
}

int ezWiFi::connFind()
{
  String keyword1="GOT IP";//"CONNECTED";
  String keyword2="DISCONNECT";
  
  byte current_char1 = 0;
  byte current_char2 = 0;
  
  byte keyword1_length = keyword1.length();
  byte keyword2_length = keyword2.length();
  
  // Fail if the target string has not been sent by deadline.
  long deadline = millis() + TIMEOUT;
  while(millis() < deadline)
  {
    if (Serial1.available())
    {
      char ch = Serial1.read();
      //Serial.write(ch);
      if (ch == keyword1[current_char1])
      {
        if (++current_char1 == keyword1_length)
        {
          delay(10);
          return 1;
        }
      }
      if (ch == keyword2[current_char2])
      {
        if (++current_char2 == keyword2_length)
        {
          delay(10);
          return 2;
        }
      }
    }
  }
  return 0; // Timed out
}

void ezWiFi::errorHalt(String msg)
{
	Serial.println(msg);
	Serial.println("HALT");
	while(1);
}

String ezWiFi::localIPstring()
{
  return "192.168."+String(local_IP[0])+"."+String(local_IP[1]);
}

boolean ezWiFi::sendIP(String user, String pwd, String dev)
{
  get_req("TihonD.RU", "/connect/set_ip.php?user="+user+"&dev="+dev+"&ip="+localIPstring()+"&pwd="+pwd, 80);
}

boolean ezWiFi::sendIP(String user, String pwd, String dev, String link)
{
  get_req("TihonD.RU", "/connect/set_ip.php?user="+user+"&dev="+dev+"&ip="+localIPstring()+"&pwd="+pwd+"&link="+link, 80);
}



/*void ezWiFi::BlinkLED(int LEDPin, int NumberOfBlinks, int OnDuration)
{
 for (int x=1; x <= NumberOfBlinks ; x ++){
 digitalWrite(LEDPin,LOW);
 delay(OnDuration);
 digitalWrite(LEDPin,HIGH);
 delay(OnDuration); 
 }
}*/
