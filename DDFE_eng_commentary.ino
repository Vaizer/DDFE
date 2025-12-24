#include <SPI.h>
#include "mcp_can.h"

// Define pins for CS and INT from shields to arduino on HighSpeed and LowSpeed CAN
#define CAN_HS_CS_PIN 10
#define CAN_HS_INT    3

#define CAN_LS_CS_PIN 9
#define CAN_LS_INT    2

// Define CAN speed and frequency for MCP
#define CAN_HS_SPEED  CAN_250KBPS // CAN HS speed for pre-facelift (-2004)
//#define CAN_HS_SPEED  CAN_500KBPS // CAN HS speed for facelift (2005+)
#define CAN_LS_SPEED  CAN_125KBPS // CAN LS speed

#define MCP_HZ        MCP_16MHZ   // If MCP have 16MHZ quartz
//#define MCP_HZ        MCP_8MHZ	// If MCP have 8MHZ quartz

// Define needed IDs (2000-2001)
#define DIAG_ID     0x000FFFFEUL  // Diagnostics id 
#define PHM_ID      0x00400008UL  // id for phone module (PHM) (2001)
#define LCD_ID      0x00C0200EUL  // id for screen control on DIM (2001)
#define L_SWITCH_ID 0x8111300AUL  // id for left steering column switch (2001)
#define TCM_ID      0x80800005UL  // id for transmission module (TCM) (2001)
#define ECM_ID      0x80800021UL  // id for engine control unit (ECM) (2001)

#define MASK_ID     0x1FFFFFFFUL  // Mask for 29-bit packets
#define NULL_ID     0x00000000UL  // Null ID for excess filters

// Define IDs (2002)
//#define PHM_ID      0x00C00008UL  // id for phone module (PHM) (2002)
//#define LCD_ID      0x0220200EUL  // id for screen control on DIM (2002)

// Define IDs (2003-2004)
//#define PHM_ID
//#define LCD_ID      0x02A07428UL  // probably 2004 id for screen control on DIM

// Define IDs (2005+) 
//#define PHM_ID      0x01800008UL  // id for phone module (PHM) (FL)
//#define LCD_ID      0x02A0240EUL  // id for screen control on DIM (FL)
//#define L_SWITCH_ID 0x8131726CUL  // id for left steering column switch (FL)
//#define TCM_ID      0x81200005UL  // id transmission module (TCM) (FL)
//#define ECM_ID      0x81200021UL  // id for engine control unit (ECM) (FL)


#define CREDITS "      DDFE      by HPM and OLAF " 

/*  If you need to change output parameters - change PAGES_COUNT number, in PAGE_COMMAND delete\comment commands that you don't need 
    correct PAGE_TARGER_ID, and diagAnswerMonitoring */

#define PAGES_COUNT   14            	// Number of pages (parameters)
#define PARAMETER_UPDATE_INTERVAL 300	// Parameter update period (in ms)
#define LONG_PRESS_TIME 500				    // Duration of long press RESET (in ms)

// Array with commands for every page (for B5244T(3)/B5234T, AW55-51, 2000-2001)
unsigned const char PAGE_COMMAND[PAGES_COUNT][8] = {
  { 0xcc, 0x6e, 0xa5, 0x0c, 0x01, 0x00, 0x00, 0x00 }, // Command for ATF temperature
  { 0xcd, 0x7a, 0xa6, 0x10, 0x07, 0x01, 0x00, 0x00 }, // Command for atmospheric pressure
  { 0xcd, 0x7a, 0xa6, 0x12, 0x9d, 0x01, 0x00, 0x00 }, // Command for boost pressure
  { 0xcd, 0x7a, 0xa6, 0x10, 0xce, 0x01, 0x00, 0x00 }, // Command for intake temperature
  { 0xcd, 0x7a, 0xa6, 0x10, 0xae, 0x01, 0x00, 0x00 }, // Command for mass airflow
  { 0xcd, 0x7a, 0xa6, 0x10, 0x0a, 0x01, 0x00, 0x00 }, // Command for battery voltage
  { 0xcd, 0x7a, 0xa6, 0x10, 0xd8, 0x01, 0x00, 0x00 }, // Command for coolant temperature
  { 0xcd, 0x7a, 0xa6, 0x10, 0x01, 0x01, 0x00, 0x00 }, // Command for AC pressure
  { 0xcd, 0x7a, 0xa6, 0x11, 0x40, 0x01, 0x00, 0x00 }, // Command for vehicle speed
  { 0xcd, 0x7a, 0xa6, 0x11, 0xd7, 0x01, 0x00, 0x00 }, // Command for idle fuel trim
  { 0xcd, 0x7a, 0xa6, 0x11, 0xd9, 0x01, 0x00, 0x00 }, // Command for long-time fuel trim (L)
  { 0xcd, 0x7a, 0xa6, 0x11, 0xdb, 0x01, 0x00, 0x00 }, // Command for long-time fuel trim (H)
  { 0xcd, 0x7a, 0xa6, 0x10, 0x70, 0x01, 0x00, 0x00 }, // Command for short-time fuel trim
  { 0xcd, 0x7a, 0xa6, 0x10, 0xca, 0x01, 0x00, 0x00 }, // Command for misfires count
};

