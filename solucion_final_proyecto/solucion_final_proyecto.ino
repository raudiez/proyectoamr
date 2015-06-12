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
const int LDR1 = 3, LDR2 = 4, LDR3 = 5; //pines de las LDRs
double umbral = 2.5; //valor de comprobación de LDRs
double lecturaLDR1 = 0, lecturaLDR2 = 0, lecturaLDR3 = 0; //variables para almacenar los valores de las LDRs

//motor paso a paso
const int period = 4; // periodo del motor. 4 ms cada paso. A menos periodo irá mas rápido y viceversa

//CNYs
int lecturaCNY1 = 0, lecturaCNY2 = 0, umbralCNY = 500; //variables para almacenar los valores de los CNYs y establecer el umbral de lectura

//Contadores para las caras de los cubos
int caras_analizadas = 0, negras = 0; //variables para llevar la cuenta de las caras que se han procesado y del número de caras negras

//Comunicación
double x = 0.0, y = 0.0, angulo = 0.0; //variables para la comunicación. Coordenadas x e y para el movimiento del brazo y el ángulo de giro de la muñeca al coger los cubos
boolean varX = false, varY = false, varAnguloDecimal = false, colorCaraSuperior = false; //variables para el control de la comunicación (comprobar que se haya realizado bien)
String auxiliar = ""; //cadena en la que se almacena lo recibido por la Raspberry
int colorSuperior = 0; //variable que controla el color de la cara superior de los cubos detectada por la cámara
char letra; //variable utilizada para los signos de puntuación que se le pasa al Arduino (la Raspberry manda la comunicación con la información separada por comas y acabada por ;)

//Servomotores
Servo myservo1, myservo2, myservo3, myservo4, myservo5, myservo6; //declaración de objetos para los 6 servomotores del brazo

//Variables para el cálculo de la cinemática inversa
double cabeceo = 90 - 20.8456;// ángulo de cabeceo en grados
double longitud_muneca = 80.5, longitud_brazo = 90, longitud_antebrazo = 90, altura_hombro = 64.23; //longitudes de las articulaciones del brazo para la cinemática inversa
double alfa, beta, gamma; //ángulos de cálculos
double angulo_brazo, angulo_antebrazo, angulo_muneca; //ángulos de las articulaciones más importantes
double x_cuadrado, y_cuadrado, giro, modulo, x_prima, y_prima, afx, afy, ladoA, ladoB, hipotenusa, operacion_beta, operacion_gamma; //variables para los cálculos de la cinemática inversa

int estado_pinza = 0; // el 0 está abierto y el 1 estará la pinza cerrada

void setup() {
  Serial.begin(9600); //Se establece la velocidad del puerto serie

  //Pines de los LEDs como salidas y se inicializan a LOW para que no de problemas

  pinMode(pinLED1,OUTPUT);
  digitalWrite(pinLED1,LOW);
  pinMode(pinLED2, OUTPUT);
  digitalWrite(pinLED2,LOW);
  pinMode(pinLED3, OUTPUT);
  digitalWrite(pinLED3,LOW);

  //Pines del motor como salida

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

  calibrar_pinza(); //Se calibra la pinza al principio abriéndola y cerrándola para que quede en la posición correcta
  reposo(); //El brazo al principio siempre va a estar en la posición de reposo
  delay(1000);
}

