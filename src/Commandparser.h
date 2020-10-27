//#include <Arduino.h>
//#ifndef SETTINGS_H
//  #include "Settings.h"
//#endif

#define BUFSIZE 350
#define RSTTIMEOUT 300000
#define DEBUGPORT Serial
extern void Send(String test);

 // inkomende seriele data (byte)
bool readnextLine = false;
char buff[BUFSIZE]; //Buffer voor seriele data om \n te vinden.
int bufpos = 0;
//float SendPrepSeconds = 0;
//float BroadCastCnt = 0;
unsigned long lastUpload = 0;
extern String getHttp(String url);
//float Voltage[2]={1.0 / 0.0, 1.0 / 0.0};
//float Current[2]={0.1, 0.2}; //float Current[2]={1.0 / 0.0, 1.0 / 0.0};
//float Power[2]={1.0 / 0.0, 1.0 / 0.0};

//float Current[2][3]={{1.0 / 0.0, 1.0 / 0.0, 1.0 / 0.0},{1.0 / 0.0, 1.0 / 0.0, 1.0 / 0.0}};
//float Power[2][3]={{1.0 / 0.0, 1.0 / 0.0, 1.0 / 0.0},{1.0 / 0.0, 1.0 / 0.0, 1.0 / 0.0}};
//float Voltage[2][3]={{1.0 / 0.0, 1.0 / 0.0, 1.0 / 0.0},{1.0 / 0.0, 1.0 / 0.0, 1.0 / 0.0}};
//float Energy[2]={1.0 / 0.0, 1.0 / 0.0};
//float Frequency[2]={1.0 / 0.0, 1.0 / 0.0};

float Current[2][3]={{0,0,0},{0,0,0}};
float Power[2][3]={{0,0,0},{0,0,0}};
float PhaseVoltage[2][3]={{0,0,0},{0,0,0}};
float LineVoltage[2][3]={{0,0,0},{0,0,0}};
float Energy[2]={0,0};
float Frequency[2]={0,0};
float CurrentList[20];
int numberOfZeroReadings[2]={0,0};

//void softReset(){
//  ESP.reset();
//}
//struct FloatTriplet
//{
//    float Fl[3];
//};

float bytesToFloat(unsigned char  b0, unsigned char  b1, unsigned char  b2, unsigned char  b3)
{
    float output;

    *((unsigned char *)(&output) + 3) = b0;
    *((unsigned char *)(&output) + 2) = b1;
    *((unsigned char *)(&output) + 1) = b2;
    *((unsigned char *)(&output) + 0) = b3;

    return output;
}

bool bytesArrToFloatArr(char * Arr, unsigned int ArrLen, float * OutputArr, unsigned int FloatLen)
{
    //FloatTriplet TriOutput;
    for(int i=0; (i<FloatLen) && (4*i+3 < ArrLen); i++)
    {
        float output;

        *((unsigned char *)(&output) + 3) = (unsigned char) Arr[4*i+0];
        *((unsigned char *)(&output) + 2) = Arr[4*i+1];
        *((unsigned char *)(&output) + 1) = Arr[4*i+2];
        *((unsigned char *)(&output) + 0) = Arr[4*i+3];
        
        OutputArr[i] = output;
    }

    return true;
}

