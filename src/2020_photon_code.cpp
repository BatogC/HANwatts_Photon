/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "d:/UNI_S5/new_particle_code/2020_photon_code/src/2020_photon_code.ino"

#include <MQTT.h>

#include <MFRC522.h>

#include "Commandparser.h"

#include <JsonParserGeneratorRK.h>
//#include "RFIDfunctions.h"
//extern void setupread();
int resetOlimex(String input);
int WifiSignal(String input);
int resetParticle(String input);
int progModeOlmx(String input);
void blinkRFIDled(int charger,int action);
int activeCharger();
int switchTest(String valueString);
int maxCurrentC1(String setPointStr);
int maxCurrentC2(String setPointStr);
int maxCurrentC1_test(unsigned int setPoint);
int maxCurrentC2_test(unsigned int setPoint);
String getUserIdAtSocket(int socket);
void allowUser_callback(byte* payload, unsigned int length);
int initRFID(String input);
bool readRFIDCard(int Charger);
void setup();
void loop();
#line 11 "d:/UNI_S5/new_particle_code/2020_photon_code/src/2020_photon_code.ino"
extern int readSerialOlimex();
extern bool readRFIDCard(int Charger);
extern float Current[2][3];
extern float Power[2][3];
extern float PhaseVoltage[2][3];
extern float LineVoltage[2][3];
extern float Energy[2];
extern float Frequency[2];
extern float CurrentList[20];
extern int numberOfZeroReadings[2];
void reconnect(void);
void callback(char* topic, byte* payload, unsigned int length);
void charToString(const char in[], String &out);
void getMeasure_callback(byte* payload, unsigned int length);

//! var to hold swiped RFID tag at first socket
String UIDtagCharger1="No ID";
//! var to hold swiped RFID tag at second socket
String UIDtagCharger2="No ID";

//! constant that sets for which Photon this program is intended
/*!
	For Photon 1 set it to 0, for Photon 2 set to 2. Any more and program would need to be edited.
*/
#define CHARGEROFFSET 0 //use 0 for socket 1&2, or 2 for socket 3&4, etc.
#define DEBUGPORT Serial
#define SIZEOFUSERLIST 2
//#define NUMBEROFMETERS 5
//SYSTEM_MODE(SEMI_AUTOMATIC);

#define SS_PIN_CHARGER1 A1
#define SS_PIN_CHARGER2 A2
#define RST_PIN A0
//Additional UART port not possible on D1,D2??
#define EXTRA_DIGITAL_BREAKOUT_1 D0 // Not used yet
#define EXTRA_DIGITAL_BREAKOUT_2 D1 // Not used yet
#define EXTRA_DIGITAL_BREAKOUT_3 D3 // Not used yet
#define WAKEUP_OLIMEX D2 //needed for in system programming (ISP)
#define RESET_OLIMEX D4 //needed for in system programming (ISP)
#define PILOT_FEEDBACK_CAR_1 A6 // To read the feedback signal from EV shield in paralel with the Olimex
#define PILOT_FEEDBACK_CAR_2 A7 // To read the feedback signal from EV shield in paralel with the Olimex
#define AUTHENTICATION_CAR1 D5 //Enable car 1 --> to Olimex A2
#define AUTHENTICATION_CAR2 D6 //Enable car 2 --> to Olimex A3
#define EXTRA D7 // No function yet --> to Olimex A4
//#define SERVER "80.113.19.23:8080"
//"hannl-lmrt-particle-api.azurewebsites.net"

STARTUP(WiFi.selectAntenna(ANT_EXTERNAL)); // selects the u.FL antenna //+
//SYSTEM_THREAD(ENABLED);

//MQTT setting
//byte server[] = {192,168,43,249};
//MQTT client(server, 1883, callback);

//! MQTT client details; do not set last number to over 512!
MQTT client("broker.hivemq.com", 1883, MQTT_DEFAULT_KEEPALIVE, callback, 512);
//char ID[] = "11111";

String test = "0";


int counter=1;
MFRC522 mfrc522_Charger1(SS_PIN_CHARGER1, RST_PIN);   // Create MFRC522 instance.
MFRC522 mfrc522_Charger2(SS_PIN_CHARGER2, RST_PIN);   // Create MFRC522 instance.
//! Holds latest start of new charge if charger is in use
unsigned long LatestStartTime[2]={0,0};
//! Holds last handled socket (0 for first socket)
bool handledCharger=0;
String ShareVar;
//String Current_Str="0";

//! var that holds the charging mode (TRUE = renewable)
bool TESTCASE = false;

//! var that holds answer from Pi but is unused now
ushort Pianswer=0;

//struct EMeter {
//    float PhaseVoltage[3];
//    float PhaseCurrent[3];
//    float PhasePower[3];
//    float Frequency;
//    unsigned long Time;
//};

// struct EVUser {
//     int Id;
//     String CarBrand;
//     String CarType;
//     int CarYear;
//     String Owner;
//     float BatteryCapacity;
//     String UIDtag;
//     int PendingCharger;
//     unsigned long StartTime;
// };

//EMeter EMeterData[NUMBEROFMETERS];
//EMeter EMeterData;
//``String EVListStr="";
String currentStr="";
//! Next timestamp to publish measurements in ms
unsigned int nextTime[2] = {30000,30000};    // Next time to pub measurements

