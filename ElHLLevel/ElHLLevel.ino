#include <EEPROM.h>
//#include "./ElHLLevel_ATMega.h"
#include "./ElHLLevel_ATTiny.h"

//** Define  **//
#define LOOPMS 150 // main loop delay in ms
#define MOTORLOOPS 13 // LOOPMS loops to wait before motor starts to move
#define SETUP 20 // # of LOOPMS loops to wait to enter setup mode
#define RESETLOOPS 66 // # of LOOPMS to wait to reset the eeprom
#define EERESETLOOPS 100 // # of LOOPMS to wait to reset the eeprom

#define TURNS 4 // Turns for each setting diff - TODO change to user defined array

#define EEADDR 0 // Base eeprom address
#define MAXSET 4  // The number of sets available - /!\ It affects the array of settings, the 

int pos = 0;
int p_pos = 0;
int m_pos = 0;
int ee_addr = 0; //EEPROM address to start reading from
//***** EEPROM ************
// Addr 0: last pos
// Addr 1: last m_pos
// Addr 2: last m_delay
//*************************

int pos_wait = 0; 
int m_wait = 0;
int m_ticks = 0;
int m_delay = 0;

void ledSet(bool led1 = false, bool led2 = false, bool led3 = false, bool led4 = false)  {
  if (led1 && led2) {
    pinMode(LED01, OUTPUT);
    analogWrite(LED01, 127);
  } else if (led1 && !led2) {
    pinMode(LED01, OUTPUT);
    analogWrite(LED01, 0);
  } else if (!led1 && led2) {
    pinMode(LED01, OUTPUT);
    analogWrite(LED01, 255);   
  } else {
    pinMode(LED01, INPUT);    
  }    

  if (led3 && led4) {
    pinMode(LED23, OUTPUT);
    analogWrite(LED23, 127);
  } else if (led3 && !led4) {
    pinMode(LED23, OUTPUT);
    analogWrite(LED23, 0);
  } else if (!led3 && led4) {
    pinMode(LED23, OUTPUT);
    analogWrite(LED23, 255);   
  } else {
    pinMode(LED23, INPUT);    
  } 
}

// Set leds
void led_state(bool on) { //pattern, int ) {
  if (on)
    ledSet(pos>=0, pos>=1, pos>=2, pos>=3);
  else
    ledSet();
}

// move the motor, choosing and returning the direction
int move_motor() {
  int diff = pos-m_pos;
  logger("diff: ");
  logger(String(diff));
  logger(" - m_pos: ");
  logger(String(m_pos));
  logger(" - pos: ");
  logger(String(pos));
  logger("\n");

  if ( diff > 0 ) {
    diff = 1;
    digitalWrite(MOTOR1, HIGH);
    digitalWrite(MOTOR2, LOW);
  } else if ( diff < 0 ) {
    diff = -1;
    digitalWrite(MOTOR2, HIGH);
    digitalWrite(MOTOR1, LOW);
  } else {
    digitalWrite(MOTOR1, LOW);
    digitalWrite(MOTOR2, LOW);
    led_state(true);
    m_wait = 0;
  }
  return diff;
}
void FSM_EEReset() {
  ledSet( true, true, true, true);
  
  pos = 0;
  m_pos = 0;
  m_delay = 0;

  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.put(ee_addr, pos);
  EEPROM.put(ee_addr+1, m_pos);
  EEPROM.put(ee_addr+2, m_delay);

  delay(LOOPMS * 6);
  ledSet( false, false, false, false);
}

void FSM_Error() {
  bool blink = false;
  int btn_wait = 0;
  bool loop = true;

  digitalWrite(MOTOR1, LOW);
  digitalWrite(MOTOR2, LOW);

  while (loop) {
    if ( digitalRead(BUTTON)==LOW )
      btn_wait++;
    else if ( btn_wait >= EERESETLOOPS ) {
      FSM_EEReset();
      break;
    } else if ( btn_wait >= RESETLOOPS )
      break;
    else
      btn_wait = 0;
   
    ledSet( blink, !blink, !blink, blink);
    blink = !blink;
    delay(LOOPMS);
  }
  
  btn_wait = 0;
  
}

// Uninterruptible Motor Turning state
void FSM_MT() {
  int m_dir = 0;
  int p_m_ticks = m_ticks;

  do {
    m_dir = move_motor();
    m_delay = m_delay + ( m_dir * m_dir );

    if ( m_delay >= ( TURNS * m_dir * m_dir ) ) {
      m_delay = 0;
      m_pos += m_dir;
    }

    delay(LOOPMS);

    if (p_m_ticks == m_ticks && m_dir != 0)
      FSM_Error();
    else
      p_m_ticks = m_ticks;

  } while ( m_dir != 0 );
  m_ticks=0;
  EEPROM.put(ee_addr+1, m_pos);
  EEPROM.put(ee_addr+2, m_delay);
}

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pins
  pinMode(MOTOR1, OUTPUT);
  pinMode(MOTOR2, OUTPUT);
  pinMode(LED01, OUTPUT);
  pinMode(LED23, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(FEEDBACK, INPUT);
  S_BUILTIN();

  attachInterrupt(PINTOINT(FEEDBACK), ISR_fb, FALLING);
 
  int savedSet = 0; //Variable to store data read from EEPROM
  
  //Get the data from the EEPROM at position 'ee_addr'
  EEPROM.get(ee_addr, pos);
  EEPROM.get(ee_addr+1, m_pos);
  EEPROM.get(ee_addr+2, m_delay);

  logger(" - m_pos: ");
  logger(String(m_pos));
  logger(" - pos: ");
  logger(String(pos));
  logger("\n");

  custom_init();
  
  // // if eeprom data is corrupted, reset the savedSet
  // if (savedSet < 0 || savedSet > MAXSET) {
  //   savedSet = 0;
  // }
  // pos=savedSet;

  if (pos != m_pos)
    FSM_MT(); 

}

// the loop function runs over and over again forever
void loop() {  
  if ( digitalRead(BUTTON)==LOW ) {
    pos_wait++;
  } else {
    if ( pos_wait >= SETUP ) {
      logger("pos setup!\n");
      L_BUILTIN(HIGH);
      pos_wait = 0;
    } else {
      if ( pos_wait > 0 ) {
        pos=(pos+1)%MAXSET;
        pos_wait = 0;
        L_BUILTIN(LOW);
      }
    }
  }
  
  // Update LEDs and EEPROM state
  if ( pos != p_pos ) { 
    led_state(true);
    EEPROM.put(ee_addr, pos);
    p_pos = pos;
    m_wait = 1;
  }

  // Turn motor, if needed, after some time
  if ( m_wait > 0 && m_wait < MOTORLOOPS && pos_wait == 0 ) 
    led_state( ((++m_wait)%4)>1 );
  else if ( m_wait >= MOTORLOOPS && pos_wait == 0 )
    FSM_MT();
  else 
    m_wait = 0;

  delay(LOOPMS); // repeat every LOOPMS ms
}

void ISR_fb() {
  m_ticks++;
}
