#!/usr/bin/env python
# -*- coding: utf-8 -*-

#### TO DO: - Calcular X e Y respecto al brazo.

import numpy as np
import cv2
from math import atan, degrees , sqrt, pow

########## Variables globales #################
workzone = []
topside_center_workzone = [0,0]
bottomside_center_worzone = [0,0]
square = [] # Cubos encontrados, respecto a (0,0) de img.
sorted_cube = [] # Cubos con vértices ordenados por menor Y (para getWristAngle).
center = [0,0]
cont = 0
new_center = [0,0]
sideWristAngle = [0,0]

# Variables finales a enviar a Arduino (estas variables se reinicializan para
# cada nuevo cubo encontrado):
upperSideColor = 0 # Valor del color de la cara superior.
wristAngle = 0 # Ángulo de giro de la muñeca.
xFromArm = 0 # X respecto al brazo robótico.
yFromArm = 0 # Y respercto al brazo robótico.

# Colores para pintar:
RED=(0,0,255)
BLUE=(255,0,0)
CYAN=(255,150,0)
GREEN=(0,255,0)
YELLOW=(0,255,255)
ORANGE=(0,150,255)
WHITE=(255,255,255)
BLACK=(0,0,0)

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
        if len(cnt) == 4 and cv2.contourArea(cnt) > 40000 and cv2.contourArea(cnt) < 49000 and cv2.isContourConvex(cnt):
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
  print "Tiene un giro sobre si mismo de",alpha,"grados"
  wristAngle = alpha

# Función que obtiene el color de la cara superior de un cubo.
def getColor(img):
  global upperSideColor
  imggray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
  upperSideColor = imggray[center[0]][center[1]] # Color de cara superior
  if upperSideColor >= 100 :
    color = "blanco"
  else :
    color = "negro"
  print "Su color es",color

# Función que limpia las variables globales que se reutilizan en el
# programa principal.
def clean():
  square = []
  sorted_cube = []
  center = [0,0]
  new_center = [0,0]
  sideWristAngle = [0,0]
  upperSideColor = 0
  wristAngle = 0
  xFromArm = 0
  yFromArm = 0

# Función que realiza una captura con la cámara, busca los cubos, e imprime
# por pantalla información como ángulos, posición, y los pinta en la captura.
def captureAndFind():
  global cont
  cam = cv2.VideoCapture(0)
  ret, img = cam.read()
  # Si es la primera vez que se ejecuta la función, se busca la zona de trabajo,
  # que servirá como referencia.
  if cont == 0 :
    findWorkzone(img)
    print "El nuevo origen de coordenadas sera [",bottomside_center_worzone[0]/3.78,",",bottomside_center_worzone[1]/3.78,"] mm"
    print "Utilizando ese punto como nuevo SR."
  cv2.drawContours(img, workzone, -1, GREEN, 2)
  findSquares(img)
  getColor(img)
  cv2.drawContours(img, square, -1, RED, 3) # Se pinta el cubo.
  # Se pinta el eje de referencia.
  cv2.line(img, (topside_center_workzone[0],topside_center_workzone[1]), (bottomside_center_worzone[0],bottomside_center_worzone[1]), BLUE, 1)
  changeSR()
  getWristAngle()
  # Se pinta el lado con el que se calcula el ángulo de la muñeca.
  cv2.line(img, (sorted_cube[0][0],sorted_cube[0][1]), (sorted_cube[1][0],sorted_cube[1][1]), YELLOW, 1)
  cv2.imshow('Cube detection', img)
  ch = 0xFF & cv2.waitKey()
  cv2.destroyAllWindows()
  clean()
  cam.release()

if __name__ == '__main__':
  while cont < 2 : # Se itera mientras cont < 2 (2 = num cubos -1)
    # Se realiza una captura con la cámara y se busca un cubo.
    captureAndFind()
    print "##############################"