//! Deprecated function to convert char to String - the String class already has one
void charToString(const char in[], String &out) {
    byte index = 0;
    const char *pointer = in;
    out = "";

    while (*pointer++) {
      out.concat(in[index++]);
      }
}

//! Sends reset signal to EV charger controller
int resetOlimex(String input) {
    digitalWrite(RESET_OLIMEX, LOW);
    delay(500);
    digitalWrite(RESET_OLIMEX, HIGH);
    return 1;
}

//! Return wifi strength
int WifiSignal(String input) {
    return WiFi.RSSI();
}

//! Resets Photon
int resetParticle(String input) {
    System.reset();
}

//! Sets Olimex into programming mode
int progModeOlmx(String input) {
    digitalWrite(WAKEUP_OLIMEX, HIGH);
    delay(500);
    resetOlimex("");
    delay(500);
    digitalWrite(WAKEUP_OLIMEX, LOW);
    return 1;
}

//! unused function to blink the Photon LED
void blinkRFIDled(int charger,int action) {
    //action=1  succesfull start new charge (charger is free and last stoped session > 20 sec ago)
    //action=2  charger is free, but you allready swiped the card in the last 20 sec (second swipe within 20sec)
    //action=3  charger is occupied by annother user
    //action=4  succesful stop this charge session
    //action=5  you just started a charge on this charger, but have another consecutive RFID read/swipe within 20 seconds
    //action=6  you are already charging at another charger
    //action=7  succesfull RFID read, but you are not in the userlist
    
    digitalWrite(D7,HIGH);
    delay(100);
    digitalWrite(D7,LOW);
    return;
}

//! Return 1 if socket 1 is used, 2 if socket 2 is used, and 3 if both are in use
int activeCharger() {
    int number = 0;
    for (int i=0; i<3; i++) {
        if (Current[0][i] != 0.0) {
            number += 1;
            break;
        }
    }
    
    for (int i=0; i<3; i++) {
        if (Current[1][i] != 0.0) {
            number += 2;
            break;
        }
    }
    
    return number;
}

//! Switches between renewable mode (-input "true") and manual setpoint mode
int switchTest(String valueString) {
    if (valueString == "true") {
        TESTCASE = true;
        return 1;
    }
    if (valueString == "false") {
        TESTCASE = false;
        maxCurrentC1("32");
        maxCurrentC2("32");
        return 0;
    }
}

//! Sets max Current output at socket 1 in manual mode
int maxCurrentC1(String setPointStr) {
    unsigned int setPoint = setPointStr.toInt();
    if (setPoint < 7)
        setPoint = 6;
    byte olimexMessage[4] = {0xFE,1,setPoint,0xFF};
    if (!TESTCASE) {
        Serial1.write(olimexMessage,4);
        DEBUGPORT.println("maxCurrentC"+String(CHARGEROFFSET+1)+">\tNew setpoint set at "+String(setPoint)+" Amps.");
        return 0;
    }
    return 1;
}

//! Sets max Current output at socket 2 in manual mode
int maxCurrentC2(String setPointStr) {
    unsigned int setPoint = setPointStr.toInt();
    if (setPoint < 7)
        setPoint = 6;
    byte olimexMessage[4] = {0xFE, 2, setPoint, 0xFF};
    if (!TESTCASE) {
        Serial1.write(olimexMessage,4);
        DEBUGPORT.println("maxCurrentC"+String(CHARGEROFFSET+2)+">\tNew setpoint set at "+String(setPoint)+" Amps.");
        return 0;
    }
    return 1;
}

//! Sets max Current output at socket 1/3 in renewable mode and publishes new setpoint at "HANevse/photonMaxC1" or C3
int maxCurrentC1_test(unsigned int setPoint) {
    if (setPoint < 7)
        setPoint = 6;
    byte olimexMessage[4] = {0xFE, 1, setPoint, 0xFF};
    if (TESTCASE) {
        Serial1.write(olimexMessage,4);
        DEBUGPORT.println("maxCurrentC"+String(CHARGEROFFSET+1)+">\tNew setpoint set at "+String(setPoint)+" Amps.");
        String topic_str = "HANevse/photonMaxC";
        topic_str.concat(CHARGEROFFSET+1);
        client.publish(topic_str, String(setPoint)); 
        return 0;
    }
    return 1;
}

//! Sets max Current output at socket 2/4 in renewable mode and publishes new setpoint at "HANevse/photonMaxC2" or C4
int maxCurrentC2_test(unsigned int setPoint) {
    if (setPoint < 7)
        setPoint = 6;
    byte olimexMessage[4] = {0xFE,2,setPoint,0xFF};
    if (TESTCASE) {
        Serial1.write(olimexMessage,4);
        DEBUGPORT.println("maxCurrentC"+String(CHARGEROFFSET+2)+">\tNew setpoint set at "+String(setPoint)+" Amps.");
        String topic_str = "HANevse/photonMaxC";
        topic_str.concat(CHARGEROFFSET+2);
        client.publish(topic_str, String(setPoint)); 
        return 0;
    }
    return 1;
}
/*
int AuthPinsHigh(String input)
{
    digitalWrite(AUTHENTICATION_CAR1, HIGH); //digitalWrite(D1,LOW);
    digitalWrite(AUTHENTICATION_CAR2, HIGH);//digitalWrite(D2,LOW);
    //digitalWrite(D7,HIGH);
    delay(10000);
    return 1;
}

int AuthPinsLow(String input)
{
    digitalWrite(AUTHENTICATION_CAR1, LOW); //digitalWrite(D1,LOW);
    digitalWrite(AUTHENTICATION_CAR2, LOW);//digitalWrite(D2,LOW);
    //digitalWrite(D7,LOW);
    delay(10000);
    return 1;
}*/

