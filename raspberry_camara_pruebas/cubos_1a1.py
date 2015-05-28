#!/usr/bin/env python
# -*- coding: utf-8 -*-

#### TO DO: -distancia desde el nuevo SR hasta el centro de los cubos.
####        -reconocer unicamente 1 cubo cada vez, para la version final.

import numpy as np
import cv2
from math import atan, degrees , sqrt, pow

##########Variables globales#################
squares = [] #Cubos encontrados, respecto a (0,0) de img.
sorted_cubes = [] #Cubos con vertices ordenados por menor Y (para angulos).
cont = 0
new_sides_angles = []
new_centres = []
arm_angles = []
hand_angles = []
centers = []
workzone = []
newsquares = [] #Cuadrados calculados, respecto a (0,0) del nuevo SR.
topside_center_workzone = [0,0]
bottomside_center_worzone = [0,0]
#Colores para pintar:
RED=(0,0,255)
BLUE=(255,0,0)
CYAN=(255,150,0)
GREEN=(0,255,0)
YELLOW=(0,255,255)
ORANGE=(0,150,255)
WHITE=(255,255,255)
BLACK=(0,0,0)

def angle_cos(p0, p1, p2):
  d1, d2 = (p0-p1).astype('float'), (p2-p1).astype('float')
  return abs( np.dot(d1, d2) / np.sqrt( np.dot(d1, d1)*np.dot(d2, d2) ) )


#Función que encuentra los cuadrados de las caras superiores de cada cubo,
#obtiene los cuadrados del contorno, y los centros.
def find_squares(img):
  img = cv2.GaussianBlur(img, (5, 5), 0)
  global sorted_cubes
  global squares
  global centers
  global cont
  previous = []
  sideColor = 0
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
              if len(cnt) == 4 and cv2.contourArea(cnt) > 1200 and cv2.contourArea(cnt) < 4000 and cv2.isContourConvex(cnt):
                cnt = cnt.reshape(-1, 2)
                encontrado = False
                #Se hace un bucle para buscar dentro de los contornos los más próximos
                #para cada cubo.
                for i in previous:
                  if i[0] == cnt[0][0] or (i[0] <= cnt[0][0]+3 and i[0] >= cnt[0][0]-3) :
                    #Si se encuentra este contorno en los anteriormente analizados,
                    #no se usa.
                    encontrado = True
                    break
                max_cos = np.max([angle_cos( cnt[i], cnt[(i+1) % 4], cnt[(i+2) % 4] ) for i in xrange(4)])
                if max_cos < 0.1 and not encontrado:
                  #Se almacena el contorno del cuadrado con sus vertices, se almacena
                  #como analizado tambien.
                  squares.append(cnt)
                  previous.append(cnt[0])
                  previous.append(cnt[1])
                  previous.append(cnt[2])
                  previous.append(cnt[3])
                  #Se ordenan los vertices de los cubos, para luego usarlos para
                  #los angulos.
                  cubo = np.copy(cnt)
                  dt = [('col1', cubo.dtype),('col2', cubo.dtype)]
                  aux = cubo.ravel().view(dt)
                  aux.sort(order=['col2','col1'])
                  sorted_cubes.append(cubo)
                  #Se obtienen el centro del cuadrado, y se saca el color de la cara.
                  center = [(cnt[0][0]+cnt[2][0])/2,(cnt[0][1]+cnt[2][1])/2]
                  centers.append([(cnt[0][0]+cnt[2][0])/2,(cnt[0][1]+cnt[2][1])/2])
                  imggray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
                  sideColor = imggray[center[1]][center[0]] # Color de cara superior
                  if sideColor >= 100 :
                    color = "blanco"
                  else :
                    color = "negro"
                  print "Encontrado cubo numero",cont+1,"con centro en [",center[0]/3.78,",",center[1]/3.78,"] mm y es de color",color
                  cont+=1
                  cuboFound = True


