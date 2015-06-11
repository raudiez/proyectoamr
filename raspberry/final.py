#!/usr/bin/env python
# -*- coding: utf-8 -*-

#### TODO: - Comunicación con Arduino mediante serial.
####        - Pruebas del programa.

import numpy as np
import cv2
from math import atan, degrees , sqrt, pow
import serial
#import RPi.GPIO as GPIO

########## Variables globales #################
arduino = serial.Serial('/dev/ttyACM0', 9600)
workzone = []
workzone2 = []
square2 = []
topside_center_workzone = [0,0]
bottomside_center_worzone = [0,0]
square = [] # Cubos encontrados, respecto a (0,0) de img.
sorted_cube = [] # Cubos con vértices ordenados por menor Y (para getWristAngle).
center = [0,0]
cont = 0
new_center = [0,0]
sideWristAngle = [0,0]
raspiData = ""

# Variables finales a enviar a Arduino (estas variables se reinicializan para
# cada nuevo cubo encontrado):
upperSideColor = 0 # Valor del color de la cara superior.
wristAngle = 0 # Ángulo de giro de la muñeca.
xFromArm = 0 # X respecto al brazo robótico.
yFromArm = 0 # Y respercto al brazo robótico.

# Datos conocidos:
yDistanceFromArm = 124.8 # Distancia desde el nuevo SR (0,0) al (0,0) del brazo, en mm.

# Colores para pintar:
RED=(0,0,255)
BLUE=(255,0,0)
CYAN=(255,150,0)
GREEN=(0,255,0)
YELLOW=(0,255,255)
ORANGE=(0,150,255)
WHITE=(255,255,255)
BLACK=(0,0,0)

### LED RGB ###
# Mapeo de pines del LED.
blue = 18
green = 27 #Pin 27 en B+, 21 en B.
red = 17

# # Configuración GPIO.
# GPIO.setmode(GPIO.BCM)
# GPIO.setwarnings(False)
# GPIO.setup(red, GPIO.OUT)
# GPIO.setup(green, GPIO.OUT)
# GPIO.setup(blue, GPIO.OUT)

# # Configuración de colores usando PWM por software. Control individual del
# # brillo de cada color.
# RED = GPIO.PWM(red, 100)
# GREEN = GPIO.PWM(green, 100)
# BLUE = GPIO.PWM(blue, 100)
# RED.start(100)
# GREEN.start(100)
# BLUE.start(100)

######## Definición de funciones ##################

def angle_cos(p0, p1, p2):
  d1, d2 = (p0-p1).astype('float'), (p2-p1).astype('float')
  return abs( np.dot(d1, d2) / np.sqrt( np.dot(d1, d1)*np.dot(d2, d2) ) )

# Función que encuentra los cuadrados de las caras superiores de cada cubo,
# obtiene los cuadrados del contorno, y los centros.
def findSquares(img):
  img = cv2.GaussianBlur(img, (5, 5), 0)
  global sorted_cube
  global square
  global center
  global cont
  previous = []
  alpha = 0
  cuboFound = False
  for gray in cv2.split(img):
    if not cuboFound:
      for thrs in xrange(0, 255, 26):
        if thrs == 0:
          bin = cv2.Canny(gray, 0, 50, apertureSize=5)
          bin = cv2.dilate(bin, None)
        else:
          retval, bin = cv2.threshold(gray, thrs, 255, cv2.THRESH_BINARY)
        contours, hierarchy = cv2.findContours(bin, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)
        if not cuboFound:
          for cnt in contours:
            if not cuboFound:
              cnt_len = cv2.arcLength(cnt, True)
              cnt = cv2.approxPolyDP(cnt, 0.02*cnt_len, True)
              if len(cnt) == 4 and cv2.contourArea(cnt) > 1200 and cv2.contourArea(cnt) < 1800 and cv2.isContourConvex(cnt):
                cnt = cnt.reshape(-1, 2)
                encontrado = False
                # Se hace un bucle para buscar dentro de los contornos los más próximos
                # para cada cubo.
                for i in previous:
                  if i[0] == cnt[0][0] or (i[0] <= cnt[0][0]+3 and i[0] >= cnt[0][0]-3) :
                    # Si se encuentra este contorno en los anteriormente analizados,
                    # no se usa.
                    encontrado = True
                    break
                max_cos = np.max([angle_cos( cnt[i], cnt[(i+1) % 4], cnt[(i+2) % 4] ) for i in xrange(4)])
                if max_cos < 0.1 and not encontrado:
                  # Se almacena el contorno del cuadrado con sus vértices, se almacena
                  # como analizado también.
                  square = np.copy(cnt)
                  previous.append(cnt[0])
                  previous.append(cnt[1])
                  previous.append(cnt[2])
                  previous.append(cnt[3])
                  # Se ordenan los vértices del cubo, para luego usarlos para
                  # los ángulos.
                  cubo = np.copy(cnt)
                  dt = [('col1', cubo.dtype),('col2', cubo.dtype)]
                  aux = cubo.ravel().view(dt)
                  aux.sort(order=['col2','col1'])
                  sorted_cube = np.copy(cubo)
                  # Se obtienen el centro del cuadrado, y se saca el color de la cara.
                  center = [(cnt[0][0]+cnt[2][0])/2,(cnt[0][1]+cnt[2][1])/2]
                  cont+=1
                  print "Encontrado cubo numero",cont
                  cuboFound = True