/*
// Commands for B5254T2, AW55-51, 2005
unsigned const char PAGE_COMMAND[PAGES_COUNT][8] = {  
  { 0xcc, 0x6e, 0xa5, 0x0c, 0x01, 0x00, 0x00, 0x00 }, // Command for ATF temperature
  { 0xcd, 0x7a, 0xa6, 0x10, 0x05, 0x01, 0x00, 0x00 }, // Command for atmospheric pressure
  { 0xcd, 0x7a, 0xa6, 0x10, 0xef, 0x01, 0x00, 0x00 }, // Command for boost pressure
  { 0xcd, 0x7a, 0xa6, 0x10, 0xaf, 0x01, 0x00, 0x00 }, // Command for intake temperature
  { 0xcd, 0x7a, 0xa6, 0x10, 0x9a, 0x01, 0x00, 0x00 }, // Command for mass airflow
  { 0xcd, 0x7a, 0xa6, 0x10, 0x0a, 0x01, 0x00, 0x00 }, // Command for battery voltage (not sure)
  { 0xcd, 0x7a, 0xa6, 0x10, 0xb8, 0x01, 0x00, 0x00 }, // Command for coolant temperature
  { 0xcd, 0x7a, 0xa6, 0x10, 0x01, 0x01, 0x00, 0x00 }, // Command for AC pressure
  { 0xcd, 0x7a, 0xa6, 0x11, 0x40, 0x01, 0x00, 0x00 }, // Command for vehicle speed (not sure)
  { 0xcd, 0x7a, 0xa6, 0x11, 0x4c, 0x01, 0x00, 0x00 }, // Command for long-time fuel trim (L)
  { 0xcd, 0x7a, 0xa6, 0x11, 0x4e, 0x01, 0x00, 0x00 }, // Command for long-time fuel trim (M)
  { 0xcd, 0x7a, 0xa6, 0x11, 0x50, 0x01, 0x00, 0x00 }, // Command for long-time fuel trim (H)
  { 0xcd, 0x7a, 0xa6, 0x10, 0x51, 0x01, 0x00, 0x00 }, // Command for short-time fuel trim 
  { 0xcd, 0x7a, 0xa6, 0x10, 0xad, 0x01, 0x00, 0x00 }, // Command for misfires count
};
*/

// Array of IDs for every page (pre-fl/2000-2001)
unsigned const long PAGE_TARGET_ID[PAGES_COUNT] = {
  0x80800005, // ID for TCM (0x00800005) (2000-2001/pre-fl)
  0x80800021, // ID for ECM (0x00800021) (2000-2001/pre-fl)
  0x80800021,  
  0x80800021,  
  0x80800021,
  0x80800021,
  0x80800021,
  0x80800021,
  0x80800021,
  0x80800021,
  0x80800021,
  0x80800021,
  0x80800021,
  0x80800021
};
/*
// Array of IDs for every page (facelift/2005+)
unsigned const long PAGE_TARGET_ID[PAGES_COUNT] = {
  0x81200005, // ID for TCM (0x01200005) 
  0x81200021, // ID for ECM (0x01200021)
  0x81200021,  
  0x81200021,  
  0x81200021,
  0x81200021,  
  0x81200021,
  0x81200021,
  0x81200021,
  0x81200021,
  0x81200021,
  0x81200021,
  0x81200021,
  0x81200021
};
*/

