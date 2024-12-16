#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <LiquidCrystal.h>
#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

#define sensorPin A0
#define G_led 8
#define R_led 9

#define motor_open 2
#define motor_close 3

// Keypad
const byte ROW_NUM = 4;
const byte COLUMN_NUM = 3;
char keys[ROW_NUM][COLUMN_NUM] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }
};
byte pin_rows[ROW_NUM] = { A3, 13, 12, 10 };
byte pin_column[COLUMN_NUM] = { 11, A1, A2 };
Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);
// Keypad

int set_timer = 6;

int sensorState = 0;
int seconds = set_timer;
int flag = 0;
long delay_Start;

String lcdTextLine1 = "";
String lcdTextLine2 = "";
String lcdTextLine3 = "";
String lcdTextLine4 = "";

int passwordMode = 0;
String password = "251104";
String inputPassword = "_ _ _ _ _ _";
int currentIndexInput = 0;

void lcdRemoveLine(int line) {
  lcd.setCursor(0, line);
  lcd.print("                    ");
}

void lcdPrintLine(int line, String text) {
  lcd.setCursor(0, line);
  lcd.print(text);
}

void lcdPrint(int line, String content) {
  if ((line == 0 && content != lcdTextLine1) || (line == 1 && content != lcdTextLine2) || (line == 2 && content != lcdTextLine3) || (line == 3 && content != lcdTextLine4)) {
    lcdRemoveLine(line);
    lcd.setCursor(0, line);
    lcd.print(content);
    if (line == 0) lcdTextLine1 = content;
    if (line == 1) lcdTextLine2 = content;
    if (line == 2) lcdTextLine3 = content;
    if (line == 3) lcdTextLine4 = content;
  }
}

void lcdPrintCenter(int line, String text) {
  int len = text.length();
  int col = (20 - len) / 2;
  lcd.setCursor(col, line);
  lcd.print(text);
}

void showMembers() {
  if (passwordMode == 1) return;
  String members[5] = { "Nguyen Van Khoi", "Hoang Van Vu", "Nguyen Huy Dung", "Vu Tri Minh", "Nguyen Van Tran" };
  int memberIndex = 0;
  lcd.clear();
  lcdPrintCenter(0, "THANH VIEN");
  while (memberIndex < 5 && memberIndex >= 0) {
    if (memberIndex == 3) {
      delay(1000);
      lcdRemoveLine(1);
      lcdRemoveLine(2);
      lcdRemoveLine(3);
    }
    if (memberIndex < 3) {
      lcd.setCursor(0, memberIndex + 1);
    } else {
      lcd.setCursor(0, memberIndex + 1 - 3);
    }
    lcd.print(String(memberIndex + 1) + ". " + members[memberIndex]);
    memberIndex++;
    // if (memberIndex >= 5) {
    //   delay(2000);
    //   lcdRemoveLine(1);
    //   lcdRemoveLine(2);
    //   lcdRemoveLine(3);
    //   memberIndex = 0;
    // }
  }
}

void menu() {
  lcd.clear();
  lcdPrintCenter(0, "Muc luc");
  lcd.setCursor(0, 1);
  lcd.print("1. Thanh vien");
  lcd.setCursor(0, 2);
  lcd.print("2. Nhap mat khau");
  lcd.setCursor(0, 3);
  lcd.print("3. Dong cua");
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  lcd.init();
  // lcd.backlight();
  lcd.begin(20, 4);
  lcdPrintCenter(0, "HE THONG CUA TU DONG");
  delay(1000);
  menu();
  pinMode(sensorPin, INPUT);

  pinMode(R_led, OUTPUT);
  pinMode(G_led, OUTPUT);

  pinMode(motor_open, OUTPUT);
  pinMode(motor_close, OUTPUT);

  digitalWrite(motor_open, LOW);
  digitalWrite(motor_close, HIGH);
  delay(2000);
  digitalWrite(motor_open, LOW);
  digitalWrite(motor_close, LOW);
}

// Password
void resetInputPassword() {
  currentIndexInput = 0;
  passwordMode = 1;
  inputPassword = "_ _ _ _ _ _";
}

