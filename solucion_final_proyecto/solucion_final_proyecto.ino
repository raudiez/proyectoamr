#include <Servo.h>
#include <math.h>

#define pinLED1 13
#define pinLED2 A0
#define pinLED3 2

#define CNY_Pin1 1
#define CNY_Pin2 2

#define pinMotor1 3
#define pinMotor2 4
#define pinMotor3 5
#define pinMotor4 6

//LDRs
const int LDR1 = 3, LDR2 = 4, LDR3 = 5;
double umbral = 2.5;
double lecturaLDR1 = 0, lecturaLDR2 = 0, lecturaLDR3 = 0;

//motor paso a paso
const int period = 4; // 2 ms cada paso

//CNYs
int lecturaCNY1 = 0, lecturaCNY2 = 0;

//Contadores para las caras de los cubos
int caras_analizadas = 0, negras = 0;

//Comunicación
double x = 0.0, y = 0.0, angulo = 0.0;
boolean varX = false, varY = false, varAnguloDecimal = false, colorCaraSuperior = false;
String auxiliar = "";
int colorSuperior = 0;
char letra;

//Servomotores
Servo myservo1, myservo2, myservo3, myservo4, myservo5, myservo6;

//Variables para el cálculo de la cinemática inversa
double cabeceo = 90 - 20.8456;// angulo en grados
double longitud_muneca = 80.5, longitud_brazo = 90, longitud_antebrazo = 90, altura_hombro = 64.23;
double alfa, beta, gamma;
double angulo_brazo, angulo_antebrazo, angulo_muneca;
double x_cuadrado, y_cuadrado, giro, modulo, x_prima, y_prima, afx, afy, ladoA, ladoB, hipotenusa, operacion_beta, operacion_gamma;

int estado_pinza = 0; // el 0 está abierto y el 1 estará la pinza cerrada

void setup() {
  Serial.begin(9600);

  pinMode(pinLED1,OUTPUT);
  digitalWrite(pinLED1,LOW);
  pinMode(pinLED2, OUTPUT);
  digitalWrite(pinLED2,LOW);
  pinMode(pinLED3, OUTPUT);
  digitalWrite(pinLED3,LOW);

  pinMode(pinMotor1, OUTPUT);
  pinMode(pinMotor2, OUTPUT);
  pinMode(pinMotor3, OUTPUT);
  pinMode(pinMotor4, OUTPUT);

  myservo1.attach(7); //servo del soporte giratorio! base
  myservo2.attach(8); //hombro
  myservo3.attach(9); //codo...sube o baja el brazo
  myservo4.attach(10); ///muñeca vertical
  myservo5.attach(11); //muñeca horizontal - mano
  myservo6.attach(12); //pinza

  calibrar_pinza();
  reposo();
  delay(1000);
}

void loop() {
// Se inicia la comunicación y se convierte el valor leído a double
// (o float en este caso, puesto que no existe función toDouble() de
// String), para después pasarlo a la función mover_brazo

 /* ///PRUEBAS DE MOVIMIENTO A PLATAFORMA
  plataforma();
  delay(500);
  subir_brazo();// FIN PRUEBAS DE MOVIMIENTO A PLATAFORMA */
  /*// PRUEBA DE CNYS
  Serial.print("CNY abajo :");
  Serial.println(lectura_CNY(CNY_Pin2));
  Serial.println(lectura_CNY(CNY_Pin1));
  mover_motor();
  stopMotor();
  delay(50);
  Serial.println(lectura_CNY(CNY_Pin1));
  mover_motor();
  stopMotor();
  delay(50);
  Serial.println(lectura_CNY(CNY_Pin1));
  mover_motor();
  stopMotor();
  delay(50);
  Serial.println(lectura_CNY(CNY_Pin1));
  mover_motor();
  stopMotor();
  delay(50);
  vuelve_motor();//FIN PRUEBA DE CNYS*/


  if(Serial.available()){
    String lecturaSerie = Serial.readString();
    auxiliar = "";
    colorCaraSuperior = false;
    varAnguloDecimal = false;
    varX = false;
    varY = false;

    for (int i = 0; i < lecturaSerie.length();i++){
      if(!colorCaraSuperior){
        letra = lecturaSerie.charAt(i);
        if(letra !=','){
          auxiliar += letra;
        }else{
          colorCaraSuperior = true;
          colorSuperior = auxiliar.toInt();
          auxiliar = "";
        }
      }else if (!varAnguloDecimal){
        letra = lecturaSerie.charAt(i);
        if(letra !=','){
          auxiliar += letra;
        }else{
          varAnguloDecimal = true;
          angulo = auxiliar.toFloat();
          auxiliar = "";
        }
      }else if(!varX){
        letra = lecturaSerie.charAt(i);
        if(letra !=','){
          auxiliar += letra;
        }else{
          varX = true;
          x = auxiliar.toFloat();
          auxiliar = "";
        }
      }else if (!varY){
        letra = lecturaSerie.charAt(i);
        if(letra !=';'){
          auxiliar += letra;
        }else{
          varY = true;
          y = auxiliar.toFloat();
          auxiliar = "";
        }
      }//if-else
    }//for
    trabajo_brazo();
  }//if-serial.available*/
  comprueba_LDRs();
}//loop