# Se encuentra un rectángulo de area mayor que los cubos, que los contiene y
# vale de referencia para ubicación. Hace prácticamente lo mismo que la función
# anterior para los cubos. Obtiene además puntos de referencia a mitad del
# rectángulo de referencia para construir un eje de coordenadas.
def findWorkzone(img):
  img = cv2.GaussianBlur(img, (5, 5), 0)
  global workzone
  previous = []
  for gray in cv2.split(img):
    for thrs in xrange(0, 255, 26):
      if thrs == 0:
        bin = cv2.Canny(gray, 0, 50, apertureSize=5)
        bin = cv2.dilate(bin, None)
      else:
        retval, bin = cv2.threshold(gray, thrs, 255, cv2.THRESH_BINARY)
      contours, hierarchy = cv2.findContours(bin, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)
      for cnt in contours:
        cnt_len = cv2.arcLength(cnt, True)
        cnt = cv2.approxPolyDP(cnt, 0.02*cnt_len, True)
        if len(cnt) == 4 and cv2.contourArea(cnt) > 42000 and cv2.contourArea(cnt) < 49000 and cv2.isContourConvex(cnt):
          cnt = cnt.reshape(-1, 2)
          encontrado = False
          for i in previous:
            if i[0] == cnt[0][0] or (i[0] <= cnt[0][0]+3 and i[0] >= cnt[0][0]-3) :
              encontrado = True
              break
          max_cos = np.max([angle_cos( cnt[i], cnt[(i+1) % 4], cnt[(i+2) % 4] ) for i in xrange(4)])
          if max_cos < 0.1 and not encontrado:
            workzone = np.copy(cnt)
            previous.append(cnt[0])
            getReference()

# Función que ordena los puntos del rectángulo de referencia y obtiene el
# lado superior y el inferior.
def getReference():
  top = [0,0]
  bottom = [0,0]
  topi=bottomi=i=0
  while i < 4:
    # Se compara respecto a la mitad de la imagen (240 px).
    if workzone[i][1] < 240:
      top[topi]=[workzone[i][0],workzone[i][1]]
      topi+=1
    else:
      bottom[bottomi]=[workzone[i][0],workzone[i][1]]
      bottomi+=1
    i+=1
  top.sort
  bottom.sort
  global topside_center_workzone
  topside_center_workzone = [(top[0][0]+top[1][0])/2,(top[0][1]+top[1][1])/2]
  global bottomside_center_worzone
  bottomside_center_worzone = [(bottom[0][0]+bottom[1][0])/2,(bottom[0][1]+bottom[1][1])/2]

# Función que obtiene nuevas coordenadas para los centros de los cubos y los lados
# de los cubos que serviran para determinar el giro de la pinza del brazo robótico.
def changeSR():
  # Se modifican las coordenadas respecto al nuevo SR:
  # El punto medio del lado inferior del rectángulo de referencia.
  global new_center
  global sideWristAngle
  # Se almacena el lado importante para el cálculo de ángulos de giro.
  # El resto de lados se ignora.
  a = [sorted_cube[0][0]-bottomside_center_worzone[0],bottomside_center_worzone[1]-sorted_cube[0][1]]
  b = [sorted_cube[1][0]-bottomside_center_worzone[0],bottomside_center_worzone[1]-sorted_cube[1][1]]
  sideWristAngle = [a,b]
  # Ahora se cambian las coordenadas de los centros respecto al nuevo SR.
  new_center = [center[0]-bottomside_center_worzone[0],bottomside_center_worzone[1]-center[1]]

# Función que calcula el ángulo de giro para el brazo robótico (giro respecto al
# eje del nuevo SR).
def getWristAngle():
  global wristAngle
  x = [sideWristAngle[0][0],sideWristAngle[1][1]]
  b = sqrt(pow((x[0]-sideWristAngle[0][0]),2)+pow((x[1]-sideWristAngle[0][1]),2))
  c = sqrt(pow((x[0]-sideWristAngle[1][0]),2)+pow((x[1]-sideWristAngle[1][1]),2))
  alpha = degrees(atan(b/c))
  if sideWristAngle[0][0] < sideWristAngle[1][0] and sideWristAngle[0][1] > sideWristAngle[1][1]:
    alpha = -degrees(atan(b/c))
  # Hasta aquí, cálculo del ángulo respecto a SR. Ahora se cambia respecto
  # al 0,0 del brazo robótico.
  alpha = -alpha
  print "Tiene un giro sobre si mismo de",alpha,"grados"
  wristAngle = alpha

