#include <mysql.h>
#include <Keypad.h>
#include<LiquidCrystal.h>

LiquidCrystal lcd(9, 8, 7, 6, 5, 4); //definire lcd si, respectiv, legare la pinii de comanda

char *host, *user, *pass, *db;
char password[4]; // char array-ul in care vor fi preluate caracterele de la tastura
int i = 0;
char customKey = 0;
const byte ROWS = 4; // numarul de randuri ale tastaturii
const byte COLS = 4; // numarul de coloane ale tastaturii
char hexaKeys[ROWS][COLS] = {
  {'1', '4', '7', '*'},
  {'2', '5', '8', '0'},
  {'3', '6', '9', '#'},
  {'A', 'B', 'C', 'D'}
}; // definire matricei de butoane
byte rowPins[ROWS] = {A0, A1, A2, A3}; // conectarea randurilor tastaturii la liniile de pinout
byte colPins[COLS] = {A4, A5, 3, 2}; // conectarea coloanelor tastaturii la liniile de pinout

Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); // definire tastatura matriceala

int led;
int buzzer = 10;
int m11;
int m12;

void setup()
{
  pinMode(11, OUTPUT); // se seteaza pinul de comanda al releului ca iesire

  Serial.begin(9600); // se incepe comunicatia seriala cu 9600 bauds

  host = "localhost";
  user = "root";
  pass = "asus271995";
  db = "digital_door_lock"; // se preiau credentialele pentru comunicarea cu serverul php pe care se regaseste baza de date MySQL

  lcd.begin(16, 2); // se initializeaza lcd-ul

  pinMode(led, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(m11, OUTPUT);
  pinMode(m12, OUTPUT); // se seteaza pinii led, buzzer, m11, m12 ca output

  digitalWrite(11, HIGH);
  delay(600);

  Serial.println(" Digital ");
  lcd.print(" Digital ");
  lcd.setCursor(0, 1);
  Serial.println(" Door Lock ");
  lcd.print(" Door Lock ");
  delay(1000);
  lcd.clear();

  lcd.print("Enter Passkey");
  lcd.setCursor(0, 1);
  lcd.print("or D to delete");
  delay (1500);
  Serial.println("Password:");
  lcd.clear();
  lcd.print("Enter passkey:");
  lcd.setCursor(0, 1);
}

void loop() {
  digitalWrite(11, HIGH);
  customKey = customKeypad.getKey(); // se defineste variabila in care se vor prelua caracterele de la tastura

  // Se introduc caracterele de la tastatura diferite de D in char array-ul password
  if (customKey && customKey != 'D')
  {
    password[i++] = customKey;
    lcd.print("*");
    Serial.print(customKey);
    beep();
  }

  // Daca s-au introdus 4 caractere se verifica daca parola se regaseste in baza de date
  if (i == 4)
  {
    String buffer = String(password[0]) + String(password[1]) + String(password[2]) + String(password[3]); // buffer-ul contine cele 4 caractere
    check_password(buffer); // se verifica validitatea parolei
  }

  // Daca s-a introdus D se realizeaza stergerea parolei si se va reveni la introducerea unei noi parole
  if (customKey == 'D')
  {
    lcd.print('D');
    delay(300);
    lcd.clear();
    lcd.print("Password cleared!");

    for (int k = 0; k < i; k++) {
      password[k] = "";
    }
    delay(1000);
    lcd.clear();
    lcd.print("Your Passkey:");
    Serial.println("Your Passkey:");
    lcd.setCursor(0, 1);
    i = 0;
  }
}

// Se implementeaza functia pentru beep-ul tastelor si de verificare pentru buzzer
void beep()
{
  digitalWrite(buzzer, HIGH);
  delay(20);
  digitalWrite(buzzer, LOW);
}

// Se implementeaza functia care verifica existenta parolei in baza de date
void check_password (String intreg) {
  // Se realizeaza conexiunea la baza de date utilizand credentialele predefinite prin conexiune seriala
  if (mysql_connect(host, user, pass, db)) {
    Serial.print("Connected to ");
    Serial.println(host);

    // Se defineste query-ul SQL ce va fi transmis pentru executie bazei de date
    char generic_query[] = {"SELECT user FROM passwords WHERE parola = '"};
    String str(generic_query);
    String query = str + intreg + "'";
    String result = mysql_result_query(query, "user");

    // Se analizeaza rezultatul query-ului primit de la baza de date prin conexiunea seriala
    if (result.length() > 0) {
      beep();
      lcd.clear();
      lcd.print("Welcome, user:");
      Serial.println("Welcome, user:");
      lcd.setCursor(0, 1);
      lcd.print(result);
      Serial.println(result);
      delay(1000);
      digitalWrite(led, HIGH);
      digitalWrite(11, LOW); // comuta releul, iar solenoidul isi retrage boltul pentru deschiderea usii
      delay(4500);

      // Se revine in starea de asteptare a unei parole
      lcd.clear();
      lcd.print("Enter Passkey:");
      Serial.println("Enter Passkey:");
      lcd.setCursor(0, 1);
      i = 0;
      digitalWrite(led, LOW);
    }
    else {
      Serial.println("Query execution failed or no record(s) found.");
      digitalWrite(11, HIGH);
      digitalWrite(buzzer, HIGH);
      lcd.clear();
      lcd.print("Access Denied...");
      Serial.println("Access Denied...");
      delay(1500);
      lcd.clear();
      lcd.print("Wrong Password...");
      Serial.println("Wrong Password...");
      lcd.setCursor(0, 1);
      lcd.print("TRY Again");
      Serial.println("TRY Again");
      delay(2000);

      //
      lcd.clear();
      lcd.print("Enter Passkey:");
      Serial.println("Enter Passkey:");
      lcd.setCursor(0, 1);
      i = 0;
      digitalWrite(buzzer, LOW);
    }
  }
  else {
    Serial.println("Connection failed.");
  }
}