//! Returns RFID tag at the asked socket
String getUserIdAtSocket(int socket) {
    if (socket == 1+CHARGEROFFSET)
        return UIDtagCharger1;
    if (socket == 2+CHARGEROFFSET)
        return UIDtagCharger2;
    return "00";
}

//! Callback function to automatically set max Currents from MQTT message if in renewable mode
void getMeasure_callback(byte* payload, unsigned int length) {

    //int sockets = 1;
    //char p[length + 1];
    //memcpy(p, payload, length);
    
    //int setPoint =  (String(p)).toInt();
    
    int setP = 0;
    
    JsonParser parser1;
    parser1.clear();
    parser1.addData( (char*)(payload), length); 
    parser1.parse();

//    parser1.getOuterValueByKey("I1", EMeterData.PhaseCurrent[0]);
//    parser1.getOuterValueByKey("I2", EMeterData.PhaseCurrent[1]);
//    parser1.getOuterValueByKey("I3", EMeterData.PhaseCurrent[2]);
//    parser1.getOuterValueByKey("Sockets", sockets);

    parser1.getOuterValueByKey("setPoint", setP);
    
    unsigned int setPoint = setP;
    //client.publish("HANevse/photonMax", String(setPoint));
    // if (activeCharger() != 0)
    // {
      maxCurrentC1_test(setPoint); //Emeter3, I1
      delay(10);
      maxCurrentC2_test(setPoint); //Emeter3, I1
    // }
}


// void old_getMeasure_callback(byte* payload, unsigned int length) {
//     String data;
//     unsigned int from = 0;
//     unsigned int to = 0;
//    
//     char p[length + 1];
//     memcpy(p, payload, length);
//    
//     p[length] = NULL;
//     charToString(p, data);
//     for(int i=0; i<NUMBEROFMETERS; i++) {
//         //Read Phase Voltage
//         for(int j=0; j<3; j++) {
//             while (data[to]!='%') {
//                 to++;
//             }
//             EMeterData[i].PhaseVoltage[j] = (data.substring(from, to)).toFloat();
//             to++;
//             from = to;
//         }
//         //Read Phase Current
//         for(int j=0; j<3; j++) {
//             while (data[to]!='%') {
//                 to++;
//             }
//             EMeterData[i].PhaseCurrent[j] = (data.substring(from, to)).toFloat();
//             to++;
//             from = to;
//         }
//         //Read Phase Power
//         for(int j=0; j<3; j++) {
//             while (data[to]!='%') {
//                 to++;
//             }
//             EMeterData[i].PhasePower[j] = (data.substring(from, to)).toFloat();
//             to++;
//             from = to;
//         }
//         //Read Frequency
//         while (data[to]!='%') {
//             to++;
//         }
//         EMeterData[i].Frequency = (data.substring(from, to)).toFloat();
//         to++;
//         from = to;
//         //Read StartTime
//         while (data[to]!='%') {
//             to++;
//         }
//         EMeterData[i].Time = atol((data.substring(from, to)).c_str());
//         to++;
//         from = to;
//     }
//     time_t time = Time.now();
//     //DEBUGPORT.println(time);
//     DEBUGPORT.print("MQTT>\tReceive energy meter data from broker at: ");
//     DEBUGPORT.println(Time.format(time, TIME_FORMAT_DEFAULT));
//    
//     //Current_Str = String((int)(EMeterData[2].PhaseCurrent[0]));
//    
//     //Send current to OLIMEX
//     /*
//     if (AUTHENTICATION_CAR1) {
//         if (AUTHENTICATION_CAR2) {
//             maxCurrentC1_test((int)(EMeterData[2].PhaseCurrent[0]/2)); //Emeter3, I1
//             maxCurrentC2_test((int)(EMeterData[2].PhaseCurrent[0]/2)); //Emeter3, I1
//         }
//         else
//             maxCurrentC1_test((int)(EMeterData[2].PhaseCurrent[0])); //Emeter3, I1
//     }
//     else {
//         if (AUTHENTICATION_CAR2) {
//             maxCurrentC2_test((int)(EMeterData[2].PhaseCurrent[0])); //Emeter3, I1
//         }
//     }
//     */
//     if (activeCharger()==1) {
//         maxCurrentC1_test((int)(EMeterData[2].PhaseCurrent[0]+EMeterData[2].PhaseCurrent[1]+EMeterData[2].PhaseCurrent[2])); //Emeter3, I1
//     }
//     else if (activeCharger()==2) {
//         maxCurrentC2_test((int)(EMeterData[2].PhaseCurrent[0]+EMeterData[2].PhaseCurrent[1]+EMeterData[2].PhaseCurrent[2])); //Emeter3, I1
//     }
//     else {
//         maxCurrentC1_test((int)((EMeterData[2].PhaseCurrent[0]+EMeterData[2].PhaseCurrent[1]+EMeterData[2].PhaseCurrent[2])/2)); //Emeter3, I1
//         maxCurrentC2_test((int)((EMeterData[2].PhaseCurrent[0]+EMeterData[2].PhaseCurrent[1]+EMeterData[2].PhaseCurrent[2])/2)); //Emeter3, I1
//     }
// }