void trabajo_brazo(){
  mover_brazo(x,y,-4.0,angulo); //Se le manda la x e y recibidas desde la Raspberry
  delay(500);
  //cerrar_pinza();
  delay(500);
  subir_brazo();

 /* // LINEAS DE PRUEBA DE TRABAJO BRAZO SIN LECTURAS DE CNY
  plataforma2();
  delay(500);
  abrir_pinza();
  delay(500);
  subir_brazo();
  for(int i = 0; i<=3;i++){
    mover_motor();
    stopMotor();
    delay(50);
  }
  delay(30);
  vuelve_motor();
  plataforma();
  delay(500);
  cerrar_pinza();
  delay(500);
  subir_brazo();
  cubeta3();// FIN LINEAS DE PRUEBA*/
/*
  plataforma();
  delay(500);
  abrir_pinza();
  delay(500);
  subir_brazo();
  for(int i = 0; i<=3;i++){
    lecturaCNY1 = lectura_CNY(CNY_Pin1);
    if(lecturaCNY1 > 600){
      negras++;
    }
    caras_analizadas++;
    mover_motor();
    stopMotor();
    delay(50);
  }
  delay(30);
  vuelve_motor();

  lecturaCNY2 = lectura_CNY(CNY_Pin2);
  if(lecturaCNY2 > 600){
    negras++;
  }
  caras_analizadas++;
  if(colorSuperior < 100){
    negras++;
  }
  caras_analizadas++;
  plataforma();
  delay(500);
  cerrar_pinza();
  delay(500);
  subir_brazo();
  if(negras == 0){
    cubeta1();
  }else if(negras == 1){
    cubeta2();
  }else if(negras == 2){
    cubeta3();
  } //*/

  //Serial.write("terminado\n"); //Si no funcionase, cambiar por Serial.println
}

void vuelve_motor(){
  mover_motor_inv();
  stopMotor();
  delay(50);
  mover_motor_inv();
  stopMotor();
  delay(50);
  mover_motor_inv();
  stopMotor();
  delay(50);
  mover_motor_inv();
  stopMotor();
  delay(50);
}

void mover_motor(){
  for (int i = 0; i < 128; i++){
    step1();
    step2();
    step3();
    step4();
  }
}

void mover_motor_inv(){
  for (int i = 0; i < 128; i++){
    step4();
    step3();
    step2();
    step1();
  }
}

void comprueba_LDRs(){
  lecturaLDR1 = lectura_LDR(LDR1);
  if(lecturaLDR1 > umbral){
    digitalWrite(pinLED1, HIGH);
  } else {
    digitalWrite(pinLED1,LOW);
  }

  lecturaLDR2 = lectura_LDR(LDR2);
  if(lecturaLDR2 > umbral){
    digitalWrite(pinLED2, HIGH);
  } else {
    digitalWrite(pinLED2,LOW);
  }

  lecturaLDR3 = lectura_LDR(LDR3);
  if(lecturaLDR3 > umbral){
    digitalWrite(pinLED3, HIGH);
  } else {
    digitalWrite(pinLED3,LOW);
  }
}

double lectura_LDR(const int pin){
  double valorLDR = analogRead(pin);
  valorLDR *= 0.0049;
  return valorLDR;
}

