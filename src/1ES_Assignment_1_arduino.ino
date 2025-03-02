// Define GPIO pins
const int switch1Pin = 16;  // Switch 1 (Enable/Disable WaveForm)
const int switch2Pin = 17;  // Switch 2 (Mode change)
const int ledPin = 27;      // Signal A (Main Waveform)
const int signalBPin = 26;  // Signal B (Sync Signal)

// Parameters for DAFF
unsigned long a = 400;  // Pulse width for first pulse (D = 4 * 100μs = 400μs)
unsigned long b = 100;  // Spaces between pulses (A = 1 * 100μs = 100μs)
int c = 10;             // Default number of pulses in a block (F = 6 + 4 = 10)
unsigned long d = 3000; // Space between blocks (F = 6 * 500μs = 3000μs)

// Modes
int mode = 1;             // Starting mode
bool enableWaveform = false; // Start with waveform generation disabled
bool switch2Pressed = false; 

void setup() {
  // Initialize GPIOs
  pinMode(switch1Pin, INPUT_PULLUP); // Switch 1
  pinMode(switch2Pin, INPUT_PULLUP); // Switch 2
  
  pinMode(ledPin, OUTPUT);           // Signal A
  pinMode(signalBPin, OUTPUT);       // Signal B

  // Start serial communication
  Serial.begin(115200);
  Serial.println("ESP32 Waveform Generator with Trigger Signal Started");
}

void loop() {
  // Read Switch 1 (Enable/Disable)
  enableWaveform = !digitalRead(switch1Pin); // Active LOW
  if (!enableWaveform) {
    digitalWrite(ledPin, LOW);      // Turn off Signal A
    digitalWrite(signalBPin, LOW); // Turn off Signal B
    return; 
  }

  // Read Switch 2 (Mode change)
  if (!digitalRead(switch2Pin)) { // Active LOW
    if (!switch2Pressed) { 
      mode = (mode % 2) + 1; // Cycle through modes 1 to 2 (default and alternative)
      Serial.print("Mode changed to: ");
      Serial.println(mode);
      switch2Pressed = true;
    }
  } else {
    switch2Pressed = false; 
  }

  // Generate waveform and trigger signal
  generateWaveform();
}

void generateWaveform() {
  Serial.println("Generating waveform...");
  unsigned long pulseWidth;
  int pulseCount = c;

  // Generate a trigger pulse (Signal B)
  digitalWrite(signalBPin, HIGH);
  delayMicroseconds(50); // 50 μs trigger pulse
  digitalWrite(signalBPin, LOW);

  // Alternative Mode
  // f maps to 6, (6 % 4) + 1 = 3 
  // Mode 3 is + 3 pulses to the end of a block
  if (mode == 2){
    // Add 3 pulses
      pulseCount = c + 3;
  }

  // Generate Signal A pulses in a block
  for (int i = 0; i < pulseCount; i++) {
    pulseWidth = a + i * 50; // Increment pulse width
    digitalWrite(ledPin, HIGH);
    delayMicroseconds(pulseWidth);
    digitalWrite(ledPin, LOW);
    delayMicroseconds(b);
    Serial.print("Pulse ");
    Serial.print(i + 1);
    Serial.println(" generated.");
  }

  // Wait before starting the next block
  delayMicroseconds(d);
  Serial.println("Waveform block complete.");
}