//! Callback function to process and execute approval or denial to charge from Pi, then MQTT publish reason to website GUI
void allowUser_callback(byte* payload, unsigned int length) {
    char payl[length+1];
    char *endchar;
        
    memcpy(payl, payload, length);
    payl[length] = NULL;
    int port = (int) strtol(payl, &endchar, 10);
    //action=1  successfully start new charge (charger is free and last stopped session > 20 sec ago)
    //action=2  charger is free, but you already swiped the card in the last 20 sec (second swipe within 20sec)
    //action=3  charger is occupied by another user
    //action=4  succesful stop of charge session
    //action=5  you just started a charge at this charger, but had another consecutive RFID swipe within 20 seconds
    //action=6  you are already charging at another charger
    //action=7  succesful RFID read, but you are not in the userlist
    int socketNr = port - 1 - CHARGEROFFSET;
    
    String topic_str = "HANevse/photonConverted/";
    topic_str.concat(port);

    endchar = endchar + 1;
    if (port == (1 + CHARGEROFFSET))
        port = AUTHENTICATION_CAR1;
    else if (port == (2 + CHARGEROFFSET))
        port = AUTHENTICATION_CAR2;
    else
        return; //port = EXTRA;
        
    int retPi = (int) strtol(endchar, &endchar, 10);
    Pianswer = retPi; 
    // if (Pianswer == 0)
        // Pianswer = 9;

    switch(retPi) {
        case 1:
            digitalWrite(port, HIGH);
            LatestStartTime[socketNr] = Time.now();
            client.publish(topic_str, "successful start new charge");
            break;
        case 2:
            client.publish(topic_str, "charger is free, but card was swiped in the last 20 sec");
            break;
        case 3:
            client.publish(topic_str, "charger is occupied by another user");
            break;
        case 4:
            digitalWrite(port, LOW);
            if (socketNr == 0)
                UIDtagCharger1="No ID";
            else if (socketNr == 1)
                UIDtagCharger2="No ID";
            client.publish(topic_str, "successful stop charge session");
            break;
        case 5:
            client.publish(topic_str, "consecutive RFID swipe within 20s of new charge start");
            break;
        case 6:
            client.publish(topic_str, "you are already charging at another charger");
            break;
        case 7:
            client.publish(topic_str, "you are in the userlist, but not verified by admin");
            break;
        case 8:
            client.publish(topic_str, "successful RFID read, but you are not in the userlist");
            break;
        default:
            client.publish(topic_str, "ERROR: unknown scenario");
        
    }
    
}

//! Function ran for each socket every 30s in main loop to send measurements through MQTT
void add_Measurement(float phaseVoltageL1, float phaseVoltageL2, float phaseVoltageL3, float currentL1, float currentL2, float currentL3, /* float Power, float Energy,*/ float Frequency, unsigned long Timestamp, int socketId=0, String userId="00") {
	
	//< Filter to skip if current values are impossibly high or under 0.1A
    //if ((currentL1 > 50.0)||(currentL2 > 50.0)||(currentL3 > 50.0)) 
    //    return;
    //if ((currentL1 < 0.1)&&(currentL2 < 0.1)&&(currentL3 < 0.1) )
    //    return;

    //This rounds floats to 3 decimal places
    // float newvar = (float)(((int)(oldvar * 1000 + .5)) / 1000); 
    
    // phaseVoltageL1 = (float)(((int)(phaseVoltageL1 * 1000 + .5)) / 1000);
    // phaseVoltageL2 = (float)(((int)(phaseVoltageL2 * 1000 + .5)) / 1000);
    // phaseVoltageL3 = (float)(((int)(phaseVoltageL3 * 1000 + .5)) / 1000);
    // currentL1 = (float)(((int)(currentL1 * 1000 + .5)) / 1000);
    // currentL2 = (float)(((int)(currentL2 * 1000 + .5)) / 1000);
    // currentL3 = (float)(((int)(currentL3 * 1000 + .5)) / 1000);
    // Frequency = (float)(((int)(Frequency * 1000 + .5)) / 1000);


    JsonWriterStatic<512> jsonMessage;     
        {
		JsonWriterAutoObject obj(&jsonMessage);
		// Add various types of data        
        jsonMessage.insertKeyValue("V1", phaseVoltageL1);
        jsonMessage.insertKeyValue("V2", phaseVoltageL2);
        jsonMessage.insertKeyValue("V3", phaseVoltageL3);
        jsonMessage.insertKeyValue("I1", currentL1);
        jsonMessage.insertKeyValue("I2", currentL2);
        jsonMessage.insertKeyValue("I3", currentL3);
        //jsonMessage.insertKeyValue("P", Power);
        //jsonMessage.insertKeyValue("E", Energy);
        jsonMessage.insertKeyValue("F", Frequency);
          
		jsonMessage.insertKeyValue("UserID", userId);
		jsonMessage.insertKeyValue("SocketID", socketId);
		jsonMessage.insertKeyValue("Time", Timestamp);
	    }


	for(int i=0; i<3; i++) {
		if(client.publish("HANevse/photonMeasure", jsonMessage.getBuffer())) {
			break;
		}
	}
}

