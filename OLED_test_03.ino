#include <EEPROM.h>
#include <Wire.h>
#include <OzOLED.h>

//#include <LiquidCrystal.h>


struct display_time {
  int minutes;
  int seconds;
};

static const unsigned char UP_DOWN_ARROWS_CHAR[] PROGMEM ={
//byte UP_DOWN_ARROWS_CHAR[16] = {
  0b00000,
  0b01000,
  0b00100,
  0b00010,
  0b11111,
  0b00010,
  0b00100,
  0b01000,
  0b00000,
  0b00010,
  0b00100,
  0b01000,
  0b11111,
  0b01000,
  0b00100,
  0b00010
};


//define select statuses
#define SS_NONE     0
#define SS_SELECTED 1
//define reset statuses
#define RS_YES      1
#define RS_NO       2

//lcd defines
#define LCD_WIDTH     16
#define LCD_HEIGHT     2
#define LCD_BRIGHTNESS_PIN 10

//define default factory settings
#define DEFAULT_TRESHOLD                     10 //int
#define DEFAULT_BUTTON_DOWN_TIME_FAST        1000 //milliseconds
#define DEFAULT_BUTTON_DOWN_TIME_SEQUENTIAL  90 //seconds
#define DEFAULT_LONG_EXP_TIME                10 //minutes
#define DEFAULT_SILENT_BETWEEN_SOOTING       1000 //milliseconds

////define pins
#define SHOOTER_PIN 2
#define TRIGGER_PIN A1

//
#define BASE_UPDATE_INTERVAL 3000
#define VOLTAGE_UPDATE_INTERVAL 5 * 60 * 1000 // 5 minutes

//define buttons
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
#define btnLONGRIGHT  10
#define btnLONGUP     11
#define btnLONGDOWN   12
#define btnLONGLEFT   13
#define btnLONGSELECT 14
#define btnLONGNONE   15
#define btnIDLE       25


//define menus
// new
#define mSetMode              1
#define mSetTreshold          2
#define mSetBDTimeFast        3
#define mSetBDTimeSequential  4
#define mSetSilentTime        5
#define mSetExpD              6
#define mReset                7 //set settings to default

//define modes
#define modeFAST_ON_FLASH             4
#define modeFAST_ADJUSTABLE_ON_FLASH  3
#define modeSEQUENTIAL                2
#define modeLONG_EXPOSURE             1


//
#define MAX_LINE  50
char Line_1[MAX_LINE];
char Line_2[MAX_LINE];

#define DEBUG
#undef DEBUG
#ifdef DEBUG
#define pout(STR) Serial.println((STR))
#else
#define pout(STR)
#endif

// Global variables
int mainMenuItem;
int treshold;
int down_time_fast;
int down_time_long_exp;
int down_time_sequential;
int silent_between;

int shootingMode;
int select_status;
int reset_status;
int long_eposure_first_shot = 0;


////////////////////////////////////////////////////////////////////
// SETTINGS
// default settings
// read & write settings data to EEPROM
////////////////////////////////////////////////////////////////////
void write_settings()
{
  int addr = 0;
  EEPROM.put(addr, treshold);
  addr += sizeof(treshold);
  EEPROM.put(addr, down_time_fast);
  addr += sizeof(down_time_fast);
  EEPROM.put(addr, down_time_long_exp);
  addr += sizeof(down_time_long_exp);
  EEPROM.put(addr, down_time_sequential);
  addr += sizeof(down_time_sequential);
  EEPROM.put(addr, shootingMode);
  addr += sizeof(shootingMode);
  //long exp
  EEPROM.put(addr, silent_between);
  addr += sizeof(silent_between);
}

void read_settings()
{
  int addr = 0;
  EEPROM.get(addr, treshold);
  addr += sizeof(treshold);
  EEPROM.get(addr, down_time_fast);
  addr += sizeof(down_time_fast);
  EEPROM.get(addr, down_time_long_exp);
  addr += sizeof(down_time_long_exp);
  EEPROM.get(addr, down_time_sequential);
  addr += sizeof(down_time_sequential);
  EEPROM.get(addr, shootingMode);
  addr += sizeof(shootingMode);
  EEPROM.get(addr, silent_between);
  addr += sizeof(silent_between);
}

void set_default_factory_vals()
{
  mainMenuItem = mSetMode;
  treshold = DEFAULT_TRESHOLD;
  down_time_fast = DEFAULT_BUTTON_DOWN_TIME_FAST;
  down_time_sequential = DEFAULT_BUTTON_DOWN_TIME_SEQUENTIAL;
  down_time_long_exp = DEFAULT_LONG_EXP_TIME;
  silent_between = DEFAULT_SILENT_BETWEEN_SOOTING;
  shootingMode = modeFAST_ON_FLASH;
}