int lectura_CNY(const int pin){
  int Valor_CNY = analogRead(pin);
  return Valor_CNY;
}

void mover_brazo(double x, double y, double z, double angulo_pinza){
  if(x > 0){ x+=30.0; y-=23;}
  calcula_angulos(x,y+50.0,75.0+z,angulo_pinza);
  delay(1000);
}

void cubeta1(){
  myservo1.write(13);
  mapeo_servo4(130);
  mapeo_servo2(84);
  mapeo_servo3(50);
  delay(1000);
  abrir_pinza();
  delay(500);
  comprueba_LDRs();
  mapeo_servo2(120);
  delay(1000);
  reposo();
}

void cubeta2(){
  myservo1.write(30);
  mapeo_servo4(130);
  mapeo_servo2(84);
  mapeo_servo3(50);
  delay(1000);
  abrir_pinza();
  delay(500);
  comprueba_LDRs();
  mapeo_servo2(120);
  delay(1000);
  reposo();
}

void cubeta3(){
  myservo5.write(15);
  myservo1.write(45);
  mapeo_servo4(130);
  mapeo_servo2(84);
  mapeo_servo3(50);
  delay(1000);
  abrir_pinza();
  delay(500);
  comprueba_LDRs();
  mapeo_servo2(120);
  delay(1000);
  reposo();
}

void plataforma(){
  myservo5.write(161);
  myservo1.write(180);
  mapeo_servo4(120);
  mapeo_servo2(97);
  mapeo_servo3(49);
}

void plataforma2(){
  myservo5.write(161);
  myservo1.write(180);
  mapeo_servo4(116);
  mapeo_servo2(102);
  mapeo_servo3(53);
}

void subir_brazo(){
  mapeo_servo2(145);
  reposo();
}

void mapeo_servo1(double angulo){
  double angulonuevo = map(angulo,0,180,20,190);
  if(angulonuevo > 180){
    angulonuevo = 180;
  }
  double anguloactual = myservo1.read();
  if(angulonuevo >= anguloactual){
    for(int i=anguloactual;i<=angulonuevo;i++){
      myservo1.write(i);
      delay(15);
    }
  }else{
    for(int i=anguloactual;i>=angulonuevo;i--){
      myservo1.write(i);
      delay(15);
    }
  }
}

void mapeo_servo2(double angulo){
  double angulonuevo = map(angulo,0,180,-10,170);
  double anguloactual = myservo2.read();
  if(angulonuevo >= anguloactual){
  for(int i=anguloactual;i<=angulonuevo;i++){
      myservo2.write(i);
      delay(15);
    }
  }else{
    for(int i=anguloactual;i>=angulonuevo;i--){
      myservo2.write(i);
      delay(15);
    }
  }
}

void mapeo_servo3(double angulo){
 double angulonuevo = map(angulo,0,180,-21,159);
 double anguloactual = myservo3.read();
  if(angulonuevo >= anguloactual){
  for(int i=anguloactual;i<=angulonuevo;i++){
      myservo3.write(i);
      delay(15);
    }
  }else{
    for(int i=anguloactual;i>=angulonuevo;i--){
      myservo3.write(i);
      delay(15);
    }
  }
}

void mapeo_servo4(double angulo){
 double angulonuevo = map(angulo,0,180,180,0);  //NO necesita mapeo
 double anguloactual = myservo4.read();
  if(angulonuevo >= anguloactual){
  for(int i=anguloactual;i<=angulonuevo;i++){
      myservo4.write(i);
      delay(15);
    }
  }else{
    for(int i=anguloactual;i>=angulonuevo;i--){
      myservo4.write(i);
      delay(15);
    }
  }
}

void mapeo_servo5(double angulo){
  double angulonuevo = map(angulo,0,180,16,180);
  double anguloactual = myservo5.read();
  if(angulonuevo >= anguloactual){
    for(int i=anguloactual;i<=angulonuevo;i++){
      myservo5.write(i);
      delay(15);
    }
  }else{
    for(int i=anguloactual;i>=angulonuevo;i--){
      myservo5.write(i);
      delay(15);
    }
  }
}