# Función que obtiene el color de la cara superior de un cubo.
def getColor(img):
  global upperSideColor
  global center
  imggray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
  upperSideColor = imggray[center[1]][center[0]] # Color de cara superior
  if upperSideColor >= 100 :
    color = "blanco"
  else :
    color = "negro"
  print "Su color es",color

# Función que convierte a milímetros un valor en px, según las medidas de
# la zona de espera de los cubos (medidas utilizadas como referencia).
def convertPixelsToMillimetres(pixels):
  # Para convertir de px a mm, se utiliza una regla de 3. Sabiendo que el
  # rectángulo de la zona de espera mide 130x60mm (ancho x alto), si en la
  # captura, el alto es de X px, entonces 1mm = 60 px / X.
  X = bottomside_center_worzone[1] - topside_center_workzone[1]
  one_mm_in_px = 60.0/X
  return pixels * one_mm_in_px

# Función que obtiene la coordenada X respecto al brazo robótico.
def getXFromArm():
  aux = -new_center[0]
  return convertPixelsToMillimetres(aux)

# Función que obtiene la coordenada Y respecto al brazo robótico.
def getYFromArm():
  global yDistanceFromArm
  return yDistanceFromArm - convertPixelsToMillimetres(new_center[1])

# Función que formatea la cadena a enviar a Arduino, añadiendo los valores
# a enviar.
def getDataString():
  global raspiData
  raspiData = raspiData + np.array_str(upperSideColor)
  raspiData = raspiData + ","
  raspiData = raspiData + str(round(wristAngle))
  raspiData = raspiData + ","
  raspiData = raspiData + str(round(xFromArm))
  raspiData = raspiData + ","
  raspiData = raspiData + str(round(yFromArm))
  raspiData = raspiData + ";"

# Función que obtiene la información a enviar a Arduino y la envía.
def sendDataToArduino():
  global raspiData
  getDataString()
  arduino.write(raspiData)

# Función que espera hasta que Arduino termine su función y envíe un mensaje
# de finalización de tarea.
def waitForArduino():
  wait = True
  arduinoState = ''
  while wait :
    arduinoState = arduino.readline()
    if arduinoState == 'terminado\n':
      wait = False

# Función que limpia las variables globales que se reutilizan en el
# programa principal.
def clean():
  global square
  global square2
  global sorted_cube
  global center
  global new_center
  global sideWristAngle
  global upperSideColor
  global wristAngle
  global xFromArm
  global yFromArm
  global raspiData
  square = []
  square2 = []
  sorted_cube = []
  center = [0,0]
  new_center = [0,0]
  sideWristAngle = [0,0]
  upperSideColor = 0
  wristAngle = 0
  xFromArm = 0
  yFromArm = 0
  raspiData = ""

# Función que realiza una captura con la cámara, busca los cubos, e imprime
# por pantalla información como ángulos, posición, y los pinta en la captura.
def captureAndFind():
  global cont
  global xFromArm
  global yFromArm
  global workzone
  global workzone2
  global square
  global square2
  global raspiData
  cam = cv2.VideoCapture(0)
  ret, img = cam.read()
  # Si es la primera vez que se ejecuta la función, se busca la zona de trabajo,
  # que servirá como referencia.
  if cont == 0 :
    findWorkzone(img)
    print "El nuevo origen de coordenadas sera [",convertPixelsToMillimetres(bottomside_center_worzone[0]),",",convertPixelsToMillimetres(bottomside_center_worzone[1]),"] mm"
    print "Utilizando ese punto como nuevo SR."
    workzone2.append(workzone)
  findSquares(img)
  getColor(img)
  square2.append(square)
  changeSR()
  getWristAngle()
  xFromArm = getXFromArm()
  yFromArm = getYFromArm()
  print "El cubo se encuentra en la posición [",xFromArm,",",yFromArm,"] mm respecto al brazo"
  sendDataToArduino()
  waitForArduino()
  clean()
  cam.release()

# Set a color by giving R, G, and B values of 0-255.
# def setColor(rgb = []):
#   print rgb
#   # Convert 0-255 range to 0-100.
#   rgb = [(abs(x-255) / 255.0) * 100 for x in rgb]
#   RED.ChangeDutyCycle(rgb[0])
#   GREEN.ChangeDutyCycle(rgb[1])
#   BLUE.ChangeDutyCycle(rgb[2])

if __name__ == '__main__':
  clean()
  while cont <= 2 : # Se itera mientras cont < 2 (2 = num cubos -1)
    # Se realiza una captura con la cámara y se busca un cubo.
    captureAndFind()
    print "#################################################"
  arduino.close()
  #GPIO.cleanup()