////////////////////////////////////////////////////////////////////
// LCD functions
////////////////////////////////////////////////////////////////////

void line_out(byte column, byte line, char *str)
{
  //lcd.setCursor(column, line);
  //lcd.print(str);
  //reduce string length to 16 symbols
  if (strlen (str)>16)
    str[16] = '\0';
  OzOled.printString(str, column, line);
}

void line_out_special(byte column, byte line, byte symbol )
{
//  lcd.setCursor(column, line);
//  lcd.write(byte(symbol));
}

void set_LCD_setiing()
{
  OzOled.init();
  OzOled.setNormalDisplay();      //Set display to normal mode (i.e non-inverse mode)
  OzOled.setPageMode();           //Set addressing mode to Page Mode 
}

void LCD_print_screen()
{
  int max_line = MAX_LINE;
  memset(Line_1, ' ', max_line);
  memset(Line_2, ' ', max_line);
  switch (mainMenuItem)
  {
    case mSetMode:
      {
        pout("mode");
        snprintf(Line_1, max_line, "Select mode:");
        switch (shootingMode)
        {
          case modeFAST_ON_FLASH:
            {
              snprintf(Line_2, max_line, "On flash adjustable");
              break;
            }
          case modeFAST_ADJUSTABLE_ON_FLASH:
            {
              snprintf(Line_2, max_line, "On flash fast+");
              break;
            }
          case modeSEQUENTIAL:
            {
              snprintf(Line_2, max_line, "Sequential shooting");
              break;
            }
          case modeLONG_EXPOSURE:
            {
              snprintf(Line_2, max_line, "Long exposure");
              break;
            }
        }
        break;
      }
    case mSetTreshold:
      {
        pout("tr");
        snprintf(Line_1, max_line, "Set treshold");
        snprintf(Line_2, max_line, "%d", treshold);
        break;
      }
    case mSetBDTimeFast:
      {
        pout("BA");
        snprintf(Line_1, max_line, "Set shooting time");
        snprintf(Line_2, max_line, "%d (millis)", down_time_fast);
        break;
      }
    case mSetBDTimeSequential:
      {
        pout("Sh ");
        snprintf(Line_1, max_line, "Set sequential time");
        snprintf(Line_2, max_line, "%d (seconds)", down_time_sequential);
        break;
      }
    case mSetSilentTime:
      {
        pout("BC");
        snprintf(Line_1, max_line, "Set time between");
        snprintf(Line_2, max_line, "%d (millis)", silent_between);
        break;
      }
    case mSetExpD:
      {
        pout("ECXP");
        snprintf(Line_1, max_line, "Set long exposure time");
        snprintf(Line_2, max_line, "%d (minutes)", down_time_long_exp);
        break;
      }
    case mReset:
      {
        pout("BC");
        snprintf(Line_1, max_line, "Reset to default");
        snprintf(Line_2, max_line, "");
        if ( SS_SELECTED == select_status && RS_YES == reset_status)
          snprintf(Line_2, max_line, "   NO    >YES<  ");
        else if ( SS_SELECTED == select_status && RS_NO == reset_status)
          snprintf(Line_2, max_line, "  >NO<    YES  ");
        else
          snprintf(Line_2, max_line, "               ");
        break;
      }
  }
  //lcd.clear();
  OzOled.clearDisplay();
  
  line_out(0, 2, Line_1);
  line_out(0, 3, Line_2);

  if (SS_SELECTED == select_status && mReset != mainMenuItem)
  {
    //lcd.setCursor(0, 0);
    //lcd.write(byte(0));
    OzOled.printString("*", 0, 0);
    //OzOled.drawBitmap(UP_DOWN_ARROWS_CHAR, 0, 0, 1, 1);
  }
  else
  {
    //ch_up = 9650
    OzOled.drawBitmap(UP_DOWN_ARROWS_CHAR, 0, 0, 1, 2);
    
//    line_out(0, 0, "<->");
      
  }
  pout(Line_1);
  pout(Line_2);
  get_power_voltage();
}

