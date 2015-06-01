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

void setup(){

  Serial.begin(9600);

  myservo1.attach(7); //servo del soporte giratorio! base
  myservo2.attach(8); //hombro
  myservo3.attach(9); //codo...sube o baja el brazo
  myservo4.attach(10); ///muñeca vertical
  myservo5.attach(11); //muñeca horizontal - mano
  myservo6.attach(12); //pinza

  //reposo();
  delay(1000);
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

void loop(){
  /*double x=10.0;
  double y=100.0;
  double z=10.0;*/

  reposo();
  delay(1000);
  abrir_pinza();
  delay(2000);
  mover_hacia_cubo(-12.8,107.6,-5.0);
  delay(2000);
/*
  //va a por el cubo.
  if(x<0){ x-=15.0; y+=5;}
  else if(x > 0){ x+=50.0; z+=10; y-=23;}
  calcula_angulos(x,y+50.0,75.0+z);
  delay(1000);*/
  
  cerrar_pinza();
  delay(1000);
  reposo();

  //subir_brazo(10.0,100.0,60.0);
  //soltar_cubo(120.0,120.0,10.0);
  //subir_brazo_2(120.0,120.0,60.0);
/*
  //sube
  x=10.0;
  y=100.0;
  z=60.0;
  if(x<0){ x-=15.0; y+=5;}
  else if(x > 0){ x+=50.0; z+=10; y-=23;}
  calcula_angulos(x,y+50.0,75.0+z);
  delay(2000);*/
/*
  //va a soltar el cubo.
  reposo();
  delay(1000);
  x=100.0;
  y=120.0;
  z=10.0;
  if(x<0){ x-=15.0; y+=5;}
  else if(x > 0){ x+=50.0; z+=10; y-=23;}
  calcula_angulos(x,y+50.0,75.0+z);
  abrir_pinza();
  delay(2000);*/
/*
  //sube
  x=100.0;
  y=120.0;
  z=60.0;
  if(x<0){ x-=15.0; y+=5;}
  else if(x > 0){ x+=50.0; z+=10; y-=23;}
  calcula_angulos(x,y+50.0,75.0+z);
  delay(2000);*/
}


void mover_hacia_cubo(double x, double y, double z){

  //if(x<0){ x-=15.0; y+=5;}
  if(x > 0){ x+=50.0; z+=10; y-=23;}
  calcula_angulos(x,y+50.0,75.0+z);
  delay(1000);

}

void subir_brazo(double x, double y, double z){

  if(x<0){ x-=15.0; y+=5;}
  else if(x > 0){ x+=50.0; z+=10; y-=23;}
  calcula_angulos(x,y+50.0,75.0+z);
  delay(2000);
  
}

void soltar_cubo(double x, double y, double z){
  
  reposo();
  delay(1000);
  if(x<0){ x-=15.0; y+=5;}
  else if(x > 0){ x+=50.0; z+=10; y-=23;}
  calcula_angulos(x,y+50.0,75.0+z);
  abrir_pinza();
  delay(2000);
  
}

void subir_brazo_2(double x, double y, double z){

  if(x<0){ x-=15.0; y+=5;}
  else if(x > 0){ x+=50.0; z+=10; y-=23;}
  calcula_angulos(x,y+50.0,75.0+z);
  delay(2000);

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
  myservo6.write(30);
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

void calcula_angulos(double x, double y, double z){

  double x_cuadrado = pow(x,2);
  double y_cuadrado = pow(y,2);

 double giro = asin(abs(x)/(sqrt(x_cuadrado+y_cuadrado)));
 Serial.print("Angulo giro radianes: ");
 Serial.println(giro);
  giro = (giro*360)/(2*PI);
    if(x > 0){
    giro = giro + 90;
    mapeo_servo1(giro);
  }
  else {
     giro =  90 - giro;
     mapeo_servo1(giro);
  }
  delay(2000);
  Serial.print("Angulo giro grados: ");
  Serial.println(giro);

  double modulo = (sqrt(x_cuadrado+y_cuadrado))-18.3;
   Serial.print("Modulo: ");
   Serial.println(modulo);


  double y_prima = z;

  double afx = abs(cos(cabeceo))*longitud_muneca;
  Serial.print("AFX: ");
  Serial.println(afx);

  double x_prima = modulo;
  double ladoB = x_prima - afx;
  Serial.print("LadoB: ");
  Serial.println(ladoB);

  double afy = abs(sin(cabeceo))*longitud_muneca;
  Serial.print("AFY: ");
  Serial.println(afy);

  double ladoA = y_prima + afy - altura_hombro;
  Serial.print("LadoA: ");
  Serial.println(ladoA);

  double hipotenusa = sqrt(pow(ladoA,2)+pow(ladoB,2));
  Serial.print("Hipotenusa: ");
  Serial.println(hipotenusa);

  alfa = abs(atan2(ladoA,ladoB));
  Serial.print("Alfa en radianes: ");
  Serial.println(alfa);

  double operacion_beta = ((pow(longitud_brazo,2)) - (pow(longitud_antebrazo,2)) + (pow(hipotenusa,2)))/(2*longitud_brazo*hipotenusa);
    Serial.print("operacion_beta: ");
  Serial.println(operacion_beta);

  beta = abs(acos(operacion_beta));
  Serial.print("Beta en radianes: ");
  Serial.println(beta);

  //angulo_brazo = alfa+beta;
  angulo_brazo = ((alfa*360)/(2*PI))+((beta*360)/(2*PI));
  Serial.print("Angulo hombro (brazo) en grados: ");
  Serial.println(angulo_brazo);


  double operacion_gamma = ((pow(longitud_brazo,2)) + (pow(longitud_antebrazo,2)) - (pow(hipotenusa,2)))/(2*longitud_brazo*longitud_antebrazo);
  gamma = ((acos(operacion_gamma))*360)/(2*PI);

  Serial.print("Gamma en grados: ");
  Serial.println(gamma);

  angulo_antebrazo = gamma;

  Serial.print("Angulo antebrazo (codo) en grados: ");
  Serial.println(angulo_antebrazo);

  angulo_muneca = 180+(cabeceo- angulo_brazo - angulo_antebrazo);
  Serial.print("Angulo muneca en grados: ");
  Serial.println(angulo_muneca);

  mapeo_servo4(angulo_muneca+30);
  mapeo_servo3(angulo_antebrazo);
  mapeo_servo2(angulo_brazo);

}