// Commands for LCD_ID to manage DIM's display
unsigned const char EnableLCD_1[8] = { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05 };  // Command to turn on screen (1 из 2) (pre-fl)
unsigned const char EnableLCD_2[8] = { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  // Command to turn on screen (2 из 2) (pre-fl)
//unsigned const char EnableLCD_1[8] = { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x35 };  // Command to turn on screen (1 из 2) (fl)
//unsigned const char EnableLCD_2[8] = { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31 };  // Command to turn on screen (2 из 2) (fl)
unsigned const char ClearLCD[8]    = { 0xE1, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  // Command to clear screen
unsigned const char DisableLCD[8]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04 };  // Commnad to disable screen


// Define CAN on MCP
const MCP_CAN CAN_HS(CAN_HS_CS_PIN);
const MCP_CAN CAN_LS(CAN_LS_CS_PIN);

// Variables for in- packets
unsigned long inId;         // Contains ID of in- packet
unsigned char inLength;     // Contains length of in- packet (always the same)
unsigned char inBuffer[8];  // Contains message itself

// Variables for RESET button to work 
bool          isButtonDown        = false;  // Check if button is pressed
unsigned long buttonDownTime      = 0;      // How long button have been pressed
unsigned long lastParamUpdateTime = 0;      // Time (in ms) since last parameter have been updated (also affect RESET button reading)
unsigned long currentProgTime     = 0;      // Program execution time at the moment (in ms)

int8_t        currentPage         = -1;     // Current page (obviously)


void setup() {
// Serial.begin(115200); 

// Define pins (for interrupts)
//  pinMode(CAN_HS_INT, INPUT_PULLUP);
//  pinMode(CAN_LS_INT, INPUT_PULLUP);

// Инициализируем CAN-шилды
  shieldInit();
//  Serial.println("All Shields init ok!");

  screenEnabler();                          // Enable and clear screen
  printOnLCD(CREDITS);                      // Display welcome message and disable screen 
  delay(1500);
  CAN_LS.sendMsgBuf(LCD_ID, 1, 8, DisableLCD);  
}


void loop() {
  buttonCheck();                            // Function for RESET button reading

  if (currentPage == -1) {	                // Don't send messages while screen is disabled
    return;
  }
  currentProgTime = millis();
  if (currentProgTime - lastParamUpdateTime < PARAMETER_UPDATE_INTERVAL) { 
    return;                                 // Don't send message, until set time is passed
  }
  lastParamUpdateTime = currentProgTime;    // Reset update time 

// Send diagnostic message in CAN HighSpeed
  CAN_HS.sendMsgBuf(DIAG_ID, 1, 8, PAGE_COMMAND[currentPage]);

// Monitor CAN for messages
  diagAnswerMonitoring();
}

void shieldInit() {                         // Shield initialization
// Init shields one by one with parameters: speed and frequency
  while (CAN_OK != CAN_HS.begin(MCP_STDEXT, CAN_HS_SPEED, MCP_HZ)) {
//    Serial.println("CAN HighSpeed: Init FAIL!");
    delay(100);
  }
//    Serial.println("CAN HighSpeed: Init OK!");
// Set masks and filters (all of them! it is necessary) on shield on CAN_HS
    CAN_HS.init_Mask(0, 1, MASK_ID);
     CAN_HS.init_Filt(0, 1, ECM_ID);
     CAN_HS.init_Filt(1, 1, TCM_ID);
    CAN_HS.init_Mask(1, 1, MASK_ID);
     CAN_HS.init_Filt(2, 1, DIAG_ID);
     CAN_HS.init_Filt(3, 1, NULL_ID);
     CAN_HS.init_Filt(4, 1, NULL_ID);
     CAN_HS.init_Filt(5, 1, NULL_ID);
    CAN_HS.setMode(MCP_NORMAL);             // After init set in normal mode (to send and recieve messages)

  while (CAN_OK != CAN_LS.begin(MCP_STDEXT, CAN_LS_SPEED, MCP_HZ)) {
  //    Serial.println("CAN LowSpeed: Init FAIL!");
    delay(100);
  }
//    Serial.println("CAN LowSpeed: Init OK!");
// Set masks and filters (all of them! it is necessary) on shield on CAN_LS
    CAN_LS.init_Mask(0, 1, MASK_ID);
     CAN_LS.init_Filt(0, 1, L_SWITCH_ID);
     CAN_LS.init_Filt(1, 1, PHM_ID);
    CAN_LS.init_Mask(1, 1, MASK_ID);
     CAN_LS.init_Filt(2, 1, LCD_ID);
     CAN_LS.init_Filt(3, 1, NULL_ID);
     CAN_LS.init_Filt(4, 1, NULL_ID);
     CAN_LS.init_Filt(5, 1, NULL_ID);
    CAN_LS.setMode(MCP_NORMAL);
}

void screenEnabler() {                      // Function for enable and clear screen
  CAN_LS.sendMsgBuf(LCD_ID, 1, 8, EnableLCD_1);
  delay(30);
  CAN_LS.sendMsgBuf(LCD_ID, 1, 8, EnableLCD_2);
  CAN_LS.sendMsgBuf(LCD_ID, 1, 8, ClearLCD);
}

void buttonCheck() {											  // Function for reading RESET button
  while (CAN_LS.checkReceive() == CAN_MSGAVAIL) {
    CAN_LS.readMsgBuf(&inId, &inLength, inBuffer);
    if (!isButtonDown && (inId == L_SWITCH_ID) && (inBuffer[4] == 0xC0)) {      // Compare needed ID and message ID (2000-2001)
//    if (!isButtonDown && (inId == L_SWITCH_ID) && (inBuffer[7] == 0xBF)) {      // Compare needed ID and message ID (2005+)
      isButtonDown = true;														                          // Flag for button press
      buttonDownTime = millis();												                        // Timer start for button press duration
    
    } else if (isButtonDown && (inId == L_SWITCH_ID) && (inBuffer[4] == 0x80)) {// Execute when button in unpressed (2000-2001)
//    } else if (isButtonDown && (inId == L_SWITCH_ID) && (inBuffer[?] == 0x??)){// Execute when button in unpressed (2005+ - don't know the command!)
      isButtonDown = false;
      buttonDownTime = millis() - buttonDownTime;
    
      if ((buttonDownTime >= LONG_PRESS_TIME) && (currentPage >= 0)) {       	  // Check if long press and disabled screen
        currentPage = -1;
        CAN_LS.sendMsgBuf(LCD_ID, 1, 8, DisableLCD);                        	  // Disable screen if long pressed
        lastParamUpdateTime = millis();											                    // Reset update timer
    
      } else if (buttonDownTime < LONG_PRESS_TIME) {					                  // Check if short press
        if (currentPage == -1) {							                                  // Check if screen is disabled
          screenEnabler();									                                    // If so - enable screen and set to 0 page
          currentPage = 0;
        } else {												                                        // If screen was already enabled - change page
          currentPage = (currentPage + 1) % PAGES_COUNT;
        }
        lastParamUpdateTime = millis();
      }
    }
  }
}


void diagAnswerMonitoring() {               // Function for monitoring CAN after diagnostic message have been sent
// Monitor CAN for packets available
    while (CAN_HS.checkReceive() == CAN_MSGAVAIL) {
    CAN_HS.readMsgBuf(&inId, &inLength, inBuffer);

    if (PAGE_TARGET_ID[currentPage] == inId) {
      long int resInt = 0;
      float resFloat = 0.0;
      switch (currentPage) {
/*  There is probability that some parameters would display incorrect because of int-char operations.
    As for solution - change in formulas inBuffer[] to String(inBuffer[]).toInt(), so it would be int.
    It could slow down program execution but not very much (maybe even negligible)*/
		  
      case 0:                                           // First page - ATF Temperature
        resInt = 256 * inBuffer[6] + inBuffer[7];
        printOnLCD(String(resInt) + " C ATF Temp");        // Display parameter with additional text on display
        break;

      case 1:                                           // Second page - Atmosphere Pressure
        resInt = inBuffer[5] * 5;
        printOnLCD(String(resInt) + " hPa AtmPres");
        break;

      case 2:                                           // Third page - Boost Pressure
        resInt = (256L * inBuffer[5] + inBuffer[6]) / 25.6;
        printOnLCD(String(resInt) + " hPa Boost");
        break;
		
      case 3:                                           // Fourth page - Intake Air Temperature
        resInt = inBuffer[5] * 0.75 - 48;
        printOnLCD(String(resInt) + " C IntAirTemp");
        break;

      case 4:                                           // Fifth page - Mass Airflow
        resFloat = (round(256L * inBuffer[5] + inBuffer[6])) * 0.1;
        printOnLCD(String(resFloat) + " kg/h MAF");
        break;

      case 5:                                           // Sixth page - Voltage
        resFloat = inBuffer[5] * 0.07;
        printOnLCD(String(resFloat) + " V Voltage");
        break;

      case 6:                                           // Seventh page - Coolant Temperature
        resInt = inBuffer[5] * 0.75 - 48;
        printOnLCD(String(resInt) + " C CoolantT");
        break;

      case 7:                                           // Eigth page - AC Pressure
        resFloat = inBuffer[5] * 13.54 - 176;
        printOnLCD(String(resFloat) + " kPa AC");
        break;

      case 8:                                           // Ninth page - Vehicle Speed
        resInt = inBuffer[5] * 1.25;
        printOnLCD(String(resInt) + " kmh Speed");
        break;

                                                        // On FL it is LTFT (L)
      case 9:                                          // Tenth page - Idle Fuel Trim
        resFloat = (round(256L * inBuffer[5] + inBuffer[6])) * 0.046875;
        printOnLCD(String(resFloat) + " p. Idle FT");
        break;

                                                        // On FL it is LTFT (M)
      case 10:                                          // Eleventh page - Long Term Fuel Trim (Low Power)
        resFloat = (round(256L * inBuffer[5] + inBuffer[6])) * 0.00003052;
        printOnLCD(String(resFloat) + " LTFT (L)");
        break;

      case 11:                                          // Twelveth page - Long Term Fuel Trim (High Power)
        resFloat = (round(256L * inBuffer[5] + inBuffer[6])) * 0.00003052;
        printOnLCD(String(resFloat) + " LTFT (H)");
        break;

      case 12:                                          // Thirteenth page - ShortTermCorrection
        resFloat = ((round(256L * inBuffer[5] + inBuffer[6])) * 2) / 65535.0;
        printOnLCD(String(resFloat) + " STFT");
        break;

      case 13:                                          // Fourteenth page - MisfireCounter
        resInt = 256L * inBuffer[5] + inBuffer[6];
        printOnLCD(String(resInt) + " Misfires");
        break;

      default:                                          // Error page
        printOnLCD("ERR PG " + String(currentPage));
        break;
      }
    }
  }
}

void printOnLCD(String input) {             // Send string on input to convert it in needed format for DIM to display it
  if (input.length() > 32)
    input = "Message length  too long";     // Check if string is too long

  while (input.length() < 32)               // Fill string with spaces until it would be 32 symbols
    input += " ";

  unsigned char msg[5][8] = {               // Copy string in array in needed format for full (2х16) message to display 
    { 0xA7, 0x00, input[0], input[1], input[2], input[3], input[4], input[5] },
    { 0x21, input[6], input[7], input[8], input[9], input[10], input[11], input[12] },
    { 0x22, input[13], input[14], input[15], input[16], input[17], input[18], input[19] },
    { 0x23, input[20], input[21], input[22], input[23], input[24], input[25], input[26] },
    { 0x65, input[27], input[28], input[29], input[30], input[31], 0x00, 0x00 }
  };

  CAN_LS.sendMsgBuf(LCD_ID, 1, 8, ClearLCD);// Cleaning screen before displaying new message so phantom symbols would't appear
  delay(10);

  for (int i = 0; i < 5; i++) {             // Send message on DIM's display with needed delays (should be 30ms, but it works with 10ms)
    CAN_LS.sendMsgBuf(PHM_ID, 1, 8, msg[i]);
    delay(10);
  }

}