void LCD_print_working_screen()
{
  int max_line = MAX_LINE;
  memset(Line_1, ' ', max_line);
  memset(Line_2, ' ', max_line);

  switch (shootingMode)
  {
    case modeFAST_ON_FLASH:
      {
        snprintf(Line_1, max_line, "Fast Adjustable");
        snprintf(Line_2, max_line, "Sens %d Int %d", 50 - treshold, down_time_fast);
        break;
      }
    case modeFAST_ADJUSTABLE_ON_FLASH:
      {
        snprintf(Line_1, max_line, "Fast+");
        snprintf(Line_2, max_line, "Sens %d Int %d", 50 - treshold, down_time_fast);
        break;
      }
    case modeSEQUENTIAL:
      {
        snprintf(Line_1, max_line, "Sequential photos");
        snprintf(Line_2, max_line, "Int %d seconds", down_time_sequential);
        break;
      }
    case modeLONG_EXPOSURE:
      {
        snprintf(Line_1, max_line, "Long exposure");
        snprintf(Line_2, max_line, "Int %d minutes", down_time_long_exp);
        break;
      }
  }
  //lcd.clear();
  OzOled.clearDisplay();
  line_out(0, 2, Line_1);
  line_out(0, 3, Line_2);
  get_power_voltage();
}

void print_timer_screen(struct display_time *dt)
{
  int max_line = MAX_LINE;
  char time_str[10];
  memset(Line_1, ' ', max_line);
  memset(Line_2, ' ', max_line);
  display_time_to_str (dt, time_str);
  

  switch (shootingMode)
  {
    case modeSEQUENTIAL:
      {
        snprintf(Line_1, max_line, "Sequential photos");
        snprintf(Line_2, max_line, "Int %s minutes", time_str);
        break;
      }
    case modeLONG_EXPOSURE:
      {
        snprintf(Line_1, max_line, "Long exposure");
        snprintf(Line_2, max_line, "Int %s minutes", time_str);
        break;
      }
  }
  //lcd.clear();
  OzOled.clearDisplay();
  pout(Line_1);
  pout(Line_2);
  line_out(0, 2, Line_1);
  line_out(0, 3, Line_2);
  get_power_voltage();
}

////////////////////////////////////////////////////////////////////
// MENU
////////////////////////////////////////////////////////////////////

int inc_menu (int menu_item, int max_items, int delta, int select_status_param)
{
  if (SS_SELECTED == select_status_param)
  {
    pout("before");
    pout(menu_item);
    pout("max_items");
    pout(max_items);
    pout("delta");
    pout(delta);
    menu_item = menu_item + delta;
    if ( abs(delta) > menu_item)
    {
      menu_item = max_items;
    }
    else
    {
      menu_item = menu_item % (max_items + abs(delta));
      menu_item = menu_item == 0 ? menu_item + abs(delta) : menu_item;
    }
    pout("after");
    pout(menu_item);
  }
  return menu_item;
}

void menu()
{
  int key;
  int exit_loop = 0;
  int key_pressed = btnNONE;
  //  int last_key_pressed = btnNONE;
  
  reset_status = RS_NO;
  select_status = SS_NONE;
  //mainMenuItem = mSetMode;
  
  while ( 0 == exit_loop )
  {
    
    key = wait_for_keypress();
    
    switch (key)
    {
      case btnLEFT:
        {
          pout("left");
          mainMenuItem = inc_menu(mainMenuItem, 7, -1, (select_status + 1) % 2);
          /* moved to from right/left section
          if (mReset == mainMenuItem && SS_SELECTED == select_status)
          {
            reset_status = inc_menu(reset_status, 2, 1, select_status);
            break;
          }
          */
          break;
        }
      case btnRIGHT:
        {
          pout("right");
          mainMenuItem = inc_menu(mainMenuItem, 7, 1, (select_status + 1) % 2);
          /* moved to from right/left section
          if (mReset == mainMenuItem && SS_SELECTED == select_status)
          {
            reset_status = inc_menu(reset_status, 2, -1, select_status);
            break;
          }
          */
          break;
        }
      case btnUP:
        {
          pout("up");
          switch (mainMenuItem)
          {
            case mSetMode:
              {
                shootingMode = inc_menu(shootingMode, 4, -1, select_status);
                break;
              }
            case mSetTreshold:
              {
                treshold = inc_menu(treshold, 50, 5, select_status);
                break;
              }
            case mSetBDTimeFast:
              {
                down_time_fast = inc_menu(down_time_fast, 1500, 100, select_status);
                break;
              }
            case mSetBDTimeSequential:
              {
                down_time_sequential = inc_menu(down_time_sequential, 360, 10, select_status);
                break;
              }
            case mSetSilentTime:
              {
                silent_between = inc_menu(silent_between, 3000, 100, select_status);
                break;
              }
            case mSetExpD:
              {
                down_time_long_exp = inc_menu(down_time_long_exp, 45, 1, select_status);
                break;
              }
          }
          // moved from right/left section
          if (mReset == mainMenuItem && SS_SELECTED == select_status)
          {
            reset_status = inc_menu(reset_status, 2, 1, select_status);
            break;
          }

          break;
        }
      case btnDOWN:
        {
          pout("down");
          switch (mainMenuItem)
          {
            case mSetMode:
              {
                shootingMode = inc_menu(shootingMode, 4, 1, select_status);
                break;
              }
            case mSetTreshold:
              {
                treshold = inc_menu(treshold, 50, -5, select_status);
                break;
              }
            case mSetBDTimeFast:
              {
                down_time_fast = inc_menu(down_time_fast, 1500, -100, select_status);
                break;
              }
            case mSetBDTimeSequential:
              {
                down_time_sequential = inc_menu(down_time_sequential, 360, -10, select_status);
                break;
              }
            case mSetSilentTime:
              {
                silent_between = inc_menu(silent_between, 3000, -100, select_status);
                break;
              }
            case mSetExpD:
              {
                down_time_long_exp = inc_menu(down_time_long_exp, 45, -1, select_status);
                break;
              }
          }
          // moved from right/left section
          if (mReset == mainMenuItem && SS_SELECTED == select_status)
          {
            reset_status = inc_menu(reset_status, 2, -1, select_status);
            break;
          }
          break;
        }
      case btnSELECT:
        {
          select_status = ++select_status % 2;
          pout("btn select");
          pout("selected status");
          pout(select_status);
          pout(SS_SELECTED);

          pout("reset status");
          pout(reset_status);
          pout(RS_YES);
          if (SS_NONE == select_status && RS_YES == reset_status)
          {
            pout ("set to feault");
            set_default_factory_vals();
          }

          pout((SS_NONE == select_status ? "select none" : "select"));

          break;
        }
      case btnLONGSELECT:
        {
          pout("long select");
          exit_loop = 1;
          write_settings();
          break;
        }
    }
    LCD_print_screen();
    get_power_voltage();
  }
  pout("exit menu");
}

