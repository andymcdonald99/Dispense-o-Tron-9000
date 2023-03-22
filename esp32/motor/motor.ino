const int DIR = 12;
const int STEP = 13;
const int  steps_per_rev = 1000;

void setup()
{
  Serial.begin(115200);
  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
}
void loop()
{
  digitalWrite(DIR, LOW);
  Serial.println("Spinning Clockwise...");
  
  for(int i = 0; i<steps_per_rev; i++)
  {
    digitalWrite(STEP, HIGH);
    delayMicroseconds(300);
    digitalWrite(STEP, LOW);
    delayMicroseconds(300);
  }
  delay(1000); 
  
}
