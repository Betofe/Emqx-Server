// ***********************************************************************
// Include Statements
// ***********************************************************************
#include <Arduino.h>
#include <Adafruit_Fingerprint.h>


// ***********************************************************************
// Hardware Configuration
// ***********************************************************************
HardwareSerial serialPort(2); // use UART1
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&serialPort);

// ***********************************************************************
// Global Variables
// ***********************************************************************
uint8_t id = 1;
uint8_t tempF[512]; // Array to store fingerprint templateF
String val = ""; // Data received from the serial port
bool enrollmentStarted = false;
unsigned long lastMillis = 0; // For non-blocking delay
bool ackReceived = false; // Flag to indicate ACK received
unsigned long ackTime = 0; // Time when ACK was received

// ***********************************************************************
// Function Prototypes
// ***********************************************************************
uint8_t getFingerprintEnroll();
bool getFingerprintTemplate();
void sendMessage();
String concatHex(byte b);
void printHex(uint8_t num);

// **********************************************************************
// Setup Function
// **********************************************************************
void setup()
{
    Serial.begin(57600);
    finger.begin(57600); 
    while (!Serial)
    ; 
  if (finger.verifyPassword())
  {
    // PURPLE LED fully on
       finger.LEDcontrol(FINGERPRINT_LED_ON, 0, FINGERPRINT_LED_PURPLE);
  }
  else
  {
    while (1)
    {
      delay(1);
    }
  } 
}



// **********************************************************************************
// Loop Function
// **********************************************************************************
void loop()
{   
    val = "";
    while (Serial.available())
    {
        val = Serial.readStringUntil('\n'); // Read the incoming data until newline;
        Serial.flush(); // Clear the serial buffer to ensure no old commands interfere
    }

    if (val == "RQT")
    {
      if (!enrollmentStarted) {
        enrollmentStarted = true;
        while (!getFingerprintEnroll());{ 
        }
       //  enrollmentStarted = false; // Ensure the flag is reset after enrollment
    }
    } else if (val == "ACK") {
      if (!ackReceived) { // Only trigger on the first ACK received
        ackReceived = true;
        ackTime = millis(); // Record the time ACK was received
      }
    }
        if (ackReceived) {
        unsigned long currentTime = millis();

        if (currentTime-ackTime <=400){
        finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_BLUE);
        }
        else if (currentTime - ackTime >1000){
        finger.LEDcontrol(FINGERPRINT_LED_ON, 0, FINGERPRINT_LED_PURPLE);
        ackReceived = false; // Reset the flag to prevent re-entering this state
        val = ""; // Clear val to get ready for the next command
        enrollmentStarted = false; // Reset enrollment flag 
        }
        }
    if (enrollmentStarted && getFingerprintTemplate())
    {
        // Enrollment completed, send fingerprint template
        sendMessage();
        // PURPLE LED fully on
       finger.LEDcontrol(FINGERPRINT_LED_ON, 0, FINGERPRINT_LED_PURPLE);
       
       if (millis() - lastMillis > 250) { 
           Serial.println("done"); 
          }
       enrollmentStarted = false; // Reset enrollment flag
       val = ""; // Clear val to get ready for the next command  
    }   
}
    