////////////////////////////////////////////////////////////////////
// KEYPAD FUNCTIONS
////////////////////////////////////////////////////////////////////

int read_keypad_button(int key_val)
{
  if (key_val > 1000) return btnNONE;
  if (key_val < 50)   return btnRIGHT;
  if (key_val < 195)  return btnUP;
  if (key_val < 380)  return btnDOWN;
  if (key_val < 555)  return btnLEFT;
  if (key_val < 790)  return btnSELECT;
  return btnNONE;  // when all others fail, return this...
}

int reda_digiatl_button()
{
  if (SS_SELECTED == select_status)
  {
    if (digitalRead(6) == HIGH)
      return 185;
    if (digitalRead(8) == HIGH)
      return 370;
  }
  else
  {
    if (digitalRead(6) == HIGH)
      return 40;
    if (digitalRead(8) == HIGH)
      return 545;
  }
  if (digitalRead(7) == HIGH)
      return 780;
  return 1000;
}

int wait_for_keypress()
{
  int exit_loop = 0;
  int key;
  int start_time = 0;
  int end_time = 0;
  while (exit_loop == 0)
  {
    key = read_keypad_button(reda_digiatl_button());
    delay (10);
    start_time = millis();
    if (key == read_keypad_button(reda_digiatl_button()) && key != btnNONE)
    {
      delay(10);
      while (read_keypad_button(reda_digiatl_button()) != btnNONE)
      {
        end_time = millis();
        exit_loop = 1;
      }
    }
    else
      continue;
  }
  if (((end_time - start_time) > 2000) && btnNONE != key )
  {
    key = key + 10;
  }
  return key;
}

////////////////////////////////////////////////////////////////////
// FUNCTIONAL
////////////////////////////////////////////////////////////////////

void long_exposure_on_flash()
{
  pout("long_exposure_on_flash wait");
  int lightning_base = analogRead(TRIGGER_PIN);
  unsigned long int refresh_base_timer = millis();
  while (1)
  {
    int newLightningVal = analogRead(TRIGGER_PIN);
    if (newLightningVal - lightning_base >= treshold)
    {
      pout(newLightningVal);
      pout(lightning_base);

      pout("take poict flash long");
      take_picture(SHOOTER_PIN, down_time_fast, silent_between);
      lightning_base = analogRead(TRIGGER_PIN);
    }
    if (millis() - refresh_base_timer > BASE_UPDATE_INTERVAL)
    {
      pout("5 sec");
      pout(lightning_base);
      lightning_base = newLightningVal;
      refresh_base_timer = millis();
      get_power_voltage();
    }
  }
}

