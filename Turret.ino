#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include <Servo.h>
#include <math.h>


// Debugging
#define DEBUG false
#define RESET_MODE false

#define SLEEP_MS 10000
#define IDLE_MS 5000


// Constants
#define TARGET_VALUE_THRESHOLD 2300

const unsigned char BASE_SIZE = 8;
const unsigned char SCALE_FACTOR = 2;

const unsigned char CAMERA_FOV = 60;        // Camera's field of view in degrees

const float SEVENTH = 4.286;                // 1/7th of the camera's FOV/2 - Used to calculate the angular distance along the yz-plane
const float EIGHTH = 3.75;                  // 1/8th of the camera's FOV/2 - Used to calculate the angular distance along the yz-plane

const float INVERSE_PI = 0.3185;            // 1/PI

const unsigned char SERVO_CENTER = 90;      // The center position of the servo

const float SPEED_X = 1;            // Speed of the rotation servo
const float SPEED_Y = 1;            // Speed of the pitch servo


// Stucture to store the coordinates within the matrix
struct coordinate {
    int x; 
    int y;
    int index;
    int value;


    // MIN: [raw : (0, 0)]
    //      x = -8
    //      y =  7 

    // MAX: [raw : (15, 15)]
    //      x =  7
    //      y = -8
        
    int angularDistanceToXYplane() {
        // Pitch servo angle
        int degrees;

        if (y == 0) {
            degrees = 0;
        } else if (y > 0) {         // If y is positive
            degrees = (int)(y * SEVENTH * SPEED_Y);
        } else {                    // If y is negative
            degrees = (int)(y * EIGHTH * SPEED_Y);
        }

        return SERVO_CENTER + degrees;
        // return (int)(atan((double)y / x) * 180 * INVERSE_PI);
    }

    int angularDistanceToYZplane() {
        // Rotation servo angle
        int degrees;

        if (x == 0) {               
            degrees = 0;
        } else if (x > 0) {         // If x is positive
            degrees = (int)(x * SEVENTH * SPEED_X);
        } else {                    // If x is negative
            degrees = (int)(x * EIGHTH * SPEED_X);
        }

        return SERVO_CENTER - degrees;
    }
};


// AMG8833
Adafruit_AMG88xx amg;           // Sensor object

// Servo
#define X_SERVO_PIN 10
#define Y_SERVO_PIN 9

Servo xServo;
Servo yServo;


void setup() {
    // * Serial
    Serial.begin(115200);
    Serial.println();
    Serial.println("Initializing...");

    // * AMG8833 Sensor
    if (!amg.begin()) {
        Serial.println("Feil: AMG8833 not found. Check your wiring or I2C ADDR!");
        while (1);
    }

    Serial.println("AMG8833 found!");

    // * Servos
    xServo.attach(X_SERVO_PIN);     // Attach servo on digital pin 10
    yServo.attach(Y_SERVO_PIN);     // Attach servo on digital pin 9

    // Move servos to the initial position
    freezeServos();

    // * Reset mode
    while (RESET_MODE) {
        Serial.println("Running in RESET_MODE");
        delay(SLEEP_MS);
    }
    
    // * Start
    if (!RESET_MODE) {
        // Idle before start
        Serial.println("Idling before start...");
        
        delay(IDLE_MS);
        
        Serial.println("STARTING!");
    }
}

void loop() {
    // * Matrices to store the data
    float rawMatrix[BASE_SIZE * BASE_SIZE];                                 // 8x8 matrix to store the temperature data
    int pixels[BASE_SIZE * BASE_SIZE];                                      // 8x8 matrix to convert the float matrix to an integer matrix
    int heatMatrix[BASE_SIZE * SCALE_FACTOR * BASE_SIZE * SCALE_FACTOR];    // 16x16 matrix to store the interpolated temperature data


    // * Read the temperature data when the button is pressed
    // Read the temperature data
    amg.readPixels(rawMatrix);

    // Convert the float matrix to an integer matrix
    floatToIntMatrixCast(rawMatrix, BASE_SIZE*BASE_SIZE, pixels);

    // Interpolate into 16x16 integer matrix
    interpolate(pixels, heatMatrix);

    // Locate the brightest value (the target) in the interpolated matrix
    coordinate target = locateTarget(heatMatrix, BASE_SIZE*SCALE_FACTOR);

    // Serial.println(target.value);
    

    // * Debug
    if (DEBUG) {
        displayTempratureMatrix<int>(heatMatrix, BASE_SIZE);
    }

    // ! Don't forget to free the memory, otherwise it will turbo rape the arduino's RAM
    delete[] heatMatrix;
    

    // * Move servos
    // Only seek the target if the value is above a certain threshold
    if (target.value > TARGET_VALUE_THRESHOLD) {
        seek(target);
    } else {
        freezeServos();
    }

    // * Debug
    if (DEBUG) {
        displayServoStates(target);

        delay(5000);

        Serial.println();
    }
    
}

// * Matrix functions
void floatToIntMatrixCast(const float floatMatrix[], const int& size, int intMatrix[]) {
    for (int i = 0; i < size; i++) {
        intMatrix[i] = static_cast<int>(floatMatrix[i] * 100);
    }
}

