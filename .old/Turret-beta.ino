#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include <Servo.h>


// AMG8833
#define BUTTON_PIN 2            // Button pin
Adafruit_AMG88xx amg;           // Sesnor object

// Servo
#define X_SERVO_PIN 10
#define Y_SERVO_PIN 9

Servo xServo;
Servo yServo;

int xTarget;
int yTarget;



void setup() {
    // * Initialize button pin
    pinMode(BUTTON_PIN, INPUT);

    // * Serial
    Serial.begin(115200);

    // * AMG8833 Sensor
    if (!amg.begin()) {
        Serial.println("Feil: AMG8833 not found. Check your wiring or I2C ADDR!");
        while (1);
    }

    Serial.println("AMG8833 found!");

    // * Servos
    xServo.attach(X_SERVO_PIN);     // Attach servo on digital pin 10
    yServo.attach(Y_SERVO_PIN);     // Attach servo on digital pin 9

    xTarget = 0;
    yTarget = 0;

}

void loop() {
    // * Read the temperature data when the button is pressed
    if (digitalRead(BUTTON_PIN) == HIGH) {
        float pixels[64];                       // 8x8 matrix to store the temperature data
        
        // Read the temperature data
        amg.readPixels(pixels);

        /*
        // Upscale the 8x8 matrix to a 64x64 matrix using bicubic interpolation
        float upscaledMatrix[4096];
        bicubicInterpolationUpscale(pixels, upscaledMatrix);
        */
        
        // Display temprature matrix
        displayTempratureMatrix(pixels);

        // // Delay sensor (refresh rate is 10Hz)
        // delay(100);     // 100ms delay     
    }

    // * Move servos
    int xPosition = xServo.read();
    int yPosition = yServo.read();

    xServo.write(xTarget%180);
    yServo.write(yTarget%180);

    displayServoStates(xPosition, yPosition);

    delay(100);

    xTarget += 2;
    yTarget += 2;

}

void displayTempratureMatrix(const float* matrix) {
    Serial.println("Temperature data:");
    
    for (int i = 0; i < 64; i++) {
        Serial.print(matrix[i]);
        Serial.print(", ");
        if ((i + 1) % 8 == 0) Serial.println();
    }
    
    Serial.println();
}

void displayServoStates(int xPosition, int yPosition) {
    Serial.print("xServo : ");
    Serial.print(xPosition);
    Serial.print(" (T: ");
    Serial.print(xTarget);
    Serial.println(")")

    Serial.print("yServo : ");
    Serial.print(yPosition);
    Serial.print(" (T: ");
    Serial.print(yTarget);
    Serial.println(")");
}
