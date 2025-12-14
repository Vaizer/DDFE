#include <SPI.h>
#include "mcp_can.h"

// Обозначаем пины подключения CS и INT на шильдах к ардуине к HighSpeed и LowSpeed CAN соответственно
#define CAN_HS_CS_PIN 10
#define CAN_HS_INT    3

#define CAN_LS_CS_PIN 9
#define CAN_LS_INT    2

// Прописываем скорость работы CAN и частоте работы для MCP
#define CAN_HS_SPEED  CAN_250KBPS  // Скорость CAN HS для дореста
//#define CAN_HS_SPEED  CAN_500KBPS  // Скорость CAN HS для реста 
#define CAN_LS_SPEED  CAN_125KBPS	// Скорость CAN LS 

#define MCP_HZ        MCP_16MHZ // Если частота работы MCP 16MHZ
//#define MCP_HZ        MCP_8MHZ // Если частота работы MCP 8MHZ

// Прописываем используемые ID (2001г)
#define DIAG_ID     0x000FFFFEUL  // Диагностический id 
#define PHM_ID      0x00400008UL  // id модуля телефона (PHM) (2001)
#define LCD_ID      0x00C0200EUL  // id управления экраном DIM (2001)
#define L_SWITCH_ID 0x8111300AUL  // id левого подрулевого переключателя (2001)
#define TCM_ID      0x80800005UL  // id блока АКПП (TCM) (2001)
#define ECM_ID      0x80800021UL  // id ЭБУ двигателя (ECM) (2001)

#define MASK_ID     0x1FFFFFFFUL  // Маска для 29-битных пакетов
#define NULL_ID     0x00000000UL  // Нулевой ID для лишних фильтров
// Идентификаторы для 2002г
//#define PHM_ID      0x00C00008UL  // id модуля телефона (PHM) (2002)
//#define LCD_ID      0x0220200EUL  // id управления экраном DIM (2002)

// Идентификаторы для 2003-2004г
//#define PHM_ID
//#define LCD_ID      0x02A07428UL  // возможно 2004г id управления экраном DIM

// Идентификаторы для 2005г+ (не точно)
//#define PHM_ID      0x01800008UL  // id модуля телефона (PHM) (рест)
//#define LCD_ID      0x02A0240EUL  // id управления экраном DIM (рест)
//#define L_SWITCH_ID 0x8131726CUL  // id SWM (рест)
//#define TCM_ID      0x81200005UL  // id блока АКПП (TCM) (2001)
//#define ECM_ID      0x81200021UL  // id ЭБУ двигателя (ECM) (2001)


#define CREDITS "      DDFE      by HPM and OLAF " 

// Для изменения количества страниц необходимо изменить значение PAGES_COUNT, скорректировать команды в PAGE_COMMAND, 
// внести ID ответа в PAGE_TARGET_ID, в loop() внутри switch добавить/убрать страницы в соответствии

#define PAGES_COUNT   14            // Общее количество страниц (параметров)
#define TICKS         30000         // Количество циклов считывания кнопки из CAN

// Массив с командами для каждой страницы (команды для B5244T(3)/B5234T, AW55-51, 2001)
unsigned const char PAGE_COMMAND[PAGES_COUNT][8] = {
  { 0xcc, 0x6e, 0xa5, 0x0c, 0x01, 0x00, 0x00, 0x00 }, // Команда на запрос температуры ATF
  { 0xcd, 0x7a, 0xa6, 0x10, 0x07, 0x01, 0x00, 0x00 }, // Команда на запрос атмосферного давления
  { 0xcd, 0x7a, 0xa6, 0x12, 0x9d, 0x01, 0x00, 0x00 }, // Команда на запрос давления наддува
  { 0xcd, 0x7a, 0xa6, 0x10, 0xce, 0x01, 0x00, 0x00 }, // Команда на запрос температуры впускного воздуха
  { 0xcd, 0x7a, 0xa6, 0x10, 0xae, 0x01, 0x00, 0x00 }, // Команда на запрос расхода воздуха MAF
  { 0xcd, 0x7a, 0xa6, 0x10, 0x0a, 0x01, 0x00, 0x00 }, // Команда на запрос напряжения батареи
  { 0xcd, 0x7a, 0xa6, 0x10, 0xd8, 0x01, 0x00, 0x00 }, // Команда на запрос температуры антифриза
  { 0xcd, 0x7a, 0xa6, 0x10, 0x01, 0x01, 0x00, 0x00 }, // Команда на запрос давления в контуре кондиционера
  { 0xcd, 0x7a, 0xa6, 0x11, 0x40, 0x01, 0x00, 0x00 }, // Команда на запрос скорости авто
  { 0xcd, 0x7a, 0xa6, 0x11, 0xd7, 0x01, 0x00, 0x00 }, // Команда на запрос коррекции на холостом ходу
  { 0xcd, 0x7a, 0xa6, 0x11, 0xd9, 0x01, 0x00, 0x00 }, // Команда на запрос долговременной коррекции топлива (L)
  { 0xcd, 0x7a, 0xa6, 0x11, 0xdb, 0x01, 0x00, 0x00 }, // Команда на запрос долговременной коррекции топлива (H)
  { 0xcd, 0x7a, 0xa6, 0x10, 0x70, 0x01, 0x00, 0x00 }, // Команда на запрос кратковременной коррекции топлива
  { 0xcd, 0x7a, 0xa6, 0x10, 0xca, 0x01, 0x00, 0x00 }, // Команда на запрос количества пропусков зажигания
};

