#include <Servo.h>
#include <math.h>

Servo myservo1, myservo2, myservo3, myservo4, myservo5, myservo6;  //create servo object

double cabeceo = 90-20.8456;// angulo en grados
double longitud_muneca = 80.5;
double longitud_brazo = 90;
double longitud_antebrazo = 90;
double altura_hombro = 64.23;
double alfa, beta, gamma;
double angulo_brazo, angulo_antebrazo, angulo_muneca;

int estado_pinza = 0; // el 0 está abierto y el 1 estará la pinza cerrada

String cadena = "";

void setup(){
  Serial.begin(9600);
  myservo1.attach(7); //servo del soporte giratorio! base
  myservo2.attach(8); //hombro
  myservo3.attach(9); //codo...sube o baja el brazo
  myservo4.attach(10); ///muñeca vertical
  myservo5.attach(11); //muñeca horizontal - mano
  myservo6.attach(12); //pinza
  calibrar_pinza();
}

void loop(){
  reposo();
  delay(1000);
  abrir_pinza();
  delay(1000);
  /*
  if (Serial.available()){

    cadena = Serial.readString();
    double val = atof(cadena);
    for(int i=0, i< cadena.

  }*/
  mover_brazo(49.0,101.0,-4.0);
  delay(1000);
  cerrar_pinza();
  delay(1000);
  subir_brazo();
  delay(1000);
  cubeta3();
  delay(1000);
  abrir_pinza();
  delay(1000);
  subir_brazo();
  delay(1000);
}

void mover_brazo(double x, double y, double z){
  if(x > 0){ x+=30.0; y-=23;}
  calcula_angulos(x,y+50.0,75.0+z);
  delay(1000);
}

void cubeta1(){
  myservo1.write(13);
  mapeo_servo4(130);
  mapeo_servo2(84);
  mapeo_servo3(50);
}

void cubeta2(){
  myservo1.write(30);
  mapeo_servo4(130);
  mapeo_servo2(84);
  mapeo_servo3(50);
}

void cubeta3(){
  myservo1.write(48);
  mapeo_servo4(130);
  myservo6.write(0);
  mapeo_servo2(84);
  mapeo_servo3(50);
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
      delay(30);
    }
  }else{
    for(int i=anguloactual;i>=angulonuevo;i--){
      myservo1.write(i);
      delay(30);
    }
  }
}
void mapeo_servo2(double angulo){
  double angulonuevo = map(angulo,0,180,-10,170);
  double anguloactual = myservo2.read();
  if(angulonuevo >= anguloactual){
  for(int i=anguloactual;i<=angulonuevo;i++){
      myservo2.write(i);
      delay(30);
    }
  }else{
    for(int i=anguloactual;i>=angulonuevo;i--){
      myservo2.write(i);
      delay(30);
    }
  }
}
void mapeo_servo3(double angulo){
 double angulonuevo = map(angulo,0,180,-21,159);
 double anguloactual = myservo3.read();
  if(angulonuevo >= anguloactual){
  for(int i=anguloactual;i<=angulonuevo;i++){
      myservo3.write(i);
      delay(30);
    }
  }else{
    for(int i=anguloactual;i>=angulonuevo;i--){
      myservo3.write(i);
      delay(30);
    }
  }
}
void mapeo_servo4(double angulo){
 double angulonuevo = map(angulo,0,180,180,0);  //NO necesita mapeo
 double anguloactual = myservo4.read();
  if(angulonuevo >= anguloactual){
  for(int i=anguloactual;i<=angulonuevo;i++){
      myservo4.write(i);
      delay(30);
    }
  }else{
    for(int i=anguloactual;i>=angulonuevo;i--){
      myservo4.write(i);
      delay(30);
    }
  }
}
void mapeo_servo5(double angulo){
  double angulonuevo = map(angulo,0,180,16,180);
  double anguloactual = myservo5.read();
  if(angulonuevo >= anguloactual){
    for(int i=anguloactual;i<=angulonuevo;i++){
      myservo5.write(i);
      delay(30);
    }
  }else{
    for(int i=anguloactual;i>=angulonuevo;i--){
      myservo5.write(i);
      delay(30);
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

void calcula_angulos(double x, double y, double z){
  double x_cuadrado = pow(x,2);
  double y_cuadrado = pow(y,2);
  double giro = asin(abs(x)/(sqrt(x_cuadrado+y_cuadrado)));
  giro = (giro*360)/(2*PI);
  if(x > 0){
    giro = giro + 90;
  }else {
     giro =  90 - giro;
  }
  double modulo = (sqrt(x_cuadrado+y_cuadrado))-18.3;
  double y_prima = z;
  double afx = abs(cos(cabeceo))*longitud_muneca;
  double x_prima = modulo;
  double ladoB = x_prima - afx;
  double afy = abs(sin(cabeceo))*longitud_muneca;
  double ladoA = y_prima + afy - altura_hombro;
  double hipotenusa = sqrt(pow(ladoA,2)+pow(ladoB,2));
  alfa = abs(atan2(ladoA,ladoB));
  double operacion_beta = ((pow(longitud_brazo,2)) - (pow(longitud_antebrazo,2)) + (pow(hipotenusa,2)))/(2*longitud_brazo*hipotenusa);
  beta = abs(acos(operacion_beta));
  angulo_brazo = ((alfa*360)/(2*PI))+((beta*360)/(2*PI));
  double operacion_gamma = ((pow(longitud_brazo,2)) + (pow(longitud_antebrazo,2)) - (pow(hipotenusa,2)))/(2*longitud_brazo*longitud_antebrazo);
  gamma = ((acos(operacion_gamma))*360)/(2*PI);
  angulo_antebrazo = gamma;
  angulo_muneca = 180+(cabeceo- angulo_brazo - angulo_antebrazo);

  mapeo_servo1(giro);
  mapeo_servo4(angulo_muneca+20);
  if (angulo_antebrazo <=38){
    mapeo_servo3(38);
    mapeo_servo2(angulo_brazo-(38-angulo_antebrazo)+2);
  }else{
    mapeo_servo3(angulo_antebrazo);
    mapeo_servo2(angulo_brazo);
  }
}