// * Interpolation functions
int cubicInterpolate(int p[4], const float& x) {
    // p: A list or array of values that are used for interpolation. In cubicInterpolate, it is a list of four values. In bicubicInterpolate, it is a 4x4 matrix of values.
    // x: The fractional distance along the x-axis for interpolation.

    // Perform cubic interpolation on a 1D array of 4 values
    // The coefficients are derived from the cubic Hermite spline interpolation formula
    
    return static_cast<int>(p[1] + 0.5 * x * (p[2] - p[0] + x * (2 * p[0] - 5 * p[1] + 4 * p[2] - p[3] + x * (3 * (p[1] - p[2]) + p[3] - p[0]))));
}


int bicubicInterpolate(int p[4][4], const float& x, const float& y) {
    // p: A list or array of values that are used for interpolation. In cubicInterpolate, it is a list of four values. In bicubicInterpolate, it is a 4x4 matrix of values.
    // x: The fractional distance along the x-axis for interpolation.
    // y: The fractional distance along the y-axis for interpolation (used only in bicubicInterpolate).
    
    // Perform cubic interpolation along the rows
    int array[4] = {
        cubicInterpolate(p[0], y),
        cubicInterpolate(p[1], y),
        cubicInterpolate(p[2], y),
        cubicInterpolate(p[3], y)
    };

    // Perform cubic interpolation along the columns
    return cubicInterpolate(array, x);
}


void interpolate(const int matrix[], int* interpolatedMatrix) {
    const int gridSize = BASE_SIZE * SCALE_FACTOR;
    const int matrixSize = gridSize * gridSize;

    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            // Find the x and y values on the input matrix corresponding to the current point (i, j) on the output matrix
            float x = (float)i / (SCALE_FACTOR) - 0.5;
            float y = (float)j / (SCALE_FACTOR) - 0.5;
            
            // Get the integer and fractional parts of the x and y values
            int x0 = int(x);
            int y0 = int(y);
            float dx = x - x0;
            float dy = y - y0;

            int p[4][4];
            for (int m = 0; m < 4; m++) {
                for (int n = 0; n < 4; n++) {
                    // Ensure that we don't go out of bounds when accessing the input matrix
                    int xm = min(max(x0 + m - 1, 0), BASE_SIZE - 1);
                    int yn = min(max(y0 + n - 1, 0), BASE_SIZE - 1);

                    // Get the values of the 16 points from the input matrix and store them in the 4x4 grid 
                    p[m][n] = matrix[xm * BASE_SIZE + yn];
                }
            }

            // Interpolate the value at the fractional position (x_frac, y_frac) using bicubic interpolation,
            // and store the interpolated value in the output matrix at position (i, j)
            interpolatedMatrix[i * gridSize + j] = bicubicInterpolate(p, dx, dy);
            
        }
    }

    // displayTempratureMatrix<int>(interpolatedMatrix, gridSize);

    // ? DEPRECATED: Changed so that the function instead alters an existing array passed by reference
    // // return interpolatedMatrix;
}


coordinate locateTarget(const int matrix[], const int& gridSize) {
    int matrixSize = gridSize * gridSize;

    // Find the maximum value in the interpolated matrix
    int val = matrix[0];
    int pos = 0;

    for (int i = 1; i < matrixSize; i++) {
        if (matrix[i] > val) {
            val = matrix[i];
            pos = i;
        }
    }

    // Convert the 1D position to 2D coordinates
    int x = pos % gridSize;
    int y = pos / gridSize;

    int offset = gridSize >> 1;

    return {x - offset, -(y - offset + 1), pos, val};   // Return the coordinates of the maximum value
}


// * Servo functions
void seek(coordinate target) {
    // Move the servos to the target position
    // int currentX = xServo.read();
    // int currentY = yServo.read();

    // Calculate the angles to move the servos
    int xAngle = target.angularDistanceToYZplane();
    int yAngle = target.angularDistanceToXYplane();

    if (DEBUG) {
        // * Target
        Serial.print("Target: (");
        Serial.print(target.x);
        Serial.print(", ");
        Serial.print(target.y);
        Serial.print(") [");
        Serial.print(target.index);
        Serial.println("]");

        Serial.print("xAngle: ");
        Serial.println(xAngle);
        Serial.print("yAngle: ");
        Serial.println(yAngle);
    }

    // Move the servos to the target position
    xServo.write(xAngle);
    yServo.write(yAngle);
}

void freezeServos() {
    // Set's the servos to the center position
    xServo.write(SERVO_CENTER);
    yServo.write(SERVO_CENTER);
}


// * Debugging functions
template <typename T>
void displayTempratureMatrix(const T matrix[], int gridSize) {    
    // Debugging function to display the temperature matrix
    Serial.println("Temperature data:");
    
    for (int i = 0; i < gridSize*gridSize; i++) {
        Serial.print(matrix[i]);
        Serial.print(", ");
        if ((i + 1) % gridSize == 0) Serial.println();
    }
    
    Serial.println();
}

void displayServoStates(coordinate target) {
    Serial.print("xServo : ");
    Serial.println(xServo.read());
    // Serial.print(" (T: ");
    // Serial.print(target.x);
    // Serial.println(")");

    Serial.print("yServo : ");
    Serial.println(yServo.read());
    // Serial.print(" (T: ");
    // Serial.print(target.y);
    // Serial.println(")");
}
