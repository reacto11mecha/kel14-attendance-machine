/**
 * Projek ini dibuat untuk memenuhi penilaian untuk kegiatan
 * seleksi anggota baru Robotic Club Universitas Negeri Jakarta.
 *
 * Program ini dimiliki oleh kelompok 14 yang beranggotakan,
 * 1. Ezra Khairan Permana (APD 2024)
 * 2. Farhan Aditya (PTIK 2024)
 * 3. Hanif Naufal (ILKOM 2024d)
 *
 * Program ini berjalan menggunakan modul PN532 untuk membaca
 * kartu elektronik pada gelombang frekuensi 13,56Mhz dan ESP32
 * WROOM sebagai board microcontrollernya.
 */

// Memanggil library yang dibutuhkan untuk keperluan
// membaca UID dari kartu elektronik yang didekatkan
// pada sensor PN532
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

// Pake vector biar gampang ngolah data yang udah hadir
#include <vector>

PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);

// Inisiasi pin LED, status LED, status terhubung ke modul pembaca, dan mode debugging
const int BLUE_LED = 2;
bool IS_LED_ON = false;
bool IS_CARD_CONNECTED = false;
bool IS_DEBUGGING_CARD = false;

// Khusus untuk handle loop pembaca kartu
TaskHandle_t ReaderLoop;

// Variabel penampung apakah ada perintah masuk
// melalui komunikasi serial arduino
String command;

struct RegisteredUser
{
  uint8_t cuid[4];
  String name;
};

const struct RegisteredUser allRegisteredUser[] = {
    {{0x93, 0x95, 0xFB, 0xEE}, "bored lookin card"},
    {{0x83, 0x2C, 0x5D, 0xA6}, "keyless lookin keychain"},
    {{0xD0, 0xC0, 0x27, 0x93}, "erabu mai"}};

// Inisiasi struct yang akan menampung data pengguna
// yang sudah tap kartu mereka
struct UserAttended
{
  int id;
  String name;
  unsigned long time; // Ini dari millis
};

// Nah, disini ditampungnya peserta yang udah tap kartu
std::vector<UserAttended> allAttendedUsers;

// Fungsi yang menjadi pintu utama apakah modul pembaca kartu
// sudah terhubung atau belum. Fungsi ini akan terus menerus
// dipanggil koneksi belum terjalin.
bool connectToCard()
{
  nfc.begin();

  // Cek apakah kartu sudah terhubung atau belum
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata)
  {
    Serial.println("PN532 card module not found!");
    return false;
  }

  Serial.println("Please tap your card to the sensor.");
  Serial.println();

  if (IS_DEBUGGING_CARD)
  {
    Serial.print("Found chip PN5");
    Serial.println((versiondata >> 24) & 0xFF, HEX);
    Serial.print("Firmware version: ");
    Serial.print((versiondata >> 16) & 0xFF, DEC);
    Serial.print('.');
    Serial.println((versiondata >> 8) & 0xFF, DEC);
    Serial.println();
  }

  // Mentapkan jumlah membaca kartu sebanyak 255 kali
  // karena secara default PN532 membaca terus menerus
  // tanpa henti.
  nfc.setPassiveActivationRetries(0xFF);

  // Kasih tau kalo PN532 bakalan baca RFID
  nfc.SAMConfig();

  return true;
}

// Fungsi yang bertujuan untuk menyalakan/mematikan LED
// bawaan yang berada di pin digital 2
void showHelpOutput()
{
  Serial.println("=======================================================================");

  Serial.println("Available commands:");
  Serial.println("* help");
  Serial.println("    > Show currently available commands to this board.");

  Serial.println("");

  Serial.println("* toggle-led");
  Serial.println("    > To toggle LED on pin 2 on or off.");

  Serial.println("");

  Serial.println("* recap");
  Serial.println("    > Recap all successfully scanned card");

  Serial.println("");

  Serial.println("* clear-list");
  Serial.println("    > Clear all the list of successfully scanned card");

  Serial.println("");

  Serial.println("* toggle-card-debugging");
  Serial.println("    > To toggle debugging mode, verbosely output the scanning process.");

  Serial.println("=======================================================================");

  Serial.println("");
}

// Fungsi yang bertujuan untuk menyalakan/mematikan LED
// bawaan yang berada di pin digital 2
void doToggleLed()
{
  IS_LED_ON = !IS_LED_ON;
  String state = IS_LED_ON ? "ON" : "OFF";

  digitalWrite(BLUE_LED, IS_LED_ON ? HIGH : LOW);

  Serial.println("LED is now " + state);
  Serial.println("");
}

// Fungsi yang bertujuan untuk menghasilkan rekap presensi kehadiran
// pada serial monitor yang berbentuk csv
void doAttendanceRecap()
{
  if (allAttendedUsers.size() > 0)
  {
    Serial.println("Output will be in CSV format, please copy the value after this line.");

    Serial.println("==============================================");
    Serial.println("id,name,time");

    for (int i = 0; i < allAttendedUsers.size(); i++)
    {
      Serial.print(allAttendedUsers[i].id);
      Serial.print(",");
      Serial.print(allAttendedUsers[i].name);
      Serial.print(",");
      Serial.println(allAttendedUsers[i].time);
    }

    Serial.println("==============================================");

    Serial.println();

    return;
  }

  Serial.println("Recap data is empty! Please tap some card to recap the data");
  Serial.println();
}

