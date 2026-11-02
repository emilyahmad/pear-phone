void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);   // onboard user LED (GPIO21 on XIAO ESP32S3)
  delay(2000);
  Serial.println("ESP32S3 Sense is alive!");
  Serial.printf("Chip: %s, rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
  Serial.printf("CPU: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("PSRAM: %d bytes\n", ESP.getPsramSize());
}

void loop() {
  digitalWrite(LED_BUILTIN, LOW);   // LED is active-low on this board
  Serial.println("LED on");
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("LED off");
  delay(500);
}