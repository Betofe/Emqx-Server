// This example sketch connects to the public shiftr.io instance and sends a message on every keystroke.
// After starting the sketch you can find the client here: https://www.shiftr.io/try.
//
// Note: If you're running the sketch via the Android Mode you need to set the INTERNET permission
// in Android > Sketch Permissions.
//
// by Joël Gähwiler
// https://github.com/256dpi/processing-mqtt
import processing.serial.*;
import mqtt.*;

MQTTClient client;
Serial myPort;
String receivedData = "";
String message="";
void setup() {
  size(200, 200);
  String portName = Serial.list()[0];
  myPort = new Serial(this, portName, 57600);
  
  client = new MQTTClient(this);
  client.connect("mqtt://broker.emqx.io", "processing");
}

void draw() {
background(255);
 while (myPort.available() > 0) {
    char receivedChar = myPort.readChar();
    receivedData += receivedChar;
    receivedData = receivedData.trim();
    if (receivedChar == '\n') {
      // Entire string received, process it
      processReceivedData();
      receivedData = "";  // Reset the string for the next data
    }
  }
}
void clientConnected() {
  client.subscribe("Processing/esp32");
  println("client connected");
}

void messageReceived(String topic, byte[] payload) {
  message = ""; // Reset the message variable before processing a new message
 // println("new message: " + new String(payload));
  for (int i=0; i< payload.length; i++){
    message += (char) payload[i]; //convert *byte to string
  }
  message = message.trim();
  println(message);
  myPort.write(message);
  myPort.write("");
  
  
}
void processReceivedData() {
 
  println("Received data: " + receivedData);
  client.publish("Processing2/esp32", receivedData);
 
 if (receivedData.equals("done")){
   myPort.write("ACK");
 }
 
 receivedData = ""; // Reset receivedData after publishing
}
void connectionLost() {
  println("connection lost");
}