/*
// Команды для B5254T2, AW55-51, 2005
unsigned const char PAGE_COMMAND[PAGES_COUNT][8] = {  
  { 0xcc, 0x6e, 0xa5, 0x0c, 0x01, 0x00, 0x00, 0x00 }, // Команда на запрос температуры ATF
  { 0xcd, 0x7a, 0xa6, 0x10, 0x05, 0x01, 0x00, 0x00 }, // Команда на запрос атмосферного давления
  { 0xcd, 0x7a, 0xa6, 0x10, 0xef, 0x01, 0x00, 0x00 }, // Команда на запрос давления наддува
  { 0xcd, 0x7a, 0xa6, 0x10, 0xaf, 0x01, 0x00, 0x00 }, // Команда на запрос температуры впускного воздуха
  { 0xcd, 0x7a, 0xa6, 0x10, 0x9a, 0x01, 0x00, 0x00 }, // Команда на запрос расхода воздуха MAF
  { 0xcd, 0x7a, 0xa6, 0x10, 0x0a, 0x01, 0x00, 0x00 }, // Команда на запрос напряжения батареи (тут не точно)!!
  { 0xcd, 0x7a, 0xa6, 0x10, 0xb8, 0x01, 0x00, 0x00 }, // Команда на запрос температуры антифриза
  { 0xcd, 0x7a, 0xa6, 0x10, 0x01, 0x01, 0x00, 0x00 }, // Команда на запрос давления в контуре кондиционера
  { 0xcd, 0x7a, 0xa6, 0x11, 0x40, 0x01, 0x00, 0x00 }, // Команда на запрос скорости авто (тут не точно)!!
  { 0xcd, 0x7a, 0xa6, 0x11, 0x4c, 0x01, 0x00, 0x00 }, // Команда на запрос долговременной коррекции топлива (L)
  { 0xcd, 0x7a, 0xa6, 0x11, 0x4e, 0x01, 0x00, 0x00 }, // Команда на запрос долговременной коррекции топлива (M)
  { 0xcd, 0x7a, 0xa6, 0x11, 0x50, 0x01, 0x00, 0x00 }, // Команда на запрос долговременной коррекции топлива (H)
  { 0xcd, 0x7a, 0xa6, 0x10, 0x51, 0x01, 0x00, 0x00 }, // Команда на запрос кратковременной коррекции топлива 
  { 0xcd, 0x7a, 0xa6, 0x10, 0xad, 0x01, 0x00, 0x00 }, // Команда на запрос количества пропусков зажигания
};
*/