// Fungsi yang bertujuan untuk menghasilkan rekap presensi kehadiran
// pada serial monitor yang berbentuk csv
void doToggleCardDebugging()
{
  IS_DEBUGGING_CARD = !IS_DEBUGGING_CARD;
  String state = IS_DEBUGGING_CARD ? "ON" : "OFF";

  Serial.println("Card scan debugging is now " + state);
  Serial.println("");
}

// Fungsi yang bertujuan untuk mengosongkan data presensi yang
// sebelumnya suadah terdaftar pada siklus sistem saat ini
void doClearAttendanceList()
{
  if (allAttendedUsers.size() == 0)
  {
    Serial.println("Data empty already!");
    Serial.println("");

    return;
  }

  allAttendedUsers.clear();

  Serial.println("Data cleared successfully!");
  Serial.println("");
}

void setup()
{
  Serial.begin(115200);
  pinMode(BLUE_LED, OUTPUT);

  Serial.println("Type \"help\" to show all available command");
  Serial.println("");

  // Menambahkan fungsi loop2 supaya bisa berjalan
  // parallel dengan loop yang menerima perintah
  // masuk melalui komunikasi serial.
  xTaskCreatePinnedToCore(
      loop2,       // Function to implement the task
      "loop2",     // Name of the task
      6000,        // Stack size in words
      NULL,        // Task input parameter
      1,           // Priority of the task
      &ReaderLoop, // Task handle.
      0            // Core where the task should run
  );
}

// Loop yang ini handle command masuk, biar ga bentrok sama readernya
void loop()
{
  if (Serial.available())
  {
    command = Serial.readStringUntil('\n');
    command.trim();

    Serial.println("Command: " + command);

    if (command.equals("help"))
      showHelpOutput();
    else if (command.equals("toggle-led"))
      doToggleLed();
    else if (command.equals("recap"))
      doAttendanceRecap();
    else if (command.equals("toggle-card-debugging"))
      doToggleCardDebugging();
    else if (command.equals("clear-list"))
      doClearAttendanceList();
    else
    {
      Serial.println("Unknown commands: " + command);
      Serial.println("");
    }

    return;
  }
}

// Kalo yang ini khusus buat readernya aja
void loop2(void *pvParameters)
{
  // pura-pura jadi loop
  while (1)
  {
    // Variabel yang menandakan kartu berhasil di baca
    boolean successfullyReaded;

    // Penyimpnan sementara untuk UID yang sudah terbaca
    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
    // Ukuran UID (4 or 7 bytes depending on card type)
    uint8_t uidLength;

    while (!IS_CARD_CONNECTED)
    {
      IS_CARD_CONNECTED = connectToCard();
    }

    successfullyReaded = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);

    if (successfullyReaded)
    {
      if (IS_DEBUGGING_CARD)
      {
        Serial.println("Card Detected");
        Serial.print("Size of UID: ");
        Serial.print(uidLength, DEC);
        Serial.println(" bytes");
        Serial.print("UID: ");
        for (uint8_t i = 0; i < uidLength; i++)
        {
          Serial.print(" 0x");
          Serial.print(uid[i], HEX);
        }
        Serial.println("");
        Serial.println("");
      }

      bool userFound = false;
      int arrayLengthOfRegisteredUsers = sizeof(allRegisteredUser) / sizeof(allRegisteredUser[0]);

      for (int i = 0; i < arrayLengthOfRegisteredUsers; i++)
      {
        RegisteredUser currentUser = allRegisteredUser[i];
        short currentUserUidLength = sizeof(currentUser.cuid);

        if (currentUserUidLength == uidLength)
        {
          for (int j = 0; j < currentUserUidLength; j++)
          {
            bool isSameByte = currentUser.cuid[j] == uid[j];

            // Ini kalo masi di peserta awal ga ada yang match masih bisa lanjut.
            // Kalau sampe akhir false terus, nanti di bagian !userFound
            // nilainya jadi true, menandakan kartunya ga terdaftar.
            if (j != arrayLengthOfRegisteredUsers - 1 && !isSameByte)
              break;

            // Nah ini kalo udah ketemu nanti di handle di baris ini.
            else if (j == arrayLengthOfRegisteredUsers - 1 && isSameByte)
            {
              UserAttended user;

              int attendedSize = allAttendedUsers.size();
              int nextUserId = attendedSize > 0 ? allAttendedUsers[attendedSize - 1].id + 1 : 1;

              user.id = nextUserId;
              user.name = currentUser.name;
              user.time = millis();

              Serial.println("You attendance successfully recorded. Your name is \"" + user.name + "\" and your attendance time is " + user.time + ".");
              Serial.println();

              allAttendedUsers.push_back(user);

              userFound = true;

              break;
            }
          }
        }
      }

      if (!userFound)
      {
        Serial.println("Card not found, try again.");
        Serial.println();
      }

      delay(userFound ? 3500 : 3000);
      IS_CARD_CONNECTED = connectToCard();
    }
  }
}