#Se encuentra un rectangulo de area mayor que los cubos, que los contiene y
#vale de referencia para ubicacion. Hace practicamente lo mismo que la funcion
#anterior para los cubos. Obtiene ademas puntos de referencia a mitad del
#rectangulo de referencia para construir un eje de coordenadas.
def find_workzone(img):
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
        if len(cnt) == 4 and cv2.contourArea(cnt) > 40000 and cv2.contourArea(cnt) < 130000 and cv2.isContourConvex(cnt):
          cnt = cnt.reshape(-1, 2)
          encontrado = False
          for i in previous:
            if i[0] == cnt[0][0] or (i[0] <= cnt[0][0]+3 and i[0] >= cnt[0][0]-3) :
              encontrado = True
              break
          max_cos = np.max([angle_cos( cnt[i], cnt[(i+1) % 4], cnt[(i+2) % 4] ) for i in xrange(4)])
          if max_cos < 0.1 and not encontrado:
            workzone.append(cnt)
            previous.append(cnt[0])
            getSR()

#Función que ordena los puntos del rectángulo de referencia y obtiene el
#lado superior y el inferior.
def getSR():
  top = [0,0]
  bottom = [0,0]
  topi=bottomi=i=0
  while i < 4:
    #Se compara respecto a la mitad de la imagen (240 px).
    if workzone[0][i][1] < 240:
      top[topi]=[workzone[0][i][0],workzone[0][i][1]]
      topi+=1
    else:
      bottom[bottomi]=[workzone[0][i][0],workzone[0][i][1]]
      bottomi+=1
    i+=1
  top.sort
  bottom.sort
  global topside_center_workzone
  topside_center_workzone = [(top[0][0]+top[1][0])/2,(top[0][1]+top[1][1])/2]
  global bottomside_center_worzone
  bottomside_center_worzone = [(bottom[0][0]+bottom[1][0])/2,(bottom[0][1]+bottom[1][1])/2]


#Función que calcula el angulo de giro para la pinza del brazo robotico (o el
#giro de los cubos sobre su propio eje).
def getArmAngle():
  global cont
  for centro in new_centres:
    x = [centro[0],0]
    b = sqrt(pow((x[0]-centro[0]),2)+pow((x[1]-centro[1]),2))
    c = sqrt(pow((x[0]-0),2)+pow((x[1]-0),2))
    alpha = degrees(atan(b/c))
    if centro[0] < 0:
      alpha = -degrees(atan(b/c))
    print "El cubo numero",cont,"esta a una inclinacion de",alpha,"grados respecto al eje de referencia"
    arm_angles.append(alpha)

#Función que calcula el angulo de giro para el brazo robotico (giro respecto al
#eje del nuevo SR).
def getHandAngle():
  global cont
  for cubo in new_sides_angles:
    x = [cubo[0][0],cubo[1][1]]
    b = sqrt(pow((x[0]-cubo[0][0]),2)+pow((x[1]-cubo[0][1]),2))
    c = sqrt(pow((x[0]-cubo[1][0]),2)+pow((x[1]-cubo[1][1]),2))
    alpha = degrees(atan(b/c))
    if cubo[0][0] < cubo[1][0] and cubo[0][1] > cubo[1][1]:
      alpha = -degrees(atan(b/c))
    print "El cubo numero",cont,"tiene un giro sobre si mismo de",alpha,"grados"
    hand_angles.append(alpha)

#Función que obtiene nuevas coordenadas para los centros de los cubos y los lados
#de los cubos que serviran para determinar el giro de la pinza del brazo robotico.
def change_SR():
  #Se modifican las coordenadas respecto al nuevo SR:
  #El punto medio del lado inferior del rectangulo de referencia.
  global new_centres
  global new_sides_angles
  #Me quedo con el lado importante para el calculo de angulos de giro.
  #El resto lo ignoro.
  for cubo in sorted_cubes:
    a = [cubo[0][0]-bottomside_center_worzone[0],bottomside_center_worzone[1]-cubo[0][1]]
    b = [cubo[1][0]-bottomside_center_worzone[0],bottomside_center_worzone[1]-cubo[1][1]]
    new_sides_angles.append([a,b])
  #Ahora cambio las coordenadas de los centros respecto al nuevo SR.
  for centro in centers:
    a = [centro[0]-bottomside_center_worzone[0],bottomside_center_worzone[1]-centro[1]]
    new_centres.append(a)