//! Initialises RFID reader
int initRFID(String input) {
    //additional config for debugging RFID readers
    pinMode(SS_PIN_CHARGER1, OUTPUT);
	digitalWrite(SS_PIN_CHARGER1, HIGH);
	pinMode(SS_PIN_CHARGER2, OUTPUT);
	digitalWrite(SS_PIN_CHARGER2, HIGH);
  
    SPI.begin(D0);      // Initiate  SPI bus
    //Particle.process();
    delay(50);
    mfrc522_Charger1.PCD_Init();   // Initiate MFRC522
    delay(500);
    mfrc522_Charger2.PCD_Init();   // Initiate MFRC522
    //mfrc522_Charger1.PCD_SetAntennaGain(mfrc522.RxGain_max);
    mfrc522_Charger1.PCD_SetAntennaGain(mfrc522_Charger1.RxGain_max);
    mfrc522_Charger2.PCD_SetAntennaGain(mfrc522_Charger2.RxGain_max);
    
    DEBUGPORT.println("Approach your card to the reader...");
    DEBUGPORT.println();    
    return 1;
}

//! Checks and reads RFID tag at the asked socket, then MQTT publishes it for Pi
bool readRFIDCard(int Charger) {
   // DEBUGPORT.print("readCard>\t");
    bool Authorized = true;
    Pianswer = 0;
    if(Charger==1+CHARGEROFFSET)
    {
      // Look for new cards
        if ( ! mfrc522_Charger1.PICC_IsNewCardPresent()) 
        {
            return false;
        }
        // Select one of the cards
        if ( ! mfrc522_Charger1.PICC_ReadCardSerial()) 
        {
            return false;
        }
  
        //Show UID on serial monitor
        DEBUGPORT.print("readCard>\tUID tag on charger1:");
        String content = "";        
        //byte letter;
        for (byte i = 0; i < mfrc522_Charger1.uid.size; i++) 
        {
            DEBUGPORT.print(mfrc522_Charger1.uid.uidByte[i] < 0x10 ? " 0" : " ");
            DEBUGPORT.print(mfrc522_Charger1.uid.uidByte[i], HEX);
            content.concat(String(mfrc522_Charger1.uid.uidByte[i] < 0x10 ? " 0" : " "));
            content.concat(String(mfrc522_Charger1.uid.uidByte[i], HEX));
        }
        JsonWriterStatic<512> jsonMessage;

        //Authorized=testUser(content,Charger);
        UIDtagCharger1=content.substring(1); //??? why does it start at 1?

        {
		JsonWriterAutoObject obj(&jsonMessage);

		jsonMessage.insertKeyValue("UserId", UIDtagCharger1);
        jsonMessage.insertKeyValue("Charger", Charger);
		jsonMessage.insertKeyValue("StartTime", Time.now());
	    }
        client.publish("HANevse/updateUser", jsonMessage.getBuffer());
    }
    if(Charger==2+CHARGEROFFSET)
    {
    
        // Look for new cards
        if ( ! mfrc522_Charger2.PICC_IsNewCardPresent()) 
        {
            return false;
        }
        // Select one of the cards
        if ( ! mfrc522_Charger2.PICC_ReadCardSerial()) 
        {
            return false;
        }
        //DEBUGPORT.println("Read something on charger2");
        //Show UID on serial monitor
        DEBUGPORT.print("readCard>\tUID tag on charger2:");
        String content = "";        
        //byte letter;
        for (byte i = 0; i < mfrc522_Charger2.uid.size; i++) 
        {
            DEBUGPORT.print(mfrc522_Charger2.uid.uidByte[i] < 0x10 ? " 0" : " ");
            DEBUGPORT.print(mfrc522_Charger2.uid.uidByte[i], HEX);
            content.concat(String(mfrc522_Charger2.uid.uidByte[i] < 0x10 ? " 0" : " "));
            content.concat(String(mfrc522_Charger2.uid.uidByte[i], HEX));
        }
        //Authorized=testUser(content,Charger);
        UIDtagCharger2=content.substring(1);
        JsonWriterStatic<512> jsonMessage;

        {
		JsonWriterAutoObject obj(&jsonMessage);
		
		jsonMessage.insertKeyValue("UserId", UIDtagCharger2);
        jsonMessage.insertKeyValue("Charger", Charger);
		jsonMessage.insertKeyValue("StartTime", Time.now());
	    }
        client.publish("HANevse/updateUser", jsonMessage.getBuffer());
        
    }
    DEBUGPORT.println("");
       
    delay(500);
    // This whole function is not interrupted by callback() so Pianswer can't be changed in the meantime
    //client.publish("HANevse/checkupdateUser", String(Pianswer));  
     return Authorized;
}

