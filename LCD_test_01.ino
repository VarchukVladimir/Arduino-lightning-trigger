#include <EEPROM.h>
#include <LiquidCrystal.h>



byte UP_DOWN_ARROWS_CHAR[8] = {
  0b00100,
  0b01110,
  0b10101,
  0b00100,
  0b00100,
  0b10101,
  0b01110,
  0b00100
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
#define DEFAULT_TRESHOLD      10
#define DEFAULT_BULB_A         1000
#define DEFAULT_BULB_C         90
#define DEFAULT_SHOOTING_TIME  700
#define DEFAULT_SHOOTING_MODE  3
//define pins
#define SHOOTER_PIN 12
#define TRIGGER_PIN A1

//
#define BASE_UPDATE_INTERVAL 5000

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

//define menus
#define mSetMode          1
#define mSetTreshold      2
#define mSetBulbA         3
#define mSetShootingTime  4
#define mSetBulbC         5
#define mReset            6 //set settings to default

//define modes
#define modeBULB_A        3
#define modeFAST_SHOOT_B  2
#define modeBULB_C        1
//
#define MAX_LINE  50
char Line_1[MAX_LINE];
char Line_2[MAX_LINE];

#define DEBUG
//#undef DEBUG
#ifdef DEBUG
#define pout(STR) Serial.println((STR))
#else
#define pout(STR)
#endif

// Global variables
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int mainMenuItem;
int treshold;
int bulbA;
int bulbC;
int shootingTimeB;
int shootingMode;
int select_status;
int reset_status;

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
  EEPROM.put(addr, bulbA);
  addr += sizeof(bulbA);
  EEPROM.put(addr, bulbC);
  addr += sizeof(bulbC);
  EEPROM.put(addr, shootingTimeB);
  addr += sizeof(shootingTimeB);
  EEPROM.put(addr, shootingMode);
  addr += sizeof(shootingMode);
}

void read_settings()
{
  int addr = 0;
  EEPROM.get(addr, treshold);
  addr += sizeof(treshold);
  EEPROM.get(addr, bulbA);
  addr += sizeof(bulbA);
  EEPROM.get(addr, bulbC);
  addr += sizeof(bulbC);
  EEPROM.get(addr, shootingTimeB);
  addr += sizeof(shootingTimeB);
  EEPROM.get(addr, shootingMode);
  addr += sizeof(shootingMode);
}

void set_default_factory_vals()
{
  mainMenuItem = mSetMode;
  treshold = DEFAULT_TRESHOLD;
  bulbA = DEFAULT_BULB_A;
  bulbC = DEFAULT_BULB_C;
  shootingTimeB = DEFAULT_SHOOTING_TIME;
  shootingMode = modeBULB_A;
}

////////////////////////////////////////////////////////////////////
// LCD functions
////////////////////////////////////////////////////////////////////

void line_out(byte column, byte line, char *str)
{
  lcd.setCursor(column, line);
  lcd.print(str);
}

void line_out_special(byte column, byte line, byte symbol )
{
  lcd.setCursor(column, line);
  lcd.write(byte(symbol));
}

void set_LCD_setiing()
{
  lcd.begin(LCD_WIDTH, LCD_HEIGHT);
  analogWrite(LCD_BRIGHTNESS_PIN, 40);
  lcd.createChar(0, UP_DOWN_ARROWS_CHAR);
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
        snprintf(Line_1, max_line, "    Mode");
        switch (shootingMode)
        {
          case modeBULB_A:
            {
              snprintf(Line_2, max_line, "A.BULB on flash");
              break;
            }
          case modeFAST_SHOOT_B:
            {
              snprintf(Line_2, max_line, "B.Fast on flash");
              break;
            }
          case modeBULB_C:
            {
              snprintf(Line_2, max_line, "C.Seq. BULB");
              break;
            }
        }
        break;
      }
    case mSetTreshold:
      {
        pout("tr");
        snprintf(Line_1, max_line, "    Treshold");
        snprintf(Line_2, max_line, "%d", treshold);
        break;
      }
    case mSetBulbA:
      {
        pout("BA");
        snprintf(Line_1, max_line, "    Interval A");
        snprintf(Line_2, max_line, "%d (millis)", bulbA);
        break;
      }
    case mSetShootingTime:
      {
        pout("Sh ");
        snprintf(Line_1, max_line, "    Shooting B");
        snprintf(Line_2, max_line, "%d (millis)", shootingTimeB);
        break;
      }
    case mSetBulbC:
      {
        pout("BC");
        snprintf(Line_1, max_line, "    Interval C");
        snprintf(Line_2, max_line, "%d (seconds)", bulbC);
        break;
      }
    case mReset:
      {
        pout("BC");
        snprintf(Line_1, max_line, "    Reset to default");
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
  lcd.clear();
  line_out(0, 0, Line_1);
  line_out(0, 1, Line_2);
  if (SS_SELECTED == select_status && mReset != mainMenuItem)
  {
    lcd.setCursor(0, 0);
    lcd.write(byte(0));
  }
  else
  {
    line_out(0, 0, "<->");
  }
  pout(Line_1);
  pout(Line_2);
}