// Массив с ID для каждой страницы
unsigned const long PAGE_TARGET_ID[PAGES_COUNT] = {
  0x80800005, // ID в ответе на запрос температуры ATF (TCM) (0x00800005) (2001г/дорест)
  0x80800021, // ID в ответе на запрос всего остального (ECM) (0x00800021) (2001г/дорест)
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
// ID ответов для рестайлинга
unsigned const long PAGE_TARGET_ID[PAGES_COUNT] = {
  0x81200005, // ID в ответе на запрос температуры ATF (TCM) (0x01200005) 
  0x81200021, // ID в ответе на запрос всего остального (ECM) (0x01200021)
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

// Эти запросы использовать с LCD_ID для управления дисплеем
unsigned const char EnableLCD_1[8] = { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05 };  // Команда на включение экрана (1 из 2) (дорест)
unsigned const char EnableLCD_2[8] = { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  // Команда на включение экрана (2 из 2) (дорест)
//unsigned const char EnableLCD_1[8] = { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x35 };  // Команда на включение экрана (1 из 2) (рест)
//unsigned const char EnableLCD_2[8] = { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31 };  // Команда на включение экрана (2 из 2) (рест)
unsigned const char ClearLCD[8]    = { 0xE1, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  // Команда на очистку экрана
unsigned const char DisableLCD[8]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04 };  // Команда на выключение экрана


// Обозначаем CAN-шины
const MCP_CAN CAN_HS(CAN_HS_CS_PIN);
const MCP_CAN CAN_LS(CAN_LS_CS_PIN);

// Переменные для работы с входящими пакетами
unsigned long inId;         // Хранит ID входящего пакета
unsigned char inLength;     // Хранит длину входящего пакета (всегда одинакова)
unsigned char inBuffer[8];  // Хранит само сообщение в пакете данных

bool          isDown            = false;  // Проверка факта нажатия на кнопку
unsigned long isDownTime        = 0;      // Переменная для времени, на сколько фактически кнопка зажата
unsigned long buttonPressTime   = 0;      // Переменная для счетчика переключения
unsigned long longPressTime     = 1000;   // Время (в мс) долгого нажатия кнопки
uint16_t      buttonCheckTick   = 0;      // Переменная для счетчика циклов считывания кнопки
int8_t        currentPage       = -1;     // Переменная для хранения текущей страницы


void setup() {
// Serial.begin(115200); 

// Прописываем функции пинов (для прерываний)
//  pinMode(CAN_HS_INT, INPUT_PULLUP);
//  pinMode(CAN_LS_INT, INPUT_PULLUP);

// Инициализируем CAN-шилды
  shieldInit();
//  Serial.println("All Shields init ok!");

  screenEnabler();                              // Включение и очистка дисплея
  PrintOnLCD(CREDITS);                          // Вывод на экран приветствия
  delay(1500);
  CAN_LS.sendMsgBuf(LCD_ID, 1, 8, DisableLCD);  // Выключение экрана после включения
}


void loop() {
// Счетчик переключения страниц по нажатию кнопки RESET подрулевого переключателя 
  buttonCheckTick++;            // Счетчик количества циклов считывания кнопки 
  buttonCheck();                // Функция считывания кнопки из CAN
  if (buttonCheckTick < TICKS)  // Продолжать считывание кнопки до достижения заданного количества циклов
    return;
  buttonCheckTick = 0;          // Обнуление счетчика 
  if (currentPage == -1)        // Не запускать запросы и вывод на экран на нулевом экране
    return;

// Направляем диагностический запрос в CAN_HS
  CAN_HS.sendMsgBuf(DIAG_ID, 1, 8, PAGE_COMMAND[currentPage]);

// Мониторим шину на предмет наличия пакетов в ней
  diagAnswerMonitoring();
}

void shieldInit() {                                       // Функция инициализации CAN-шильдов
// Инициализируем по очереди оба CAN-шилда, с прописыванием их параметров: скорости и частоты кварца
  if (CAN_HS.begin(MCP_STDEXT, CAN_HS_SPEED, MCP_HZ) == CAN_OK) {
//    Serial.println("CAN HighSpeed: Init OK!");
// Задаем маски и фильтры в MCP для CAN_HS
    CAN_HS.init_Mask(0, 1, MASK_ID);
     CAN_HS.init_Filt(0, 1, ECM_ID);
     CAN_HS.init_Filt(1, 1, TCM_ID);
    CAN_HS.init_Mask(1, 1, MASK_ID);
     CAN_HS.init_Filt(2, 1, DIAG_ID);
     CAN_HS.init_Filt(3, 1, NULL_ID);
     CAN_HS.init_Filt(4, 1, NULL_ID);
     CAN_HS.init_Filt(5, 1, NULL_ID);

    CAN_HS.setMode(MCP_NORMAL);  // После инициализации переводим в нормальный режим (возможность и приема и передачи сообщения)
  } else {
//    Serial.println("CAN HighSpeed: Init FAIL!");
    delay(100);
  }

  if (CAN_LS.begin(MCP_STDEXT, CAN_LS_SPEED, MCP_HZ) == CAN_OK) {
//    Serial.println("CAN LowSpeed: Init OK!");
// Задаем маски и фильтры в MCP для CAN_LS
    CAN_LS.init_Mask(0, 1, MASK_ID);
     CAN_LS.init_Filt(0, 1, L_SWITCH_ID);
     CAN_LS.init_Filt(1, 1, PHM_ID);
    CAN_LS.init_Mask(1, 1, MASK_ID);
     CAN_LS.init_Filt(2, 1, LCD_ID);
     CAN_LS.init_Filt(3, 1, NULL_ID);
     CAN_LS.init_Filt(4, 1, NULL_ID);
     CAN_LS.init_Filt(5, 1, NULL_ID);

    CAN_LS.setMode(MCP_NORMAL);
  } else {
//    Serial.println("CAN LowSpeed: Init FAIL!");
    delay(100);
  }
}

void screenEnabler() {                                            // Функция включения и очистки экрана
  CAN_LS.sendMsgBuf(LCD_ID, 1, 8, EnableLCD_1);
  delay(30);
  CAN_LS.sendMsgBuf(LCD_ID, 1, 8, EnableLCD_2);
  CAN_LS.sendMsgBuf(LCD_ID, 1, 8, ClearLCD);
}

void buttonCheck() {														                            // Функция проверки нажатия кнопки RESET
  while (CAN_LS.checkReceive() == CAN_MSGAVAIL) {
    CAN_LS.readMsgBuf(&inId, &inLength, inBuffer);
    if (!isDown && (inId == L_SWITCH_ID) && (inBuffer[4] == 0xC0)) {        // Сравниваем искомый ID и бит данных (2000-2001)
//    if (!isDown && (inId == L_SWITCH_ID) && (inBuffer[7] == 0xBF)) {        // Сравниваем искомый ID и бит данных (рест)
      isDown = true;														                            // Флаг нажатой клавиши
      isDownTime = millis();												                        // Начало отсчета времени нажатия клавиши
    
    } else if (isDown && (inId == L_SWITCH_ID) && (inBuffer[4] == 0x80)) {	// Выполнение при отжатии клавиши (2000-2001)
//    } else if (isDown && (inId == L_SWITCH_ID) && (inBuffer[?] == 0x??)){	// Выполнение при отжатии клавиши (рест - команда без нажатия кнопки неизвестна)
      isDown = false;
      buttonPressTime = millis() - isDownTime;
    
      if ((buttonPressTime >= longPressTime) && (currentPage >= 0)) {       // Проверка на долгое нажатие и уже включенный экран
        currentPage = -1;
        CAN_LS.sendMsgBuf(LCD_ID, 1, 8, DisableLCD);                        // Выключение экрана при соответствии
        buttonCheckTick = TICKS;											                      // Перевод счетчика для дальнейшей работы программы в loop()
    
      } else if (buttonPressTime < longPressTime) {					            		// Проверка на короткое нажатие 
        if (currentPage == -1) {							                      				// Проверка на выключенный экран
          screenEnabler();									                        				// Включить экран и перейти на первую страницу
          currentPage = 0;
        } else {												                              			// Если экран уже включен, то перелистнуть страницу
          currentPage = (currentPage + 1) % PAGES_COUNT;
        }
        buttonCheckTick = TICKS;
      }
    }
  }
}


void diagAnswerMonitoring() {                                 // Функция мониторинга CAN_HS после отправки диагностического запроса
// Мониторим шину на предмет наличия пакетов в ней
    while (CAN_HS.checkReceive() == CAN_MSGAVAIL) {
    CAN_HS.readMsgBuf(&inId, &inLength, inBuffer);

    if (PAGE_TARGET_ID[currentPage] == inId) {
      int resInt = 0;
      float resFloat = 0.0;
      switch (currentPage) {

      case 0:                                           // Первый экран - ATF Temperature
        resInt = 256 * String(inBuffer[6]).toInt() + String(inBuffer[7]).toInt();
        PrintOnLCD(String(resInt) + " C ATF Temp");        // Вывод температуры АКПП с подписью на экран
        break;

      case 1:                                           // Второй экран - Atmosphere Pressure
        resInt = String(inBuffer[5]).toInt() * 5;
        PrintOnLCD(String(resInt) + " hPa AtmPres");      // Вывод атмо давления с подписью на экран
        break;

      case 2:                                           // Третий экран - Boost Pressure
        resInt = (256 * String(inBuffer[5]).toInt() + String(inBuffer[6]).toInt()) / 25.6;
        PrintOnLCD(String(resInt) + " hPa Boost");        // Вывод давления наддува с подписью на экран
        break;
		
      case 3:                                           // Четвертый экран - Intake Air Temperature
        resInt = String(inBuffer[5]).toInt() * 0.75 - 48;
        PrintOnLCD(String(resInt) + " C IntAirTemp");     // Вывод температуры впускного воздуха с подписью на экран
        break;

      case 4:                                           // Пятый экран - Mass Airflow
        resFloat = (round(256 * String(inBuffer[5]).toInt() + String(inBuffer[6]).toInt())) * 0.1;
        PrintOnLCD(String(resFloat) + " kg/h MAF");       // Вывод массового расхода воздуха с подписью на экран
        break;

      case 5:                                           // Шестой экран - Voltage
        resFloat = String(inBuffer[5]).toInt() * 0.07;
        PrintOnLCD(String(resFloat) + " V Voltage");      // Вывод напряжения бортовой сети с подписью на экран
        break;

      case 6:                                           // Седьмой экран - Coolant Temperature
        resInt = String(inBuffer[5]).toInt() * 0.75 - 48;
        PrintOnLCD(String(resInt) + " C CoolantT");       // Вывод температуры охлаждающей жидкости с подписью на экран
        break;

      case 7:                                           // Восьмой экран - AC Pressure
        resFloat = String(inBuffer[5]).toInt() * 13.54 - 176;
        PrintOnLCD(String(resFloat) + " kPa AC");         // Вывод давления контура кондиционера с подписью на экран
        break;

      case 8:                                           // Девятый экран - Vehicle Speed
        resInt = String(inBuffer[5]).toInt() * 1.25;
        PrintOnLCD(String(resInt) + " kmh Speed");        // Вывод скорости авто с подписью на экран
        break;

                                                        // На ресте здесь LTFT (L)
      case 9:                                          // Десятый экран - Idle Fuel Trim
        resFloat = (round(256 * String(inBuffer[5]).toInt() + String(inBuffer[6]).toInt())) * 0.046875;
        PrintOnLCD(String(resFloat) + " p. Idle FT");     // Вывод адаптации холостого хода (проценты) с подписью на экран
        break;

                                                        // На ресте здесь LTFT (M)
      case 10:                                          // Одиннадцатый экран - Long Term Fuel Trim (Low Power)
        resFloat = (round(256 * String(inBuffer[5]).toInt() + String(inBuffer[6]).toInt())) * 0.00003052;
        PrintOnLCD(String(resFloat) + " LTFT (L)");       // Вывод долгосрочной коррекции при низкой нагрузке с подписью на экран
        break;

      case 11:                                          // Двенадцатый экран - Long Term Fuel Trim (High Powert)
        resFloat = (round(256 * String(inBuffer[5]).toInt() + String(inBuffer[6]).toInt())) * 0.00003052;
        PrintOnLCD(String(resFloat) + " LTFT (H)");       // Вывод долгосрочной коррекции при высокой нагрузке с подписью на экран
        break;

      case 12:                                          // Тринадцатый экран - ShortTermCorrection
        resFloat = ((round(256 * String(inBuffer[5]).toInt() + String(inBuffer[6]).toInt())) * 2) / 65535.0;
        PrintOnLCD(String(resFloat) + " STFT");           // Вывод краткосрочной коррекции с подписью на экран
        break;

      case 13:                                          // Четырнадцатый экран - MisfireCounter
        resInt = 256 * String(inBuffer[5]).toInt() + String(inBuffer[6]).toInt();
        PrintOnLCD(String(resInt) + " Misfires");         // Вывод количество пропусков зажигания с подписью на экран
        break;

      default:                                          // Экран ошибки
        PrintOnLCD("ERR PG " + String(currentPage));
        break;
      }
    }
  }
}

void PrintOnLCD(String input) {                                                           // Передча строки на вход функции, она переделает её под нужные пакеты и подаст на вход DIM для вывода на экран
  if (input.length() > 32)
    input = "Message length  too long";                                                   // На всякий случай проверка на превышение длины сообщения

  while (input.length() < 32)                                                             // Заполняем сообщение до максимальных 32 символов пробелами
    input += " ";

  unsigned char msg[5][8] = {                                                             // Заполнение массива на вывод полного сообщения на 32 символа (2х16) в требуемом DIM'ом виде 
    { 0xA7, 0x00, input[0], input[1], input[2], input[3], input[4], input[5] },
    { 0x21, input[6], input[7], input[8], input[9], input[10], input[11], input[12] },
    { 0x22, input[13], input[14], input[15], input[16], input[17], input[18], input[19] },
    { 0x23, input[20], input[21], input[22], input[23], input[24], input[25], input[26] },
    { 0x65, input[27], input[28], input[29], input[30], input[31], 0x00, 0x00 }
  };

  CAN_LS.sendMsgBuf(LCD_ID, 1, 8, ClearLCD);                                              // Очистка экрана перед выводом нового сообщения во избежание появления символов от предыдущего сообщения
  delay(30);

  for (int i = 0; i < 5; i++) {                                                           // Вывод сообщения на экран (из массива) с учетом задержки между пакетами в 30мс
    CAN_LS.sendMsgBuf(PHM_ID, 1, 8, msg[i]);
    delay(30);
  }
}