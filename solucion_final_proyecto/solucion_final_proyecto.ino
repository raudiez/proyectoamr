//LEDs
int pinLED1 = 2, pinLED2 = 13, pinLED3 = A0;

//LDRs
int LDR1 = A3, LDR2 = A4, LDR3 = A5; 
int valorLDR1 = 0, valorLDR2 = 0, valorLDR3 = 0;
int umbral = 800, umbral2 = 600, umbral3 = 800;

//motor paso a paso
const int period = 2; // 8 ms between each step
const int A = 3;
const int B = 4;
const int C = 5;
const int D = 6;

//CNYs
const int CNY_Pin1 = A1, CNY_Pin2 = A2;
int Valor_CNY1 = 0, Valor_CNY2 = 0;
int lecturaCNY1 = 0, lecturaCNY2 = 0;

//Contadores para las caras de los cubos
int caras_analizadas = 0, blancas = 0, negras = 0, blancas2 = 0, negras2 = 0, total_caras = 0;

void setup() {
 Serial.begin(9600);
 
 pinMode(pinLED1, OUTPUT);
 pinMode(LDR1,INPUT);
 digitalWrite(pinLED1,LOW);
  
 pinMode(pinLED2, OUTPUT);
 pinMode(LDR2,INPUT);
 digitalWrite(pinLED2,LOW);
 
 
 pinMode(pinLED3,OUTPUT);
 pinMode(LDR3,INPUT);
 analogWrite(pinLED3,0); //Esto no hace falta al ser entradas análogicaso
 //digitalWrite(pinLED3,LOW); //por si acaso
 
 pinMode(A, OUTPUT);
 pinMode(B, OUTPUT);
 pinMode(C, OUTPUT);
 pinMode(D, OUTPUT);

}

void loop() {
  
  lectura_LDR1();
  lectura_LDR2();
  lectura_LDR3();
  
  delay(3000);
  
 for(int i = 0; i <=3; i++){
   lecturaCNY1 = lectura_CNY1();
    if(lecturaCNY1 < 200){
        Serial.println("Cara blanca detectada");
        blancas++;
    } else if(lecturaCNY1 > 200){
        Serial.println("Cara negra detectada");
        negras++;
    } else{} //Si no es ni blanca ni negra no hace nada. Esto creo que se podría quitar pero de momento se deja
   caras_analizadas++;
   mover_motor();
   delay(2000);
   //stopMotor(); //Esto no se si hace falta debido a los delays que ya posee
 }
 
   Serial.print("Hay ");
   Serial.print(negras);
   Serial.println(" caras negras detectadas por el CNY del motor");
 
   Serial.print("Hay ");
   Serial.print(blancas);
   Serial.println(" caras blancas detectadas por el CNY del motor");
   
  lecturaCNY2 = lectura_CNY2();
    if(lecturaCNY2 < 200){
        Serial.println("Cara blanca detectada por el CNY de abajo");
        blancas2 = 1;
        caras_analizadas++;
   } else if(lecturaCNY2 > 200){
        Serial.println("Cara negra detectada por el CNY de abajo");
        negras2 = 1;
        caras_analizadas++;
   } else{}
   
   total_caras = blancas + negras + blancas2 + negras2; //Habría que sumarle también la cara detectada por la cámara

   Serial.print("En total hay ");
   Serial.print(total_caras);
   Serial.println(" caras de las cuales hay ");
   Serial.print(blancas+blancas2);
   Serial.println(" blancas y ");
   Serial.print(negras+negras2);
   Serial.println(" negras");
 
}

void lectura_LDR1(){

  valorLDR1 = analogRead(LDR1); 
  Serial.print("LDR1: ");
  Serial.println(analogRead(valorLDR1)); //Escribimos el valor en monitor serie
   
    //monitorizando este valor podemos  ajustar el umbral para encender el led
    //en función de la luminosidad
    if (valorLDR1 < umbral)// valor experimental
    {
      digitalWrite(pinLED1,HIGH);// si la luminosidad es mayor apagamos el led
    }    
    else
    {
      digitalWrite(pinLED1,LOW);
    }
   Serial.println("");
   delay(1000);

}

void lectura_LDR2(){

  valorLDR2 = analogRead(LDR2);
   Serial.print("LDR2: "); 
   Serial.println(analogRead(valorLDR2)); //Escribimos el valor en monitor serie
  
    //monitorizando este valor podemos  ajustar el umbral para encender el led
    //en función de la luminosidad
    if (valorLDR2 < umbral2)// valor experimental
    {
      digitalWrite(pinLED2,HIGH);// si la luminosidad es mayor apagamos el led
    }    
    else
    {
      digitalWrite(pinLED2,LOW);// en caso contrario encendemos el led
    }
    
   Serial.println("");
   delay(1000);
   

}

void lectura_LDR3(){
   valorLDR3 = analogRead(LDR3);
   Serial.print("LDR3: "); 
   Serial.println(analogRead(valorLDR3)); //Escribimos el valor en monitor serie
    
    //monitorizando este valor podemos  ajustar el umbral para encender el led
    //en función de la luminosidad
    if (valorLDR3 < umbral3)// valor experimental
    {
      analogWrite(pinLED3,1024);// si la luminosidad es mayor apagamos el led
    }    
    else
    {
     analogWrite(pinLED3,0);// en caso contrario encendemos el led
    }
    Serial.println(""); 
   delay(1000); // A mayor valor mas lenta sera la respuesta a los cambios de luminosidad
}

int lectura_CNY1(){
  
 Valor_CNY1 = analogRead(CNY_Pin1);
 return Valor_CNY1;
 
 }


int lectura_CNY2(){

 Valor_CNY2 = analogRead(CNY_Pin2);
 return Valor_CNY2;

}

void mover_motor(){
 for (int i = 0; i < 512; i++){
   step1();
   step2();
   step3();
   step4();
  }

}

void step1(){
 digitalWrite(A,HIGH);
 digitalWrite(B,HIGH);
 digitalWrite(C,LOW);
 digitalWrite(D,LOW);
 delay(period);
}

void step2(){
 digitalWrite(A,LOW);
 digitalWrite(B,HIGH);
 digitalWrite(C,HIGH);
 digitalWrite(D,LOW);
 delay(period);
}

void step3(){
 digitalWrite(A,LOW);
 digitalWrite(B,LOW);
 digitalWrite(C,HIGH);
 digitalWrite(D,HIGH);
 delay(period);
}

void step4(){
 digitalWrite(A,HIGH);
 digitalWrite(B,LOW);
 digitalWrite(C,LOW);
 digitalWrite(D,HIGH);
 delay(period);
}

void stopMotor() {
 digitalWrite(A,LOW);
 digitalWrite(B,LOW);
 digitalWrite(C,LOW);
 digitalWrite(D,LOW);
}
