const int LDR1 = 5, LDR2 = 4, LDR3 = 3;
int LDR_val = 0, LDR_val2 = 0, LDR_val3 = 0;

void setup() {
 Serial.begin(9600);
}


void loop(){
 LDR_val = analogRead(LDR1);
 Serial.print("Tension LDR1 = ");
 Serial.println(LDR_val*0.0049);

 delay(1000);

 LDR_val2 = analogRead(LDR2);
 Serial.print("Tension LDR2 = ");
 Serial.println(LDR_val2*0.0049);

 delay(1000);

 LDR_val3 = analogRead(LDR3);
 Serial.print("Tension LDR3 = ");
 Serial.println(LDR_val3*0.0049);

 delay(1000);
}