//! Main function for MQTT client to check for new messages and execute callback functions
void callback(char* topic, byte* payload, unsigned int length) {
    test = "99";
	time_t time = Time.now();
    //DEBUGPORT.println(time);
    DEBUGPORT.print("MQTT>\tCallback function is called at: ");
    DEBUGPORT.println(Time.format(time, TIME_FORMAT_DEFAULT));

    if (CHARGEROFFSET==0) {
         if (strcmp(topic, "HANevse/allowUser")==0)
        {
            allowUser_callback(payload, length);
            //client.publish("HANevse/photonConverted", "test photon responds");
        }
        else
         if ( (strcmp(topic, "HANevse/energyMeter")==0) && TESTCASE )
        {
            getMeasure_callback(payload, length);
            //client.publish("HANevse/photonTest", "test photon responds");
        }
        else
         if (strcmp(topic, "HANevse/resetOlimex")==0)
        {
            char payl[length+1];
            memcpy(payl, payload, length);
            payl[length] = NULL;
            resetOlimex(payl);
        }
        else
         if (strcmp(topic, "HANevse/resetPhoton")==0)
        {
            resetParticle("1");
        }
        else
         if (strcmp(topic, "HANevse/switchTest1")==0)
        {
            char payl[length+1];
            memcpy(payl, payload, length);
            payl[length] = NULL;
            switchTest(payl);
        }
        else
         if ((strcmp(topic, "HANevse/maxC1")==0)  && !TESTCASE )
        {
            char payl[length+1];
            memcpy(payl, payload, length);
            payl[length] = NULL;
            maxCurrentC1(payl);
            //client.publish("HANevse/photonTest", "test photon responds");
        }
        else
         if ((strcmp(topic, "HANevse/maxC2")==0) && !TESTCASE )
        {
            char payl[length+1];
            memcpy(payl, payload, length);
            payl[length] = NULL;
            maxCurrentC2(payl);
            //client.publish("HANevse/photonTest", "test photon responds");
        }
    }
    else {
         if (strcmp(topic, "HANevse/allowUser")==0)
        {
            allowUser_callback(payload, length);
            //client.publish("HANevse/photonConverted", "test photon responds");
        }
        else
         if ( (strcmp(topic, "HANevse/energyMeter")==0) && TESTCASE )
        {
            getMeasure_callback(payload, length);
            //client.publish("HANevse/photonTest", "test photon responds");
        }
        else
         if (strcmp(topic, "HANevse/resetOlimex")==0)
        {
            char payl[length+1];
            memcpy(payl, payload, length);
            payl[length] = NULL;
            resetOlimex(payl);
        }
        else
         if (strcmp(topic, "HANevse/resetPhoton")==0)
        {
            resetParticle("1");
        }
        else
         if (strcmp(topic, "HANevse/switchTest2")==0)
        {
            char payl[length+1];
            memcpy(payl, payload, length);
            payl[length] = NULL;
            switchTest(payl);
        }
        else
         if ((strcmp(topic, "HANevse/maxC3")==0)  && !TESTCASE )
        {
            char payl[length+1];
            memcpy(payl, payload, length);
            payl[length] = NULL;
            maxCurrentC1(payl);
            //client.publish("HANevse/photonTest", "test photon responds");
        }
        else
         if ((strcmp(topic, "HANevse/maxC4")==0) && !TESTCASE )
        {
            char payl[length+1];
            memcpy(payl, payload, length);
            payl[length] = NULL;
            maxCurrentC2(payl);
            //client.publish("HANevse/photonTest", "test photon responds");
        }
    }
}

//! Function to reconnect to MQTT server if not connected and subscribe to needed topics
void reconnect(void) {
    while (!client.isConnected()) {
        DEBUGPORT.print("MQTT>\tConnecting to MQTT broker...");
        if (CHARGEROFFSET==0) {
            if (client.connect("EV-Photon1")) {
                DEBUGPORT.println("MQTT>\tConnected");
                //client.subscribe("HANevse/#", client.QOS2);
                client.subscribe("HANevse/energyMeter"); //+
                client.subscribe("HANevse/allowUser");
                
                client.subscribe("HANevse/resetOlimex");
                client.subscribe("HANevse/resetPhoton");
                client.subscribe("HANevse/switchTest1");
                client.subscribe("HANevse/maxC1");
                client.subscribe("HANevse/maxC2");
            }
            else {
                DEBUGPORT.println("MQTT>\tConnection failed");
                DEBUGPORT.println("MQTT>\tRetrying...");
                delay(10000);
            }
        }
        else if (CHARGEROFFSET==2){
            if (client.connect("EV-Photon2")) {
                DEBUGPORT.println("MQTT>\tConnected");
                //client.subscribe("HANevse/#", client.QOS2);
                client.subscribe("HANevse/energyMeter"); //+
                client.subscribe("HANevse/allowUser");
                
                client.subscribe("HANevse/resetOlimex");
                client.subscribe("HANevse/resetPhoton");
                 client.subscribe("HANevse/maxC3");
                 client.subscribe("HANevse/maxC4");
                 client.subscribe("HANevse/switchTest2");
            }
            else {
                DEBUGPORT.println("MQTT>\tConnection failed");
                DEBUGPORT.println("MQTT>\tRetrying...");
                delay(10000);
            }
        }
    }
}

