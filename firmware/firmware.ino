
// Digital I/O
#define NRF_CE    8
#define NRF_CS    9
#define LED_RED   14
#define LED_GREEN   5
#define ACCEL_INT1 1
#define ACCEL_INT2 2
#define ACCEL_CS     4
#define LED_BLUE   15
#define LD2     6
#define LD1     7
#define VSENSE A1
#define PIEZO1 A3
#define PIEZO2 A2


// Analog I/O
void setup()
{
  // Pin Setup
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
}

void loop()
{
  /*
  digitalWrite(LED_RED, 1);  
  digitalWrite(LED_GREEN, 0);
  digitalWrite(LED_BLUE, 0);  
  delay(500);

  digitalWrite(LED_RED, 0);  
  digitalWrite(LED_GREEN, 1);
  digitalWrite(LED_BLUE, 0);  
  delay(500);

  digitalWrite(LED_RED, 0);  
  digitalWrite(LED_GREEN, 0);
  digitalWrite(LED_BLUE, 1);  
  delay(500);
  */
  int i;
  for(i=0; i < 100; i++) {
    analogWrite(LED_RED, i);
    delay(50);
  }
  for(i=0; i < 100; i++) {
    analogWrite(LED_GREEN, i);
    delay(50);
  }
  for(i=0; i < 100; i++) {
    analogWrite(LED_BLUE, i);
    delay(50);
  }  
}
