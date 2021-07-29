#include <SoftwareSerial.h> // for UART communication with the Quectel BG96 module 

/* Pins connected to Arduino 
 * A0 -> ADC pin of MAX9934T (current consumption)
 * D4 -> CS pin of MAX9934T (current consumption)
 * D5 -> BG77 RST pin
 * D6 -> BG77 PWR pin
 * D7 -> BG77 TX pin
 * D8 -> BG77 RX pin
 */

SoftwareSerial BG77(7, 8); 
const int CS = 4;
const int RST = 5;
const int PWR = 6; 
const int CC = A0;
const int RXDatalen = 200;
int Connect; 
int Restarted = 0; 
int TimeoutTime;
int Print;
int sensorValue = 0;
int Startup_Done = 0; 

int x,y,z;
int check;
int receive_done = 0;
int Datachecklen;
char Datacheck[10];    
char RXData[RXDatalen];


void setup()
{
  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  Init();  

  BG77.write("AT+CPSMS=0\r"); 
  ShowSerialData("OK",0,1);

  BG77.write("AT+QURCCFG=\"urcport\",\"uart1\"\r"); 
  ShowSerialData("OK",0,1);

  BG77.write("AT+IPR?\r"); 
  ShowSerialData("OK",5000,1);
    
  BG77.write("ATI\r");
  ShowSerialData("OK",5000,1); 

  BG77.write("AT+QGMR\r");
  ShowSerialData("OK",5000,1);   

  if(Restarted == 1)
{
  ShowSerialData("\r\nAPP RDY\r\n",0,1); //wait for APP RDY message before turning on GNSS
}

}

void loop()
{
  BG77.write("AT+QADC=0\r");
  ShowSerialData("OK",0,1); 
  
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void Init()
{ 
  pinMode(CS, OUTPUT);
  pinMode(RST, OUTPUT);
  pinMode(PWR, OUTPUT);
  
  BG77.begin(115200); //115200 is default baud rate at startup but we will set it to 9600 later for better performance with softwareserial library  
    
  //check if module is on using Curent consumption 
  digitalWrite(CS,HIGH);
  for( x = 0; x < 10; x++)
  {
    sensorValue = sensorValue + analogRead(CC);
  }

  sensorValue = sensorValue / 10; //average of 10 values

  if(sensorValue < 50)//module is off, turn it on
  {   
    Serial.write("Module did not respond\n");
        
    digitalWrite(PWR, HIGH);
    ShowSerialData("RDY",5000,0);
    digitalWrite(PWR, LOW);  
    BG77.end();
    
    while(Startup_Done != 1)
    {
      BG77.begin(9600); //  check 9600 baud for OK
      BG77.write("AT\r");    
      
      if(ShowSerialData("OK",5000,0) == 1)
      {
          Serial.write("Startup was OK\n");
          Startup_Done = 1;
          Restarted = 1;   
      }
      BG77.end();

      BG77.begin(115200); //  check 115200 baud for OK
      BG77.write("AT\r");    
      
      if(ShowSerialData("OK",5000,0) == 1)
      {
          Serial.write("Startup was OK\n");
          Startup_Done = 1;
          Restarted = 1;   
      }
      BG77.end();
    }  
  } 
  
  Baudswitch(); 
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void Baudswitch()
{
  BG77.write("AT+IPR=9600;&W0\r"); // make 9600 baud the default, and save setting
  ShowSerialData("OK",5000,0);
  BG77.end(); 
  BG77.begin(9600);

  BG77.write("AT\r");
  if(ShowSerialData("OK",5000,0)== 2)
  {
    ShowSerialData("APP RDY",0,0);
  }
}

int ShowSerialData(char Datacheck[], int TimeoutTime, int Print)
{
  receive_done = 0;
  check = 0;
  x = 0;
  y = 0; 
  z = 0;  
  Datachecklen = strlen(Datacheck); 
  
  while (receive_done == 0)
  {
    if (BG77.available() != 0)
    {    
      RXData[y] = BG77.read();
      
      for(z = 0; z < Datachecklen; z++)
      {
        if(RXData[y - z] != Datacheck[(Datachecklen - (z+1))])
        {        
          z = Datachecklen; //stop check tot volgende ontvangst  
        }
        else
        {          
          check++; 
        }
        if(check == Datachecklen)
        {
          if(Print == 1)
          {
            Serial.write("RX:");
            Serial.println(RXData); //PRINT REPLY
          }
          receive_done = 1;
        }  
      } 
      y++; 
    }

    if(TimeoutTime != 0)
    {    
      if (x == TimeoutTime) //TIMEOUT 
      {
         if(Print == 1)
         {          
          Serial.write("TIMEOUT\n");
          Serial.println(RXData); //PRINT REPLY
         }
        receive_done = 2; 
      }
      x++;
    }
  } 

  for(z = 0; z < RXDatalen; z++) //Clear RX buffer
  {
    RXData[z] = 0;
  }
 
  return receive_done;
}