//! Inital setup for pin assignments and serial links start
void setup() {
    DEBUGPORT.begin(115200); 
    Serial1.begin(9600);
    //DEBUGPORT.println(Voltage,5);
    //DEBUGPORT.println(String(Voltage,5));
    
    waitUntil(Particle.connected);
    
    pinMode(AUTHENTICATION_CAR1, OUTPUT); //pinMode(D1, OUTPUT); //Charger1_Authorized
    pinMode(AUTHENTICATION_CAR2, OUTPUT); //pinMode(D2, OUTPUT); //Charger2_Authorized
    pinMode(PILOT_FEEDBACK_CAR_1,INPUT);
    pinMode(PILOT_FEEDBACK_CAR_2,INPUT);
    pinMode(WAKEUP_OLIMEX, OUTPUT);
    pinMode(RESET_OLIMEX, OUTPUT);
    pinMode(D7, OUTPUT);
    
    digitalWrite(AUTHENTICATION_CAR1, LOW); //digitalWrite(D1,LOW);
    digitalWrite(AUTHENTICATION_CAR2, LOW);//digitalWrite(D2,LOW);
    digitalWrite(WAKEUP_OLIMEX, LOW);
    digitalWrite(RESET_OLIMEX, HIGH);
    digitalWrite(D7, LOW);
    
    initRFID(""); //+
    
    //Particle.process();  
    //resetOlimex("");  
    //Particle.process(); 

	Particle.function("switchTest",switchTest);
    Particle.function("maxCurrentC1",maxCurrentC1);
    Particle.function("maxCurrentC2",maxCurrentC2);
    Particle.function("resetOlimex",resetOlimex);
    Particle.function("progModeOlmx",progModeOlmx);
    Particle.function("resetParticl",resetParticle);
    //Particle.function("AuthPinsHigh",AuthPinsHigh);
    //Particle.function("AuthPinsLow",AuthPinsLow);
    Particle.function("WifiSignal",WifiSignal);
    Particle.function("initRFID",initRFID);
    Particle.variable("currentStr",currentStr);
    Particle.variable("ShareVar",ShareVar);
    //Particle.variable("Current", Current_Str);
    Particle.variable("Topic", test);
    Particle.process();
	
	//RGB.control(true);
    Time.zone(1); //Dutch time zone
}