if __name__ == '__main__':
  cam = cv2.VideoCapture(0)
  ret, img = cam.read()
  #BUSCAR PRIMER CUBO (1)
  #img = cv2.imread('3cubos.png')
  find_workzone(img)
  cv2.drawContours(img, workzone, -1, GREEN, 2)
  find_squares(img)
  cv2.drawContours(img, squares, -1, RED, 3)
  cv2.line(img, (topside_center_workzone[0],topside_center_workzone[1]), (bottomside_center_worzone[0],bottomside_center_worzone[1]), BLUE, 1)
  print "El nuevo origen de coordenadas sera [",bottomside_center_worzone[0]/3.78,",",bottomside_center_worzone[1]/3.78,"] mm"
  print "Utilizando ese punto como nuevo SR."
  for i in centers:
    cv2.line(img, (bottomside_center_worzone[0],bottomside_center_worzone[1]), (i[0],i[1]), ORANGE, 1)
    cv2.line(img, (i[0],bottomside_center_worzone[1]), (i[0],i[1]), CYAN, 1)
  change_SR()
  getArmAngle()
  getHandAngle()

  cv2.line(img, (sorted_cubes[0][0][0],sorted_cubes[0][0][1]), (sorted_cubes[0][1][0],sorted_cubes[0][1][1]), YELLOW, 1)

  cv2.imshow('Cube detection', img)
  ch = 0xFF & cv2.waitKey()
  cv2.destroyAllWindows()

  squares = []
  sorted_cubes = []
  new_sides_angles = []
  new_centres = []
  arm_angles = []
  hand_angles = []
  centers = []
  newsquares = [] #Cuadrados calculados, respecto a (0,0) del nuevo SR.
  cam.release()

  #BUSCAR SEGUNDO CUBO (2)
  cam = cv2.VideoCapture(0)
  ret, img = cam.read()
  #img = cv2.imread('2cubos.png')
  cv2.drawContours(img, workzone, -1, GREEN, 2)
  find_squares(img)
  cv2.drawContours(img, squares, -1, RED, 3)
  cv2.line(img, (topside_center_workzone[0],topside_center_workzone[1]), (bottomside_center_worzone[0],bottomside_center_worzone[1]), BLUE, 1)
  for i in centers:
    cv2.line(img, (bottomside_center_worzone[0],bottomside_center_worzone[1]), (i[0],i[1]), ORANGE, 1)
    cv2.line(img, (i[0],bottomside_center_worzone[1]), (i[0],i[1]), CYAN, 1)
  change_SR()
  getArmAngle()
  getHandAngle()

  cv2.line(img, (sorted_cubes[0][0][0],sorted_cubes[0][0][1]), (sorted_cubes[0][1][0],sorted_cubes[0][1][1]), YELLOW, 1)

  cv2.imshow('Cube detection', img)
  ch = 0xFF & cv2.waitKey()
  cv2.destroyAllWindows()

  squares = []
  sorted_cubes = []
  new_sides_angles = []
  new_centres = []
  arm_angles = []
  hand_angles = []
  centers = []
  cam.release()

  #BUSCAR TERCER CUBO (3)
  cam = cv2.VideoCapture(0)
  ret, img = cam.read()
  #img = cv2.imread('1cubos.png')
  cv2.drawContours(img, workzone, -1, GREEN, 2)
  find_squares(img)
  cv2.drawContours(img, squares, -1, RED, 3)
  cv2.line(img, (topside_center_workzone[0],topside_center_workzone[1]), (bottomside_center_worzone[0],bottomside_center_worzone[1]), BLUE, 1)
  for i in centers:
    cv2.line(img, (bottomside_center_worzone[0],bottomside_center_worzone[1]), (i[0],i[1]), ORANGE, 1)
    cv2.line(img, (i[0],bottomside_center_worzone[1]), (i[0],i[1]), CYAN, 1)
  change_SR()
  getArmAngle()
  getHandAngle()

  cv2.line(img, (sorted_cubes[0][0][0],sorted_cubes[0][0][1]), (sorted_cubes[0][1][0],sorted_cubes[0][1][1]), YELLOW, 1)

  cv2.imshow('Cube detection', img)
  ch = 0xFF & cv2.waitKey()
  cv2.destroyAllWindows()