/*
FloatTriplet bytesArrToFloatTriplet(unsigned char * Arr, unsigned int ArrLen)
{
    FloatTriplet TriOutput;
    for(int i=0; (i<3) && (4*i+3 < ArrLen); i++)
    {
        float output;

        *((unsigned char *)(&output) + 3) = Arr[4*i+0];
        *((unsigned char *)(&output) + 2) = Arr[4*i+1];
        *((unsigned char *)(&output) + 1) = Arr[4*i+2];
        *((unsigned char *)(&output) + 0) = Arr[4*i+3];
        
        TriOutput.fl[i] = output;
    }

    return TriOutput;
}
*/
int stringParse(char * buf, int buflen){
  String cmdValue;
  int Charger=0;
  for (int i=0; i<buflen; i++){
    if(buf[i]==0x0F && i+2>=buflen) //error check?
    {
      DEBUGPORT.println("E");
    }
    if((buf[i]==0x0F || buf[i]==0x0E) && i+2<buflen) //we detect a command-character and the next two characters exists
    {
      if (buf[i]==0x0F)
      {
        Charger=1;  
      }
      else
      {
        Charger=2;
      }
      DEBUGPORT.print("Charger "+String(Charger)+": ");
      String measurements;
      switch (buf[i+1])
      {
        case 1:
          //Phase Voltage
          if(buflen>4)
          {
              //Voltage[Charger-1] = bytesToFloat(buf[i+2], buf[i+3], buf[i+4], buf[i+5]);
              float VoltageArr[3];
              if(bytesArrToFloatArr(&buf[i+2], 12, VoltageArr,3))
              {
                //Current[Charger-1] = (CurrentArr[0]+CurrentArr[1]+CurrentArr[2])/3;  
                for(int i=0;i<3;i++)
                {
                    PhaseVoltage[Charger-1][i]=VoltageArr[i];
                }
                //Voltage[Charger-1] = {VoltageArr[0],VoltageArr[1],VoltageArr[2]};
              }
              DEBUGPORT.print("Voltage: "+String(PhaseVoltage[Charger-1][0],4)+" "+String(PhaseVoltage[Charger-1][1],4)+" "+String(PhaseVoltage[Charger-1][2],4));
          }
          else
          {
              DEBUGPORT.print("did not receive enough voltage data");
          }
          break;
        case 2:
          //Current
          if(buflen>12) //(buflen>4)
          {
              //Current[Charger-1] = bytesToFloat(buf[i+2], buf[i+3], buf[i+4], buf[i+5]);
              float CurrentArr[3];
              if(bytesArrToFloatArr(&buf[i+2], 12, Current[Charger-1],3))
              {
                //Current[Charger-1] = (CurrentArr[0]+CurrentArr[1]+CurrentArr[2])/3;  
                //Current[Charger-1] = {CurrentArr[0],CurrentArr[1],CurrentArr[2]}
              }
              DEBUGPORT.print("Current: ");
              DEBUGPORT.print(Current[Charger-1][0],4);DEBUGPORT.print(" ");
              DEBUGPORT.print(Current[Charger-1][1],4);DEBUGPORT.print(" ");
              DEBUGPORT.println(Current[Charger-1][2],4);
              if((Current[Charger-1][0]+Current[Charger-1][1]+Current[Charger-1][2])<1)
              {
                numberOfZeroReadings[Charger-1]++;
              }
              else
              {
                  numberOfZeroReadings[Charger-1]=0;
              }
              for(int i=18;i>=0;i--)
              {
                   CurrentList[i+1]= CurrentList[i];
              }
              CurrentList[0]=(Current[0][0]+Current[0][1]+Current[0][2]);//numberOfZeroReadings[0];
              CurrentList[1]=numberOfZeroReadings[0];
              CurrentList[2]=Current[0][2];
          }
          else
          {
              DEBUGPORT.print("did not receive enough current data");
          }
          break;  
        case 3:
          //Frequency
          if(buflen>4)
          {
              Frequency[0] = bytesToFloat(buf[i+2], buf[i+3], buf[i+4], buf[i+5]);
              Frequency[1]=Frequency[0];
              DEBUGPORT.print("Frequency: ");
              DEBUGPORT.print(Frequency[Charger-1],4);
          }
          else
          {
              DEBUGPORT.print("did not receive enough frequency data");
          }
          break;
        case 4:
          //Power
          if(buflen>4)
          {
              //Power[Charger-1] = bytesToFloat(buf[i+2], buf[i+3], buf[i+4], buf[i+5]);
              float PowerArr[3];
              if(bytesArrToFloatArr(&buf[i+2], 12, Power[Charger-1],3))
              {
                //Power[Charger-1] = {PowerArr[0],PowerArr[1],PowerArr[2]}
              }
              DEBUGPORT.print("Power: "+String(Power[Charger-1][0],4)+" "+String(Power[Charger-1][1],4)+" "+String(Power[Charger-1][2],4));              
              //DEBUGPORT.print("Power: ");
              //DEBUGPORT.print(Power[Charger-1],4);
          }
          else
          {
              DEBUGPORT.print("did not receive enough power data");
          }
          break;
        case 5:
          //Energy
          if(buflen>4)
          {
              Energy[Charger-1] = bytesToFloat(buf[i+2], buf[i+3], buf[i+4], buf[i+5]);
              DEBUGPORT.print("Energy: ");
              DEBUGPORT.print(Energy[Charger-1],4);
          }
          else
          {
              DEBUGPORT.print("did not receive enough energy data");
          }
          break;
        case 6:
          //Line Voltage
          if(buflen>4)
          {
              //Voltage[Charger-1] = bytesToFloat(buf[i+2], buf[i+3], buf[i+4], buf[i+5]);
              float VoltageArr[3];
              if(bytesArrToFloatArr(&buf[i+2], 12, VoltageArr,3))
              {
                //Current[Charger-1] = (CurrentArr[0]+CurrentArr[1]+CurrentArr[2])/3;  
                for(int i=0;i<3;i++)
                {
                    LineVoltage[Charger-1][i]=VoltageArr[i];
                }
                //Voltage[Charger-1] = {VoltageArr[0],VoltageArr[1],VoltageArr[2]};
              }
              DEBUGPORT.print("Voltage: "+String(LineVoltage[Charger-1][0],4)+" "+String(LineVoltage[Charger-1][1],4)+" "+String(LineVoltage[Charger-1][2],4));
          }
          else
          {
              DEBUGPORT.print("did not receive enough voltage data");
          }
          break;     
          break;
        default:
          break;
      }
    }
  }
  //Test only
  //Current[0][0] = 3.0;
  //Current[1][0] = 4.0;
  return Charger;
}


int readSerialOlimex() {
    int Charger;
     char input;
  if (millis()>(lastUpload+RSTTIMEOUT)){
   // softReset();
  }
  for(int k=0;k<10;k++){
  if (Serial1.available()) {
    input = Serial1.read();
    DEBUGPORT.print(input,HEX);
    DEBUGPORT.print(" ");
    if (bufpos<BUFSIZE)
    {
        buff[bufpos] = input;
        bufpos++;    
    }
    else
    {
        bufpos=0;
        DEBUGPORT.print("loopread>\tSerial Read Error!");
    }
    if (input == '\n') { // we hebben een regel binnen, tot aan \n   //  0x0A
      Charger = stringParse(buff, bufpos);
      //Maak de buffer leeg
      for (int i=0; i<BUFSIZE; i++)
      { buff[i] = 0;}
      bufpos = 0;
    }
  }
}
return Charger;
}