//! Main running function that executes all other functions; runs over 5times/second
void loop() {
    //Check the connection to the Particle server
    if (Particle.connected() == false) {
        Particle.connect();
    }
	//Check the connection to the MQTT broker and let client take care of messages in buffers
    if (client.isConnected()) {
        client.loop();
    }
    else reconnect();
    
    Particle.process();
    //currentStr = String(Current[0][0],1)+" "+String( Current[0][1],1)+" "+String(Current[0][2],1)+" "+String(Current[1][0],1)+" "+String( Current[1][1],1)+" "+String(Current[1][2],1)+" "+String(Frequency[0],2);
//+    currentStr = String(Current[0][0],1)+" "+String( PhaseVoltage[0][1],1)+" "+String(LineVoltage[0][2],1)+" "+String(Power[1][0],1)+" "+String( Energy[1],1)+" "+String(Current[1][2],1)+" "+String(Frequency[0],2);
    //currentStr=String(Current[1][2],1)+" "+currentStr.substring(0, max(200, currentStr.length()))
    //currentStr = String(CurrentList[0],1)+" "+String(CurrentList[1],1)+" "+String(CurrentList[2],1)+" "+String(CurrentList[3],1)+" "+String(CurrentList[4],1)+" "+String(CurrentList[5],1)+" "+String(CurrentList[6],1)+" "+String(CurrentList[7],1)+" "+String(CurrentList[8],1)+" "+String(CurrentList[9],1)+" "+String(CurrentList[10],1)+" "+String(CurrentList[11],1)+" "+String(CurrentList[12],1)+" "+String(CurrentList[13],1)+" "+String(CurrentList[14],1)+" "+String(CurrentList[15],1)+" "+String(CurrentList[16],1)+" "+String(CurrentList[17],1)+" "+String(CurrentList[18],1)+" "+String(CurrentList[19],1);
    
    //int Charger =1; //+
    
    //Read measurements from Olimex and save for which socket
    int Charger = readSerialOlimex() + CHARGEROFFSET; //+
    Particle.process();
    // !!!! This runs multiple times a second for some reason (serial) 
    // if(counter>10){
	// 	counter = 0;
	// !!!!	DEBUGPORT.println("LatestStartTime>\t"+String(LatestStartTime[0])+", "+String(LatestStartTime[1]));
	// 	DEBUGPORT.println(String(Current[1][0]+ Current[1][1]+ Current[1][2]));
    // }
    // counter++;

    // store new measurement value if it is received correctly from energymeter (via the Olimex).
    if(millis()>nextTime[handledCharger] && (Charger==1+CHARGEROFFSET || Charger==2+CHARGEROFFSET)) //+ all the if{}
    {
        Particle.process();
        //getUserIdAtSocket(Charger)
        int tempCharger = Charger;
        Charger = handledCharger + 1;
        //if(((activeCharger()==Charger) || (activeCharger() == 3)) && (getUserIdAtSocket(Charger)!="00"))
        //{
            //getUserIdAtSocket(Charger+CHARGEROFFSET);
            add_Measurement(PhaseVoltage[Charger-1][0], PhaseVoltage[Charger-1][1], PhaseVoltage[Charger-1][2], Current[Charger-1][0], Current[Charger-1][1], Current[Charger-1][2], /*Power[Charger-1][0]+Power[Charger-1][1]+Power[Charger-1][2], Energy[Charger-1],*/ Frequency[Charger-1], Time.now(), Charger+CHARGEROFFSET, getUserIdAtSocket(Charger+CHARGEROFFSET));
        //}
        Charger = tempCharger;
        nextTime[handledCharger] = millis() + 30000; //every 30 sec
    }
    
//     run loop very often to check new RFID cards
    Particle.process(); //+
    bool Authorized_Charger1=readRFIDCard(1+CHARGEROFFSET); //+
    delay(5);
    //if (Pianswer == 1 || Pianswer ==4 )    
    //    Authorized_Charger1 = TRUE;
    bool Authorized_Charger2=readRFIDCard(2+CHARGEROFFSET); //+
    //delay(5);
    //if (Pianswer == 1 || Pianswer ==4 )  //+    
    //      Authorized_Charger2 = TRUE;     //+
    
    
    //DEBUGPORT.println(Current[0][0]+ Current[0][1]+ Current[0][2],4);
    //DEBUGPORT.println(String(LatestStartTime[0]+60));
    //DEBUGPORT.println(String(Time.now()));
    //DEBUGPORT.println((LatestStartTime[0] + 60 < Time.now()),DEC);
    //if ((LatestStartTime[0] + 60 < Time.now()) && (Current[0][0]+ Current[0][1]+ Current[0][2]) < 1)
    //if (((numberOfZeroReadings[0]>10 && (LatestStartTime[0] + 60 < Time.now()))|| ((Time.now()<LatestStartTime[0] + 70)&&(LatestStartTime[0] + 60 < Time.now()))) && (Current[0][0]+ Current[0][1]+ Current[0][2]) < 1)
    
    // if 10+ Zero current readings have been taken or last start of new charge was over 1min ago and total Current is under 1A for first socket stop charge and reset StartTIme var
    if( ((numberOfZeroReadings[0]>10)||(LatestStartTime[0] + 70 > Time.now()) )&& (LatestStartTime[0] + 60 < Time.now()) && (Current[0][0]+ Current[0][1]+ Current[0][2]) < 1)
    {   
        //timeout with current almost zero
        DEBUGPORT.println("Timeout charger"+String(CHARGEROFFSET+1));
        digitalWrite(AUTHENTICATION_CAR1,LOW);
        LatestStartTime[0]=2147483548;
    }
    //DEBUGPORT.println(Current[1][0]+ Current[1][1]+ Current[1][2],4);
    //DEBUGPORT.println(String(LatestStartTime[1]+60));
    //DEBUGPORT.println(String(Time.now()));
    //DEBUGPORT.println((LatestStartTime[1] + 60 < Time.now()),DEC);
    
    // if 10+ Zero current readings have been taken or last start of new charge was over 1min ago and total Current is under 1A for second socket stop charge and reset StartTIme var
    if( ((numberOfZeroReadings[1]>10)||(LatestStartTime[1] + 70 > Time.now()) )&& (LatestStartTime[1] + 60 < Time.now()) && (Current[1][0]+ Current[1][1]+ Current[1][2]) < 1)
    {
        //timeout with current almost zero
        DEBUGPORT.println("Timeout charger"+String(CHARGEROFFSET+2));
        digitalWrite(AUTHENTICATION_CAR2,LOW);
        //digitalWrite(D7,LOW);
        LatestStartTime[1]=2147483548;
    }
    delay(100);


    //Reset the UIDtag if there is no car charging and last wsipe was over 1min ago at first socket
    if ((activeCharger()!=1)&&(activeCharger()!=3)&&(UIDtagCharger1!="No ID")&& (LatestStartTime[0] + 60 < Time.now()) ){
        
        JsonWriterStatic<512> jsonMessage;

        {
		JsonWriterAutoObject obj(&jsonMessage);
		
		jsonMessage.insertKeyValue("UserId", UIDtagCharger1);
        jsonMessage.insertKeyValue("Charger", (1 + CHARGEROFFSET));
		jsonMessage.insertKeyValue("StartTime", Time.now());
	    }
        client.publish("HANevse/updateUser", jsonMessage.getBuffer());

        UIDtagCharger1="No ID";
    }

        
    if ((activeCharger()!=2)&&(activeCharger()!=3)&&(UIDtagCharger2!="No ID")&& (LatestStartTime[1] + 60 < Time.now()) ){
        
        JsonWriterStatic<512> jsonMessage;

        {
		JsonWriterAutoObject obj(&jsonMessage);
		
		jsonMessage.insertKeyValue("UserId", UIDtagCharger2);
        jsonMessage.insertKeyValue("Charger", (2 + CHARGEROFFSET));
		jsonMessage.insertKeyValue("StartTime", Time.now());
	    }
        client.publish("HANevse/updateUser", jsonMessage.getBuffer());

        UIDtagCharger2="No ID";
    }
            
    handledCharger = !handledCharger;
}
