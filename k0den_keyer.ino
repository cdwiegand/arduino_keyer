/*
K0DEN Arduino CW Keyer

Copyright (c) 2012 by Christopher Donald Wiegand - K0DEN

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and 
associated documentation files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial 
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#define pinLed 13
#define pinSpkr 6
#define pinKey 12
#define baudSerial 115200
int wpmSpeed = 15;
int baseSpeed = 120; // the time between dits/dahs/elements in ms
boolean cmdMode = 0;
String cmdBuffer = "";
#define toneValue 850

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pins as outputs
  pinMode(pinLed, OUTPUT); 
  pinMode(pinKey, OUTPUT); 
  
  processCommand("\\WPM 15");
  
  // setup serial port
  Serial.begin(baudSerial);
  Serial.println("Initializing K0DEN CW keyer...");
  Serial.print("LED: Pin "); Serial.println(pinLed);
  Serial.print("Key: Pin "); Serial.println(pinKey);
  Serial.print("Speaker: Pin "); Serial.println(pinSpkr);
  Serial.print("WPM: "); Serial.println(wpmSpeed);
  Serial.println("All commands begin with backslash (\\). For help type \\HELP");
}

// the loop routine runs over and over again forever:
void loop() {
  if (Serial.available() > 0)
    handle(Serial.read());
}

void handle(char inbound) {
  if (inbound == '\n' || inbound == '\r') {
    Serial.println();
    if (cmdMode == 1) {
      cmdBuffer.trim();
      if (cmdBuffer.length() > 0)
        processCommand(cmdBuffer);
      cmdBuffer = "";
      cmdMode = 0;
    }
  } else if (inbound == '\\' && cmdMode == 0) {
    cmdMode = 1;
    Serial.println();
    Serial.print("CMD: ");
    bufferCommand(inbound);
  } else {
    // normal character
    if (cmdMode == 0)
      morse(inbound);
    else
      bufferCommand(inbound);
  }
}

void bufferCommand(char inbound) {
  cmdBuffer += inbound;
  Serial.print(inbound);
}

void processCommand(String cmd) {
  int idx = cmd.indexOf(' ');
  String cmdName = cmd;
  String cmdParam = "";
  if (idx > -1) cmdName = cmd.substring(0,idx);
  if (idx > -1) cmdParam = cmd.substring(idx+1);
  cmdName.toUpperCase();
  
  if (cmdName == "\\WPM") {
    unsigned int length = cmdParam.length() + 1; // NUL-terminated
    char chars[length];
    cmdParam.toCharArray(chars,length);
    setWpmSpeed(atoi(chars));
    Serial.print("Set WPM to ");
    Serial.println(wpmSpeed);
  } else if (cmdName == "\\HELP") {
    Serial.println("Help:");
    Serial.println("Type \\WPM <value> to set the words per minute.");
    Serial.println("     \\EXIT to exit command mode.");
    Serial.println("     \\HELP for help (this screen).");
  } else {
    Serial.print("Unknown command: '");
    Serial.print(cmdName);
    Serial.println("'");
  }
}

void setWpmSpeed(int theSpeed) {
  wpmSpeed = theSpeed;
  baseSpeed = 60/(float(wpmSpeed)*50)*1000;
}

char uppercase(char value) {
  if (value >= 'a' and value <= 'z')
    value = value - 32;
  return value; // FIXME do it right, please
}

String getMorseCode(char sendMe) {
  switch (sendMe) {
    case 'A': return ".-";
    case 'B': return "-...";
    case 'C': return "-.-.";
    case 'D': return "-..";
    case 'E': return ".";
    case 'F': return "..-.";
    case 'G': return "--.";
    case 'H': return "....";
    case 'I': return "..";
    case 'J': return ".---";
    case 'K': return "-.-";
    case 'L': return ".-..";
    case 'M': return "--";
    case 'N': return "-.";
    case 'O': return "---";
    case 'P': return ".--.";
    case 'Q': return "--.-";
    case 'R': return ".-.";
    case 'S': return "...";
    case 'T': return "-";
    case 'U': return "..-";
    case 'V': return "...-";
    case 'W': return ".--";
    case 'X': return "-..-";
    case 'Y': return "-.--";
    case 'Z': return "--..";
    case '0': return "-----";
    case '1': return ".----";
    case '2': return "..---";
    case '3': return "...--";
    case '4': return "....-";
    case '5': return ".....";
    case '6': return "-....";
    case '7': return "--...";
    case '8': return "---..";
    case '9': return "----.";
    case '.': return ".-.-.-";
    case ',': return "--..--";
    case '?': return "..--..";
    case '\'': return ".----.";
    case '!': return "-.-.--";
    case '/': return "-..-.";
    case '(': return "-.--.";
    case ')': return "-.--.-";
    case '&': return ".-...";
    case ':': return "---...";
    case ';': return "-.-.-.";
    case '=': return "-...-";
    case '+': return ".-.-.";
    case '-': return "-....-";
    case '_': return "..--.-";
    case '"': return ".-..-.";
    case '$': return "...-..-";
    case '@': return ".--.-.";
    case ' ': return " ";
    case '\n': return " ";
    case '\t': return " ";
    case '\r': return " ";
  }
  return ""; // FIXME?
}

void morse(char sendMe) {
  Serial.print(sendMe); // before we uppercase
  sendMe = uppercase(sendMe);
  String morse = getMorseCode(sendMe);
  for (int idx = 0; idx < morse.length(); idx++) {
    switch (morse[idx]) {
      case '.': active(1); break;
      case '-': active(3); break;
    }
    delay(baseSpeed);
  }
  delay(baseSpeed); // we already delayed after the last letter
}
 
void active(int activeMultiplier) {  
  if (pinLed > 0) digitalWrite(pinLed, HIGH);
  if (pinKey > 0) digitalWrite(pinKey, HIGH);
  if (pinSpkr > 0) tone(pinSpkr, toneValue, baseSpeed*activeMultiplier);
  
  delay(baseSpeed * activeMultiplier);
  
  if (pinLed > 0) digitalWrite(pinLed, LOW);
  if (pinKey > 0) digitalWrite(pinKey, LOW);
  if (pinSpkr > 0) noTone(pinSpkr);
}
