/**
 * Projek ini dibuat untuk memenuhi penilaian untuk kegiatan
 * seleksi anggota baru Robotic Club Universitas Negeri Jakarta.
 *
 * Program ini dimiliki oleh kelompok 14 yang beranggotakan,
 * 1. Ezra Khairan Permana (APD 2024)
 * 2. Farhan Aditya (PTIK 2024)
 * 3. Hanif Naufal (ILKOM 2024d)
 */
const int BLUE_LED = 2;
bool IS_LED_ON = false;

String command;

void doToggleLed()
{
  IS_LED_ON = !IS_LED_ON;
  String state = IS_LED_ON ? "ON" : "OFF";

  digitalWrite(BLUE_LED, IS_LED_ON ? HIGH : LOW);

  Serial.println("LED is now " + state);
}

void doAttendanceRecap()
{
  Serial.println("pura pura inin outputnya csv");
}

void setup()
{
  Serial.begin(115200);
  pinMode(BLUE_LED, OUTPUT);

  delay(2000);

  Serial.println("Machine is now ready, type \"help\" to show all available command");
}

void loop()
{
  if (Serial.available())
  {
    command = Serial.readStringUntil('\n');
    command.trim();

    Serial.println("Command: " + command);

    if (command.equals("toggle-led"))
      doToggleLed();
    else if (command.equals("rekap"))
      doAttendanceRecap();
    else
      Serial.println("Unknown commands: " + command);

    return;
  }
}
