
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
  
  Serial.begin(115200);
}

void writeLED(int color, int intensity)
{
    analogWrite(LED_RED,  intensity*(color & 1));  
    analogWrite(LED_GREEN,  intensity*((color & 2)>> 1));
    analogWrite(LED_BLUE,  intensity*((color & 4) >> 2));    
}

void loop()
{
  int color = 1;
  writeLED(color, 25);
  while(1) {
    int k = analogRead(A3);
    if (k < 300) {
      color = (color + 1) & 0x07;
      if (color == 0)
        color = 1;
        
      writeLED(color, 50);
   
//    Serial.println(k);
//    delay(30);
    }
  }
}