void goToScreenEnterPassword() {
  lcd.clear();
  resetInputPassword();
  lcdPrintCenter(0, "NHAP MAT KHAU");
  lcdPrintCenter(1, "_ _ _ _ _ _");
  lcd.setCursor(0, 3);
  lcd.print("Huy (*)");
  lcd.setCursor(14, 3);
  lcd.print("OK (#)");
  lcd.setCursor(4, 1);
}

void enterInputPassword(char key) {
  if (currentIndexInput > inputPassword.length() - 1) return;
  if (inputPassword[currentIndexInput] == '_') {
    inputPassword[currentIndexInput] = key;
    lcdPrintCenter(1, inputPassword);
    lcd.setCursor(currentIndexInput + 6, 1);
  }
  currentIndexInput += 2;
}

String removeChar(String s, char key) {
  String ans = "";
  for (int i = 0; i < s.length(); i++) {
    if (s[i] != key) ans += s[i];
  }
  return ans;
}

void confirmPassword() {
  lcd.clear();
  if ((removeChar(inputPassword, ' ')).equals(password)) {
    lcdPrintCenter(1, "Mo cua thanh cong");
    resetInputPassword();
    passwordMode = 0;
    delay(500);
    goToScreenStatus();
  } else {
    lcdPrintCenter(1, "Mo cua that bai");
    delay(500);
    goToScreenEnterPassword();
  }
}
// Password

// Door Status

void goToScreenStatus() {
  lcd.clear();
  lcdPrintCenter(0, "Trang thai");
  lcdPrintLine(1, String("Cua: ") + (flag == 0 ? "dong" : "mo"));
  lcdPrintLine(2, String("Chuyen dong: ") + (sensorState == 0 ? "khong" : "co"));
}

// Door Status


void loop() {
  sensorState = digitalRead(sensorPin);

  if (passwordMode == 0) {
    lcd.noBlink();
  } else {
    if (currentIndexInput > inputPassword.length() - 1) lcd.noBlink();
    else lcd.blink();
  }

  char key = keypad.getKey();
  if (key) {
    if (key == '*' || key == '#') passwordMode = 0;
    if (passwordMode == 0) {
      switch (key) {
        case '0':
          menu();
          break;
        case '1':
          showMembers();
          break;
        case '2':
          goToScreenEnterPassword();
          break;
        // case '3':
        //   sensorState = 0;
        //   break;
        case '*':
          menu();
          break;
        case '#':
          confirmPassword();
          break;
        default:
          lcd.clear();
          lcdPrintCenter(0, "Khong hop le!");
          delay(500);
          menu();
      }
    } else {
      passwordMode = 1;
      enterInputPassword(key);
    }
  }
  if (sensorState == 1) {
    Serial.println("Co chuyen dong");
    lcdPrint(0, "Co chuyen dong");
    digitalWrite(R_led, HIGH);
    digitalWrite(G_led, HIGH);

    if (flag == 0) {
      lcdRemoveLine(2);
      lcdPrint(1, "Cua dang mo...");
      digitalWrite(motor_open, HIGH);
      digitalWrite(motor_close, LOW);
      delay(2000);
      digitalWrite(motor_open, LOW);
      digitalWrite(motor_close, LOW);
      lcdPrint(1, "Cua da mo");
    }

    seconds = set_timer;
    flag = 1;
    delay_Start = millis();
  } else {
    Serial.println("Khong co chuyen dong");
    lcdPrint(0, "Khong co chuyen dong");
    digitalWrite(G_led, LOW);
  }

  if ((flag == 1) && (millis() - delay_Start) > 1000) {
    seconds = seconds - 1;
    lcdPrint(2, "Thoi gian cho: " + String(seconds) + "s");
    delay_Start = millis();
  }

  if (seconds <= 0 && flag == 1) {
    lcdPrint(1, "Cua dang dong...");
    digitalWrite(R_led, LOW);
    digitalWrite(motor_open, LOW);
    digitalWrite(motor_close, HIGH);
    delay(2000);
    digitalWrite(motor_open, LOW);
    digitalWrite(motor_close, LOW);
    lcdPrint(1, "Cua da dong");
    lcdRemoveLine(2);
    flag = 0;
  }
}