void loop() {
// Se inicia la comunicación y se convierte el valor leído a double
// (o float en este caso, puesto que no existe función toDouble() de
// String), para después pasarlo a la función trabajo_brazo

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
    String lecturaSerie = Serial.readString(); //Se lee la cadena que se le manda desde la Raspberry al Arduino
    auxiliar = ""; //Se ponen las banderas a falso para indicar de que no se ha recibido nada
    colorCaraSuperior = false;
    varAnguloDecimal = false;
    varX = false;
    varY = false;

    for (int i = 0; i < lecturaSerie.length();i++){ //Se recorre la cadena que manda la Raspberry
      if(!colorCaraSuperior){
        letra = lecturaSerie.charAt(i); //En cada comprobación se lee caracter a caracter
        if(letra !=','){ //Se comprueba que el caracter no es una coma
          auxiliar += letra; //Se añade a la cadena auxiliar
        }else{
          colorCaraSuperior = true; //Si el caracter es una coma, se pone la bandera a verdadero
          colorSuperior = auxiliar.toInt(); //Se convierte el valor leido a entero (o a decimal, en el caso de las coordenas x e y)
          auxiliar = ""; //Y se vuelve a poner la cadena auxiliar como vacía para que no de problemas. Se hace lo mismo para los demás datos recibidos
        }
      }else if (!varAnguloDecimal){
        letra = lecturaSerie.charAt(i); //En cada comprobación se lee caracter a caracter
        if(letra !=','){ //Se comprueba que el caracter no es una coma
          auxiliar += letra; //Se añade a la cadena auxiliar
        }else{
          varAnguloDecimal = true; //Si el caracter es una coma, se pone la bandera a verdadero
          angulo = auxiliar.toFloat();
          auxiliar = "";
        }
      }else if(!varX){
        letra = lecturaSerie.charAt(i); //En cada comprobación se lee caracter a caracter
        if(letra !=','){ //Se comprueba que el caracter no es una coma
          auxiliar += letra; //Se añade a la cadena auxiliar
        }else{
          varX = true; //Si el caracter es una coma, se pone la bandera a verdadero
          x = auxiliar.toFloat();
          auxiliar = "";
        }
      }else if (!varY){
        letra = lecturaSerie.charAt(i); //En cada comprobación se lee caracter a caracter
        if(letra !=';'){ //Se comprueba que el caracter no es un punto y coma (se comprueba con la ; porque la raspberry cuando manda la cadena finaliza con un;)
          auxiliar += letra; //Se añade a la cadena auxiliar
        }else{
          varY = true; //Si el caracter es uun punto y coma se pone la bandera a verdadero
          y = auxiliar.toFloat();
          auxiliar = "";
        }
      }//if-else
    }//for
    trabajo_brazo(); //Se llama a la función donde se realiza todo lo del brazo y los sensores (así como el movimiento del motor)
  }//if-serial.available*/
  comprueba_LDRs(); //Se leen valores de las LDRs para poder encender o apagar los LEDs
}//loop

/*
  Esta función realiza el movimiento del brazo para llegar hacia el cubo determinado, cogerlo, llevarlo a la plataforma,
  girar el motor, y en función del color de las caras del cubo, llevarlo a su cubeta correspondiente
*/

void trabajo_brazo(){
  mover_brazo(x,y,-4.0,angulo); //Se le manda la x e y recibidas desde la Raspberry, además del ángulo de la muñeca
  delay(500);
  cerrar_pinza(); //se cierra la pinza para coger el cubo
  delay(500);
  subir_brazo(); //se le sube la altura del brazo para que no choque con el suelo y se manda el brazo al reposo
  plataforma2(); //se manda el cubo a la plataforma giratoria
  delay(500);
  abrir_pinza(); //se suelta el cubo
  delay(500);
  subir_brazo(); //se sube el brazo y se manda al reposo
  for(int i = 0; i<=3;i++){ //se itera 4 veces para leer las caras del cubo que está en la plataforma (la cara superior la obtiene la cámara y la inferior el CNY de abajo de la plataforma)
    lecturaCNY1 = lectura_CNY(CNY_Pin1); //se lee el CNY de la plataforma
    if(lecturaCNY1 > umbralCNY){ //si el valor leído supera el umbral es que se ha detectado una cara negra
      negras++; //se aumenta el contador de caras negras
    }
    caras_analizadas++; //se aumenta el contador de caras analaizadas
    mover_motor(); //se mueve el motor en el sentido de las agujas del reloj
    stopMotor(); //y se frena para que le de tiempo a leer cada cara
    delay(50);
  }
  delay(30);
  vuelve_motor(); //se gira el motor en el sentido contrario a las agujas del reloj para que no se enrolle el cable del motor
  lecturaCNY2 = lectura_CNY(CNY_Pin2); //se lee la cara inferior del cubo
  if(lecturaCNY2 > umbralCNY){ //si el valor es mayor que el umbral
    negras++; //aumenta contador de negras
  }
  caras_analizadas++; //aumenta contador de caras analizadas
  if(colorSuperior < 100){ //si el color de la cara superior detectado por la camara es menor al umbral
    negras++; //aumenta contador de negras
  }
  caras_analizadas++; //aumenta contador de caras analizadas
  plataforma(); //manda el brazo hacia la plataforma
  delay(500);
  cerrar_pinza(); //coge el cubo
  delay(500);
  subir_brazo(); //sube la altura del brazo y lo manda al reposo
  if(negras == 0){ //si el cubo no tiene caras negras
    cubeta1(); //lo manda a la cubeta de la izquierda
  }else if(negras == 1){ //si el cubo tiene 1 cara negra
    cubeta2(); //lo manda a la cubeta del centro
  }else if(negras == 2){//si el cubo tiene 2 caras negras
    cubeta3(); //lo manda a la cubeta de la derecha
  } //*/

  Serial.write("terminado\n"); //se le manda la cadena "terminado" a la Raspberry para indicarle de que ha depositado uno de los cubos
}