void LCD_print_working_screen()
{
  int max_line = MAX_LINE;
  memset(Line_1, ' ', max_line);
  memset(Line_2, ' ', max_line);
  switch (shootingMode)
  {
    case modeBULB_A:
      {
        snprintf(Line_1, max_line, "A.BULB on flash");
        snprintf(Line_2, max_line, "Sens %d Int %d", 50 - treshold, bulbA);
        break;
      }
    case modeFAST_SHOOT_B:
      {
        snprintf(Line_1, max_line, "B.Fast on flash");
        snprintf(Line_2, max_line, "Sens %d Int %d", 50 - treshold, shootingTimeB);
        break;
      }
    case modeBULB_C:
      {
        snprintf(Line_1, max_line, "C.Seq. BULB");
        snprintf(Line_2, max_line, "Int %d seconds", bulbC);
        break;
      }
  }
  lcd.clear();
  line_out(0, 0, Line_1);
  line_out(0, 1, Line_2);
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
  mainMenuItem = mSetMode;
  reset_status = RS_NO;
  select_status = SS_NONE;
  while ( 0 == exit_loop )
  {
    key = wait_for_keypress();
    switch (key)
    {
      case btnLEFT:
        {
          pout("left");
          mainMenuItem = inc_menu(mainMenuItem, 6, -1, (select_status + 1) % 2);
          if (mReset == mainMenuItem && SS_SELECTED == select_status)
          {
            reset_status = inc_menu(reset_status, 2, 1, select_status);
            break;
          }
          break;
        }
      case btnRIGHT:
        {
          pout("right");
          mainMenuItem = inc_menu(mainMenuItem, 6, 1, (select_status + 1) % 2);
          if (mReset == mainMenuItem && SS_SELECTED == select_status)
          {
            reset_status = inc_menu(reset_status, 2, -1, select_status);
            break;
          }
          break;
        }
      case btnUP:
        {
          pout("up");
          switch (mainMenuItem)
          {
            case mSetMode:
              {
                shootingMode = inc_menu(shootingMode, 3, -1, select_status);
                break;
              }
            case mSetTreshold:
              {
                treshold = inc_menu(treshold, 50, 5, select_status);
                break;
              }
            case mSetBulbA:
              {
                bulbA = inc_menu(bulbA, 1500, 100, select_status);
                break;
              }
            case mSetShootingTime:
              {
                shootingTimeB = inc_menu(shootingTimeB, 1000, 100, select_status);
                break;
              }
            case mSetBulbC:
              {
                bulbC = inc_menu(bulbC, 360, 10, select_status);
                break;
              }
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
                shootingMode = inc_menu(shootingMode, 3, 1, select_status);
                break;
              }
            case mSetTreshold:
              {
                treshold = inc_menu(treshold, 50, -5, select_status);
                break;
              }
            case mSetBulbA:
              {
                bulbA = inc_menu(bulbA, 1500, -100, select_status);
                break;
              }
            case mSetShootingTime:
              {
                shootingTimeB = inc_menu(shootingTimeB, 1000, -100, select_status);
                break;
              }
            case mSetBulbC:
              {
                bulbC = inc_menu(bulbC, 360, -10, select_status);
                break;
              }
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

int wait_for_keypress()
{
  int exit_loop = 0;
  int key;
  int start_time = 0;
  int end_time = 0;
  while (exit_loop == 0)
  {
    key = read_keypad_button(analogRead(0));
    delay (10);
    start_time = millis();
    if (key == read_keypad_button(analogRead(0)) && key != btnNONE)
    {
      delay(10);
      while (read_keypad_button(analogRead(0)) != btnNONE)
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
    //
    int newLightningVal = analogRead(TRIGGER_PIN);
    if (newLightningVal - lightning_base >= treshold)
    {
      //pout(newLightningVal);
      //pout(lightning_base);

      //pout("take poict flash long");
      take_picture(SHOOTER_PIN, bulbA, 500);
      lightning_base = analogRead(TRIGGER_PIN);
    }
    if (millis() - refresh_base_timer > BASE_UPDATE_INTERVAL)
    {
      //pout("10 sec");
      //pout(lightning_base);
      lightning_base = newLightningVal;
      refresh_base_timer = millis();
    }
    delay(1000);
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
      take_picture(SHOOTER_PIN, shootingTimeB, 500);
    }
  }
}

void sequential_long_exposure()
{
  while (1)
  {
    pout("take pict seq long");
    pout (SHOOTER_PIN);
    take_picture(SHOOTER_PIN, bulbC * 1000L, 2000);
  }
}

void take_picture(byte PIN, unsigned long delay_between, unsigned long delay_after)
{
  digitalWrite(PIN, HIGH);
  delay(delay_between);
  digitalWrite(PIN, LOW);
  delay(delay_after);
}

////////////////////////////////////////////////////////////////////
// ARDUINO
////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  pinMode(SHOOTER_PIN, OUTPUT);
  set_LCD_setiing();
  read_settings();

  LCD_print_screen();
  menu();
  LCD_print_working_screen();
}

void loop() {
  switch (shootingMode)
  {
    case modeBULB_A:
      {
        long_exposure_on_flash();
        break;
      }
    case modeFAST_SHOOT_B:
      {
        fast_shoots_on_flash();
        break;
      }
    case modeBULB_C:
      {
        sequential_long_exposure();
        break;
      }
  }
}

