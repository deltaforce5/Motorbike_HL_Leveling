#define DEV_TYPE ATMEGA
#define PINTOINT(port) digitalPinToInterrupt(port)
#define L_BUILTIN(state) digitalWrite(LED_BUILTIN, state)
#define S_BUILTIN() pinMode(LED_BUILTIN, OUTPUT);

//** Define IOs **//
#define MOTOR1 8  // Motor pin #1
#define MOTOR2 9  // Motor pin #2
#define BUTTON 3 // Push button pin
#define FEEDBACK 2 // Motor feedback pin
#define LED01 6 // First dual led output
#define LED23 5 // First dual led output

void logger(String custom = "") {
  Serial.print(custom);
}

void custom_init() {
  Serial.begin(9600);
}
