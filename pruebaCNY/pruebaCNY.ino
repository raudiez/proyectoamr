
#define CNY1 A1
#define CNY2 A2


int valorCNY1 = 0, valorCNY2 = 0;


//menor de 200 --> blanco
//mayor de 200 --> negro

void setup(){
    Serial.begin(9600);
}

void loop(){
    valorCNY1 =analogRead(CNY1);
    Serial.print("Valor CNY: ");
    Serial.println(valorCNY1);
    delay(2000);
    
    valorCNY2 =analogRead(CNY2);
    Serial.print("Valor CNY: ");
    Serial.println(valorCNY2);
    delay(2000);
}