void fast_shoots_on_flash()
{
  pout("fast_shoots_on_flash");
  int lightning_base = analogRead(TRIGGER_PIN);

  while (1)
  {
    int newLightningVal = analogRead(TRIGGER_PIN);
    if (newLightningVal - lightning_base > treshold)
    {
      //pout("take poict flash short");
      take_picture(SHOOTER_PIN, down_time_fast, silent_between);
    }
  }
}

void sequential_long_exposure()
{
  while (1)
  {
    pout("take pict seq long");
    pout (SHOOTER_PIN);
    take_picture_display_time(SHOOTER_PIN, down_time_sequential * 1000L, silent_between,1000);
  }
}

void long_exposure()
{
  pout("take pict long exposure");
  take_picture_display_time(SHOOTER_PIN, down_time_long_exp * 60000L,silent_between*60,1000);
}

void take_picture(byte PIN, unsigned long delay_between, unsigned long delay_after)
{
  digitalWrite(PIN, HIGH);
  delay(delay_between);
  digitalWrite(PIN, LOW);
  delay(delay_after);
}

//add inc_val to current time 
void inctime (struct display_time *dt, int inc_val)
{
  dt->seconds = dt->seconds + inc_val;
  if (dt->seconds >= 60){
    dt->minutes = dt->minutes + dt->seconds/60;
    dt->seconds = dt->seconds % 60;
  }
  else if (dt->seconds < 0){
    dt->minutes = dt->minutes - (abs(dt->seconds/60) + 1);
    dt->seconds = 60-abs(dt->seconds) % 60;
  }
}

void decode_time(struct display_time *dt, unsigned long int time_ms)
{
  dt->minutes = time_ms/(1000L*60L);
  dt->seconds = (time_ms/1000L) % 60L;
}

void display_time_to_str(struct display_time *dt, char *str)
{
  snprintf(str, 6, "%d:%.02d", dt->minutes, dt->seconds);
}

void take_picture_display_time(byte PIN, unsigned long delay_between, unsigned long delay_after, unsigned long int show_time_interval)
{
  unsigned long int start_time = millis();
  struct display_time *dt = (struct display_time *)malloc(sizeof(struct display_time));
  digitalWrite(PIN, HIGH);
  while (delay_between > millis () - start_time)
  {
    decode_time(dt, delay_between - (millis () - start_time));
    print_timer_screen(dt);
    delay(show_time_interval);
  }
  digitalWrite(PIN, LOW);
  delay(delay_after);
}

////////////////////////////////////////////////////////////////////
// ARDUINO
////////////////////////////////////////////////////////////////////

#define FASTADC 1

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


float get_power_voltage(){
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  ADCSRA |= _BV(ADSC); // начало преобразований
  while (bit_is_set(ADCSRA, ADSC)); // измерение
  uint8_t low = ADCL; // сначала нужно прочесть ADCL - это запирает ADCH
  uint8_t high = ADCH; // разлочить оба
  float result = (high<<8) | low;
  result = (1.10 * 1023.0) / result; // Результат Vcc в милливольтах
  
  String resultstr = String(result,2); 
  if (result <= 3.65)
    OzOled.printString("Low level battery", 0, 6);
  else
    OzOled.printString("Voltage", 0, 6);
  OzOled.printString(resultstr.c_str(),0,7);
  
  return result;
}


void setup() {
  
#if FASTADC
  // set prescale to 16
  sbi(ADCSRA,ADPS2) ;
  cbi(ADCSRA,ADPS1) ;
  cbi(ADCSRA,ADPS0) ;
#endif
#ifdef DEBUG
  Serial.begin(9600);
#endif
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  pinMode(SHOOTER_PIN, OUTPUT);
  set_LCD_setiing();
  OzOled.printNumber(get_power_voltage(), 5, 0);
  OzOled.printString("Voltage", 5, 0);
  read_settings();
  mainMenuItem = mSetMode;
  LCD_print_screen();
  menu();
  LCD_print_working_screen();
}

void loop() {
  
  switch (shootingMode)
  {
    case modeFAST_ON_FLASH:
      {
        long_exposure_on_flash();
        break;
      }
    case modeFAST_ADJUSTABLE_ON_FLASH:
      {
        fast_shoots_on_flash();
        break;
      }
    case modeSEQUENTIAL:
      {
        sequential_long_exposure();
        break;
      }
    case modeLONG_EXPOSURE:
      {
        if (0 == long_eposure_first_shot)
        {
          long_exposure();
          long_eposure_first_shot = 1;
        }
        break;
      }
  }
  delay(1000);
}