/*
  Función que mueve el motor en el sentido inverso a las agujas del reloj

*/

void vuelve_motor(){
  mover_motor_inv(); //Se llama a esta función cuatro veces pues es la encargada de poner los pines del motor como LOW y para que desenrolle bien el cable del motor
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

/*
  Función para mover el motor de la plataforma. Realiza 128 iteraciones para que de un cuarto de vuelta
*/

void mover_motor(){
  for (int i = 0; i < 128; i++){
    step1();
    step2();
    step3();
    step4();
  }
}

/*
  Función para mover el motor de la plataforma en el sentido contrario. Realiza 128 iteraciones para que de un cuarto de vuelta
*/

void mover_motor_inv(){
  for (int i = 0; i < 128; i++){
    step4();
    step3();
    step2();
    step1();
  }
}

/*
  Función para comprobar los valores de los LDRs y en función  de si supera el umbral establecido, eneceder un determinado LED y sino apagarlo
  de modo que si hay cubo en la cubeta, se enciende y sino se apaga el LED
*/

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

/*
  Función que lee el valor de una LDR, se pasa el valor a voltaje y se devuelve
*/

double lectura_LDR(const int pin){
  double valorLDR = analogRead(pin);
  valorLDR *= 0.0049;
  return valorLDR;
}

/*
  Función que lee el valor de un CNY y lo deuelve
*/

int lectura_CNY(const int pin){
  int Valor_CNY = analogRead(pin);
  return Valor_CNY;
}

/*
    Función para mover el brazo hacia un cubo, dándole las coordenadas x e y, y además el ángulo de giro de la muñeca
*/

void mover_brazo(double x, double y, double z, double angulo_pinza){
  if(x > 0){ x+=30.0; y-=23;} //si la x está en el primer cuadrante, se ajustan las posiciones de x e y
  calcula_angulos(x,y+50.0,75.0+z,angulo_pinza); //se calcula la cinemática inversa teniendo en cuenta las distancias en nuestro sistema de referencia
  delay(1000);
}


/*
  Función para mandar el cubo a la primera cubeta (ninguna cara negra).
  Se establecen los ángulos correspondientes del brazo para llegar hacia la cubeta, se comprueba continuamente las LDRs, se deja el cubo en la cubeta y se manda
  el brazo hacia el reposo
*/

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

/*
  Función para mandar el cubo a la segunda cubeta (una cara negra).
  Se establecen los ángulos correspondientes del brazo para llegar hacia la cubeta, se comprueba continuamente las LDRs, se deja el cubo en la cubeta y se manda
  el brazo hacia el reposo
*/

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

/*
  Función para mandar el cubo a la tercera cubeta (dos caras negras).
  Se establecen los ángulos correspondientes del brazo para llegar hacia la cubeta, se comprueba continuamente las LDRs, se deja el cubo en la cubeta y se manda
  el brazo hacia el reposo
*/

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

/*

  Función para mandar a la plataforma cuando se han analizado las caras en la plataforma. Se ajustan los ángulos correspondientes.

*/

void plataforma(){
  myservo5.write(161);
  myservo1.write(180);
  mapeo_servo4(120);
  mapeo_servo2(97);
  mapeo_servo3(49);
}

/*
  Función para mandar a la plataforma la primera vez que coge el cubo para soltarlo. Se ajustan los ángulos correspondientes.
*/

void plataforma2(){
  myservo5.write(161);
  myservo1.write(180);
  mapeo_servo4(116);
  mapeo_servo2(102);
  mapeo_servo3(53);
}

/*
  Función para subir el brazo cuando ha soltado el cubo y mover el brazo hacia el reposo. Se gira simplemente el ángulo del servo del brazo (lo que es el siguiente del de la base)
*/

void subir_brazo(){
  mapeo_servo2(145);
  reposo();
}

/*
  Función que realiza los mapeos de los ángulos correspondientes a la base
*/

void mapeo_servo1(double angulo){
  double angulonuevo = map(angulo,0,180,20,190);
  if(angulonuevo > 180){ //Si el ángulo fuera mayor de 180, la base no puede girar más (se pone esto por si hay conflicto con el valor de los ángulos en las ecuaciones de cinemática)
    angulonuevo = 180; //Entonces se establece el ángulo como tope
  }
  double anguloactual = myservo1.read(); //Lee el ángulo que tiene el servo
  if(angulonuevo >= anguloactual){ //Esto se hace para que el movimiento vaya un poco más lento y más fluido
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

/*
  Función que realiza los mapeos de los ángulos correspondientes al hombro
*/

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

/*
  Función que realiza los mapeos de los ángulos correspondientes al codo
*/

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

/*
  Función que realiza los mapeos de los ángulos correspondientes a la muñeca vertical
*/

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

/*
  Función que realiza los mapeos de los ángulos correspondientes a la mano
*/

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

/*
  Función que abre la pinza teniendo en cuenta el estado en el que se encuentra
*/

void abrir_pinza(){
  if(estado_pinza == 1){
  myservo6.write(100);
  estado_pinza = 0;
 }
}

/*
  Función que cierra la pinza teniendo en cuenta el estado en el que se encuentra
*/

void cerrar_pinza(){
  if(estado_pinza == 0){
  myservo6.write(20);
  estado_pinza = 1;
 }
}

/*
  Función que manda el brazo al reposo poniendo los ángulos correspondientes
*/

void reposo(){
  mapeo_servo1(90);
  mapeo_servo2(145);
  mapeo_servo3(60);
  mapeo_servo4(60);
  mapeo_servo5(90);
}

/*
  Función para calibrar la pinza al principio, la abre y la cierra para que deje la pinza bien
*/

void calibrar_pinza(){
  //Todo esto es para calibrar la pinza
  myservo6.write(180);
  delay(1000);
  myservo6.write(0);
  delay(1000);
  myservo6.write(100);
  delay(1000);
}

/*
  Función que realiza ela cinemática inversa del brazo para que según las coordenadas (x,y) y el giro de la muñeca, mande el brazo a la posición deseada. Se le pasa también la altura
  con respecto al suelo de la mano (la z)
*/

void calcula_angulos(double x, double y, double z, double angulo_pinza){
  x_cuadrado = pow(x,2);
  y_cuadrado = pow(y,2);

  giro = asin(abs(x)/(sqrt(x_cuadrado+y_cuadrado))); //cálculo para saber el ángulo de giro de la base para que llegue hacia el cubo
  giro = (giro*360)/(2*PI); //como el ángulo devuelto al usar una función de la libería math, está en radianes, hay que pasarlo a grados
  if(x > 0){ //si el sentido de giro es positivo o negativo
    giro = giro + 90; //se ajusta la posición de la base (cambiando el sentido de giro)
  }else {
     giro =  90 - giro; //se ajusta la posición de la base (cambiando el sentido de giro)
  }

  modulo = (sqrt(x_cuadrado+y_cuadrado))-18.3; //se realiza el módulo de x e y, y además se le resta 18.3 que es la distancia de separación entre el hombro y el codo
  y_prima = z; //se establece y' como la altura que va a tener la mano (con respecto al suelo)
  afx = abs(cos(cabeceo))*longitud_muneca; //se calcula el incremento de x

  x_prima = modulo; //se establece x' como el módulo
  ladoB = x_prima - afx;
  afy = abs(sin(cabeceo))*longitud_muneca; //se calcula el incremento de y
  ladoA = y_prima + afy - altura_hombro;

  hipotenusa = sqrt(pow(ladoA,2)+pow(ladoB,2)); //se calcula la hipotenusa del ladoA y del ladoB
  alfa = abs(atan2(ladoA,ladoB)); //se realiza el arcotangente2 del ladoA y el ladoB. El arcotangente2 es la operación de hacer el arcotangente pero con dos argumentos
  operacion_beta = ((pow(longitud_brazo,2)) - (pow(longitud_antebrazo,2)) + (pow(hipotenusa,2)))/(2*longitud_brazo*hipotenusa); //teorema del coseno
  beta = abs(acos(operacion_beta)); //se calcula el ángulo beta

  angulo_brazo = ((alfa*360)/(2*PI))+((beta*360)/(2*PI)); //se pasa el angulo del hombro a grados
  operacion_gamma = ((pow(longitud_brazo,2)) + (pow(longitud_antebrazo,2)) - (pow(hipotenusa,2)))/(2*longitud_brazo*longitud_antebrazo); //teorema del coseno
  gamma = ((acos(operacion_gamma))*360)/(2*PI); //se calcula el ángulo gamma y se pasa a grados
  angulo_antebrazo = gamma; //se establece el ángulo del antebrazo (codo) como gamma
  angulo_muneca = 180+(cabeceo- angulo_brazo - angulo_antebrazo); //se calcula el ángulo de la muñeca horizontal

  mapeo_servo1(giro); //gira la base con el ángulo antes calculado
  //Para que siga estando la pinza recta respecto al rectángulo de referencia:
  double giro_pinza = giro - 90;
  //Para girar lo que esté girado el cubo respecto a su eje:
  giro_pinza = giro_pinza - angulo_pinza;
  mapeo_servo5(90+giro_pinza);

  mapeo_servo4(angulo_muneca+20); //se gira el ángulo de la muñeca horizontal con el ángulo antes calculado y se ajusta bien la posición
  if (angulo_antebrazo <=38){ //para que ajustar bien algunas posiciones del brazo
    mapeo_servo3(38);
    mapeo_servo2(angulo_brazo-(38-angulo_antebrazo)+3);
  }else{
    mapeo_servo3(angulo_antebrazo+1);
    mapeo_servo2(angulo_brazo+1);
  }
}

/*
  Función que realiza el primer paso del motor (estableciendo la secuencia correcta) y esperando el tiempo establecido en el periodo
*/

void step1(){
 digitalWrite(pinMotor1,HIGH);
 digitalWrite(pinMotor2,HIGH);
 digitalWrite(pinMotor3,LOW);
 digitalWrite(pinMotor4,LOW);
 delay(period);
}

/*
  Función que realiza el segundo paso del motor (estableciendo la secuencia correcta) y esperando el tiempo establecido en el periodo
*/

void step2(){
 digitalWrite(pinMotor1,LOW);
 digitalWrite(pinMotor2,HIGH);
 digitalWrite(pinMotor3,HIGH);
 digitalWrite(pinMotor4,LOW);
 delay(period);
}

/*
  Función que realiza el tercer paso del motor (estableciendo la secuencia correcta) y esperando el tiempo establecido en el periodo
*/

void step3(){
 digitalWrite(pinMotor1,LOW);
 digitalWrite(pinMotor2,LOW);
 digitalWrite(pinMotor3,HIGH);
 digitalWrite(pinMotor4,HIGH);
 delay(period);
}

/*
  Función que realiza el cuarto paso del motor (estableciendo la secuencia correcta) y esperando el tiempo establecido en el periodo
*/

void step4(){
 digitalWrite(pinMotor1,HIGH);
 digitalWrite(pinMotor2,LOW);
 digitalWrite(pinMotor3,LOW);
 digitalWrite(pinMotor4,HIGH);
 delay(period);
}

/*
  Función que realiza que para el motor (estableciendo la secuencia correcta)
*/

void stopMotor() {
 digitalWrite(pinMotor1,LOW);
 digitalWrite(pinMotor2,LOW);
 digitalWrite(pinMotor3,LOW);
 digitalWrite(pinMotor4,LOW);
}