// *********************************************************************************
// Function to enroll a fingerprint
// *********************************************************************************
uint8_t getFingerprintEnroll()
{
    int p = -1;
    if (millis() - lastMillis > 250) { // Update LED every 250 ms
            lastMillis = millis();
            finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_PURPLE);
        }
    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            //Serial.println("Image taken");
            break;
        case FINGERPRINT_NOFINGER:
            //Serial.println(".");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            //Serial.println("Communication error");
            break;
        case FINGERPRINT_IMAGEFAIL:
            //Serial.println("Imaging error");
            break;
        default:
            //Serial.println("Unknown error");
            break;
        }
    }

    // OK success!

    p = finger.image2Tz(1);
    switch (p)
    {
    case FINGERPRINT_OK:
        //Serial.println("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        //Serial.println("Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        //Serial.println("Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        //Serial.println("Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        //Serial.println("Could not find fingerprint features");
        return p;
    default:
        //Serial.println("Unknown error");
        return p;
    }

    if (millis() - lastMillis > 250) { // Update LED every 250 ms
            lastMillis = millis();
            // BLUE LED fully on
            finger.LEDcontrol(FINGERPRINT_LED_ON, 0, FINGERPRINT_LED_BLUE);
        }
        
    p = 0;
    while (p != FINGERPRINT_NOFINGER)
    {
        p = finger.getImage();
    }
   
    p = -1;
    if (millis() - lastMillis > 250) { // Update LED every 250 ms
            lastMillis = millis();
            finger.LEDcontrol(FINGERPRINT_LED_FLASHING, 25, FINGERPRINT_LED_PURPLE);
        }
    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            //Serial.println("Image taken");
            break;
        case FINGERPRINT_NOFINGER:
            //Serial.print(".");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            //Serial.println("Communication error");
            break;
        case FINGERPRINT_IMAGEFAIL:
            //Serial.println("Imaging error");
            break;
        default:
            //Serial.println("Unknown error");
            break;
        }
    }

    // OK success!

    p = finger.image2Tz(2);
    switch (p)
    {
    case FINGERPRINT_OK:
        //Serial.println("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        //Serial.println("Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        //Serial.println("Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        //Serial.println("Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        //Serial.println("Could not find fingerprint features");
        return p;
    default:
        //Serial.println("Unknown error");
        return p;
    }

    // OK converted!
    //Serial.print("Creating model for #");
    //Serial.println(id);

    p = finger.createModel();
    if (p == FINGERPRINT_OK)
    {
        //Serial.println("Prints matched!");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        //Serial.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_ENROLLMISMATCH)
    {
        //Serial.println("Fingerprints did not match");
        return p;
    }
    else
    {
        //Serial.println("Unknown error");
        return p;
    }

    //Serial.print("ID ");
    //Serial.println(id);
    p = finger.storeModel(1);
    if (p == FINGERPRINT_OK)
    {
     if (millis() - lastMillis > 250) { // Update LED every 250 ms
            lastMillis = millis();
            finger.LEDcontrol(FINGERPRINT_LED_ON, 0, FINGERPRINT_LED_BLUE);
        }
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        //Serial.println("Communication error");
        return p;
    }
    else if (p == FINGERPRINT_BADLOCATION)
    {
        //Serial.println("Could not store in that location");
        return p;
    }
    else if (p == FINGERPRINT_FLASHERR)
    {
        //Serial.println("Error writing to flash");
        return p;
    }
    else
    {
        //Serial.println("Unknown error");
        return p;
    }
    return true;
  
}

// **********************************************************************************
// Function to get the fingerprint template after enrollment
// **********************************************************************************
bool getFingerprintTemplate()
{
    int p = finger.loadModel(1); // Load the enrolled model
    if (p == FINGERPRINT_OK)
    {

        p = finger.getModel();
        if (p == FINGERPRINT_OK)
        {
          //  Serial.println("Template transferred:");

            // one data packet is 267 bytes. in one data packet, 11 bytes are 'useless' :D
            uint8_t bytesReceived[534]; // 2 data packets
            memset(bytesReceived, 0xff, 534);

            uint32_t starttime = millis();
            int i = 0;
            while (i < 534 && (millis() - starttime) < 20000)
            {
                if (serialPort.available())
                {
                    bytesReceived[i++] = serialPort.read();
                }
            }

            // Filtering only the data packets
            int uindx = 9, index = 0;
            memcpy(tempF + index, bytesReceived + uindx, 256); // first 256 bytes
            uindx += 256;                                       // skip data
            uindx += 2;                                         // skip checksum
            uindx += 9;                                         // skip next header
            index += 256;                                       // advance pointer
            memcpy(tempF + index, bytesReceived + uindx, 256);  // second 256 bytes

            // Dump the entire fingerprint template. This prints out 16 lines of 16 bytes
            for (int i = 0; i < 512; ++i)
            {
            }
            return true;
        }
        else
        {
           // Serial.print("Unknown error ");
           // Serial.println(p);
            return false;
        }
    }
    else
    {
        //Serial.print("Failed to load model for ID #");
        return false;
    }
    return true;
}
// *********************************************************************************
// Function to convert a byte to a two-character hex string
// *********************************************************************************
String concatHex(byte b)
{
    char hexBuffer[3];
    snprintf(hexBuffer, sizeof(hexBuffer), "%02X", b);
    return String(hexBuffer);
}

bool deleteFingerprint(uint8_t id) {
    int p = finger.deleteModel(id);
    if (p == FINGERPRINT_OK) {
        //Serial.println("Successfully deleted fingerprint.");
        return true;
    } else {
       // Serial.println("Error deleting fingerprint.");
        return false;
    }
}

void sendMessage() {

  // Convert the fingerprint template to a hex string
    int div = 8; // Specify the division factor
    for (int j = 0; j < div; j++) {
        String hexTemplate = "hexTemplate" + String(j) + ":";
        for (int i = 0; i < 512 / div; ++i) {
            hexTemplate += concatHex(tempF[i + j * (512 / div)]);
        }
       // hexTemplate += "\n"; // Add a newline to signify the end of this segment
       
     
        // write to port fingerprint data
        Serial.println(hexTemplate);
        delay(100);
        
        deleteFingerprint(1);
    }

}
// **********************************************************************************
// Function to print a byte in hexadecimal format
// **********************************************************************************
void printHex(uint8_t num)
{
    if (num < 0x10)
    {
        Serial.print("0");
    }
    //Serial.print(num, HEX);
    //Serial.print(", ");
}
