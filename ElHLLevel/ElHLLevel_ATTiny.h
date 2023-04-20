#define DEV_TYPE ATTINY
#define PINTOINT(port) digitalPinToPort(port)
#define L_BUILTIN(state) void_f()
#define S_BUILTIN() void_f()

//** Define IOs **//
#define MOTOR1 5  // Motor pin #1
#define MOTOR2 3  // Motor pin #2
#define BUTTON 2 // Push button pin
#define FEEDBACK 4 // Motor feedback pin
#define LED01 1 // First dual led output
#define LED23 0 // First dual led output

void logger(String custom = "") {
  return;
}

void custom_init() {
  return;
}

void void_f() {
  return;
}