void abrir_pinza(){
  if(estado_pinza == 1){
  myservo6.write(90);
  estado_pinza = 0;
 }
}

void cerrar_pinza(){
  if(estado_pinza == 0){
  myservo6.write(20);
  estado_pinza = 1;
 }
}

void reposo(){
  mapeo_servo1(90);
  mapeo_servo2(145);
  mapeo_servo3(60);
  mapeo_servo4(60);
  mapeo_servo5(90);
}

void calibrar_pinza(){
  //Todo esto es para calibrar la pinza
  myservo6.write(180);
  delay(1000);
  myservo6.write(0);
  delay(1000);
  myservo6.write(30);
  delay(1000);
  myservo6.write(90);
  delay(1000);
}

void calcula_angulos(double x, double y, double z, double angulo_pinza){
  x_cuadrado = pow(x,2);
  y_cuadrado = pow(y,2);

  giro = asin(abs(x)/(sqrt(x_cuadrado+y_cuadrado)));
  giro = (giro*360)/(2*PI);
  if(x > 0){
    giro = giro + 90;
  }else {
     giro =  90 - giro;
  }

  modulo = (sqrt(x_cuadrado+y_cuadrado))-18.3;
  y_prima = z;
  afx = abs(cos(cabeceo))*longitud_muneca;

  x_prima = modulo;
  ladoB = x_prima - afx;
  afy = abs(sin(cabeceo))*longitud_muneca;
  ladoA = y_prima + afy - altura_hombro;

  hipotenusa = sqrt(pow(ladoA,2)+pow(ladoB,2));
  alfa = abs(atan2(ladoA,ladoB));
  operacion_beta = ((pow(longitud_brazo,2)) - (pow(longitud_antebrazo,2)) + (pow(hipotenusa,2)))/(2*longitud_brazo*hipotenusa);
  beta = abs(acos(operacion_beta));

  angulo_brazo = ((alfa*360)/(2*PI))+((beta*360)/(2*PI));
  operacion_gamma = ((pow(longitud_brazo,2)) + (pow(longitud_antebrazo,2)) - (pow(hipotenusa,2)))/(2*longitud_brazo*longitud_antebrazo);
  gamma = ((acos(operacion_gamma))*360)/(2*PI);
  angulo_antebrazo = gamma;
  angulo_muneca = 180+(cabeceo- angulo_brazo - angulo_antebrazo);

  mapeo_servo1(giro);
  //Para que siga estando la pinza recta respecto al rectángulo de referencia:
  double giro_pinza = giro - 90;
  //Para girar lo que esté girado el cubo respecto a su eje:
  giro_pinza = giro_pinza - angulo_pinza;
  mapeo_servo5(90+giro_pinza);

  mapeo_servo4(angulo_muneca+20);
  if (angulo_antebrazo <=38){
    mapeo_servo3(38);
    mapeo_servo2(angulo_brazo-(38-angulo_antebrazo)+3);
  }else{
    mapeo_servo3(angulo_antebrazo+1);
    mapeo_servo2(angulo_brazo+1);
  }
}

void step1(){
 digitalWrite(pinMotor1,HIGH);
 digitalWrite(pinMotor2,HIGH);
 digitalWrite(pinMotor3,LOW);
 digitalWrite(pinMotor4,LOW);
 delay(period);
}

void step2(){
 digitalWrite(pinMotor1,LOW);
 digitalWrite(pinMotor2,HIGH);
 digitalWrite(pinMotor3,HIGH);
 digitalWrite(pinMotor4,LOW);
 delay(period);
}

void step3(){
 digitalWrite(pinMotor1,LOW);
 digitalWrite(pinMotor2,LOW);
 digitalWrite(pinMotor3,HIGH);
 digitalWrite(pinMotor4,HIGH);
 delay(period);
}

void step4(){
 digitalWrite(pinMotor1,HIGH);
 digitalWrite(pinMotor2,LOW);
 digitalWrite(pinMotor3,LOW);
 digitalWrite(pinMotor4,HIGH);
 delay(period);
}

void stopMotor() {
 digitalWrite(pinMotor1,LOW);
 digitalWrite(pinMotor2,LOW);
 digitalWrite(pinMotor3,LOW);
 digitalWrite(pinMotor4,LOW);
}
