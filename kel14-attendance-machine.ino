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

// Inisiasi pin LED, status LED, dan status debugging
const int BLUE_LED = 2;
bool IS_LED_ON = false;
bool IS_DEBUGGING_CARD = false;

// Variabel penampung apakah ada perintah masuk
// melalui komunikasi serial arduino
String command;

// Inisiasi struct yang akan menampung data pengguna
// yang sudah tap kartu mereka
struct UserAttended
{
  int id;
  String name;
  unsigned long time; // Ini dari millis
};

// Nah, disini ditampungnya peserta yang udah tap kartu
struct UserAttended allAttendedUsers[] = {{1, "Ezra", 100000}, {2, "Farhan", 200000}, {3, "Hanif", 300000}};

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
  Serial.println("Output will be in CSV format, please copy the value after this line.");

  Serial.println("==============================================");
  Serial.println("id,name,time");

  if (sizeof(allAttendedUsers) > 0)
  {
    int arrayLength = sizeof(allAttendedUsers) / sizeof(allAttendedUsers[0]);

    for (int i = 0; i < arrayLength; i++)
    {
      Serial.print(allAttendedUsers[i].id);
      Serial.print(",");
      Serial.print(allAttendedUsers[i].name);
      Serial.print(",");
      Serial.println(allAttendedUsers[i].time);
    }
  }

  Serial.println("==============================================");

  Serial.println("");
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
  if (sizeof(allAttendedUsers) == 0)
  {
    Serial.println("Data empty already!");
    Serial.println("");

    return;
  }

  // memset(allAttendedUsers, {}, sizeof(allAttendedUsers));
  // Serial.println("Data cleared successfully!");

  Serial.println("");
}

void setup()
{
  Serial.begin(115200);
  pinMode(BLUE_LED, OUTPUT);

  delay(2000);

  Serial.println("Machine is now ready, type \"help\" to show all available command");
  Serial.println("");
}

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
