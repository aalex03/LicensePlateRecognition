int counter = 0;

void setup() {
    Serial.begin(9600);
}

void loop() {
    if (Serial.available()) {
        // Read data from Raspberry Pi
        String receivedData = Serial.readStringUntil('\n');

        // Increment the counter
        counter++;

        // Create the response message
        String response = "Counter: " + String(counter) + ", Received: " + receivedData;

        // Send response back to Raspberry Pi
        Serial.println(response);
    }
}
