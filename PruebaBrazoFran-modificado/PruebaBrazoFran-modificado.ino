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
  reposo();
  delay(1000);
  abrir_pinza();
  delay(2000);
  mover_hacia_cubo(-3.5,105.5,10.0);
  delay(2000);
  cerrar_pinza();
  delay(1000);
  reposo();
}


void mover_hacia_cubo(double x, double y, double z){
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

void mapeo_servo1(double angulo){
  double angulonuevo = map(angulo,0,180,20,190);
  if(angulonuevo > 180){
    angulonuevo = 180;
  }
  myservo1.write(angulonuevo);
}
void mapeo_servo2(double angulo){
  double angulonuevo = map(angulo,0,180,-10,170);
  myservo2.write(angulonuevo);
}
void mapeo_servo3(double angulo){
  double angulonuevo = map(angulo,0,180,-21,159);
  myservo3.write(angulonuevo);
}
void mapeo_servo4(double angulo){
  double angulonuevo = map(angulo,0,180,180,0);  //NO necesita mapeo
  myservo4.write(angulonuevo);
}
void mapeo_servo5(double angulo){
  double angulonuevo = map(angulo,0,180,16,180);
  myservo5.write(angulonuevo);
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
  mapeo_servo5(90);
  delay(1000);
  mueve_brazo(90,145,60,60);
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

  mueve_brazo(giro,angulo_brazo,angulo_antebrazo,angulo_muneca+30);
}

void mueve_brazo(double giro,double angulo_brazo,double angulo_antebrazo,double angulo_muneca){
  double anguloactual1 = myservo1.read();
  double anguloactual2 = myservo2.read();
  double anguloactual3 = myservo3.read();
  double anguloactual4 = myservo4.read();
  double i1=anguloactual1, i2=anguloactual2, i3=anguloactual3, i4=anguloactual4;
  boolean i1end=false, i2end=false, i3end=false, i4end=false;

  while(!i1end || !i2end || !i3end || !i4end){
    if(giro >= anguloactual1 && !i1end){
      i1++;
    }else if(giro < anguloactual1 && !i1end){i1--;}
    if(i1 == giro) i1end = true;

    if(angulo_brazo >= anguloactual2 && !i2end){
      i2++;
    }else if(angulo_brazo < anguloactual2 && !i2end){i2--;}
    if(i2 == angulo_brazo) i2end = true;

    if(angulo_antebrazo >= anguloactual3 && !i3end){
      i3++;
    }else if(angulo_antebrazo < anguloactual3 && !i3end){i3--;}
    if(i3 == angulo_antebrazo) i3end = true;

    if(angulo_muneca >= anguloactual4 && !i4end){
      i4++;
    }else if(angulo_muneca < anguloactual4 && !i4end){i4--;}
    if(i4 == angulo_muneca) i4end = true;

    mapeo_servo1(i1);
    mapeo_servo2(i2);
    mapeo_servo3(i3);
    mapeo_servo4(i4);
    delay(60);
  }
}