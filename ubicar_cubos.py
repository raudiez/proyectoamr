import numpy as np
import cv2
from math import atan, degrees , sqrt, pow, atan2, degrees, pi

##########Variables globales#################
squares = [] #Cubos encontrados, respecto a (0,0) de img.
cubos_sorted = [] #Cubos con vertices ordenados a menor Y (para angulos).
new_sides_angles = []
new_centres = []
centers = []
workzone = []
newsquares = [] #Cuadrados calculados, respecto a (0,0) del nuevo SR.
centro_ladosup_workzone = [0,0]
centro_ladoinf_workzone = [0,0]
#Colores para pintar:
ROJO=(0,0,255)
AZUL=(255,0,0)
VERDE=(0,255,0)
AMARILLO=(0,255,255)
NARANJA=(0,150,255)
BLANCO=(255,255,255)
NEGRO=(0,0,0)

def angle_cos(p0, p1, p2):
  d1, d2 = (p0-p1).astype('float'), (p2-p1).astype('float')
  return abs( np.dot(d1, d2) / np.sqrt( np.dot(d1, d1)*np.dot(d2, d2) ) )

def find_squares(img):
  img = cv2.GaussianBlur(img, (5, 5), 0)
  global cubos_sorted
  global squares
  global centers
  previous = []
  cont = 0
  sideColor = 0
  alpha = 0
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
        if len(cnt) == 4 and cv2.contourArea(cnt) > 1200 and cv2.contourArea(cnt) < 3500 and cv2.isContourConvex(cnt):
          cnt = cnt.reshape(-1, 2)
          encontrado = False
          for i in previous:
            if i[0] == cnt[0][0] or (i[0] <= cnt[0][0]+3 and i[0] >= cnt[0][0]-3) :
              encontrado = True
              break
          max_cos = np.max([angle_cos( cnt[i], cnt[(i+1) % 4], cnt[(i+2) % 4] ) for i in xrange(4)])
          if max_cos < 0.1 and not encontrado:
            squares.append(cnt)
            previous.append(cnt[0])
            previous.append(cnt[1])
            previous.append(cnt[2])
            previous.append(cnt[3])
            #Creo cubos ordenados, para luego.
            cubo = np.copy(cnt)
            dt = [('col1', cubo.dtype),('col2', cubo.dtype)]
            aux = cubo.ravel().view(dt)
            aux.sort(order=['col2','col1'])
            cubos_sorted.append(cubo)

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
        if len(cnt) == 4 and cv2.contourArea(cnt) > 40000 and cv2.contourArea(cnt) < 110000 and cv2.isContourConvex(cnt):
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
            global centro_ladosup_workzone
            centro_ladosup_workzone = [(workzone[0][0][0]+workzone[0][1][0])/2,(workzone[0][0][1]+workzone[0][1][1])/2]
            global centro_ladoinf_workzone
            centro_ladoinf_workzone = [(workzone[0][2][0]+workzone[0][3][0])/2,(workzone[0][2][1]+workzone[0][3][1])/2]

def calculate_angle():
  cont = 0
  for cubo in new_sides_angles:
    x = [cubo[0][0],cubo[1][1]]
    b = sqrt(pow((x[0]-cubo[0][0]),2)+pow((x[1]-cubo[0][1]),2))
    c = sqrt(pow((x[0]-cubo[1][0]),2)+pow((x[1]-cubo[1][1]),2))
    alpha = degrees(atan(b/c))
    if cubo[0][0] < cubo[1][0] and cubo[0][1] > cubo[1][1]:
      alpha = -degrees(atan(b/c))
    print "El cubo numero",cont+1,"tiene un giro sobre si mismo de",alpha,"grados"
    cont+=1

def change_SR():
  global new_centres
  global new_sides_angles
  #Me quedo con el lado importante para el calculo de angulos de giro.
  #El resto lo ignoro.
  for cubo in cubos_sorted:
    a = [cubo[0][0]-centro_ladoinf_workzone[0],centro_ladoinf_workzone[1]-cubo[0][1]]
    b = [cubo[1][0]-centro_ladoinf_workzone[0],centro_ladoinf_workzone[1]-cubo[1][1]]
    new_sides_angles.append([a,b])
  for centro in centers:
    a = [centro[0]-centro_ladoinf_workzone[0],centro_ladoinf_workzone[1]-centro[1]]
    b = [centro[0]-centro_ladoinf_workzone[0],centro_ladoinf_workzone[1]-centro[1]]
    new_centres.append([a,b])

if __name__ == '__main__':
#  cam = cv2.VideoCapture(0)
#  ret, img = cam.read()
  img = cv2.imread('cubos4.png')
  find_workzone(img)
  cv2.drawContours(img, workzone, -1, VERDE, 2)
  find_squares(img)
  cv2.drawContours(img, squares, -1, ROJO, 3)
  cv2.line(img, (centro_ladosup_workzone[0],centro_ladosup_workzone[1]), (centro_ladoinf_workzone[0],centro_ladoinf_workzone[1]), AZUL, 1)
  print "El nuevo origen de coordenadas sera [",centro_ladoinf_workzone[0]/3.78,",",centro_ladoinf_workzone[1]/3.78,"] mm"
  for i in centers:
    cv2.line(img, (centro_ladoinf_workzone[0],centro_ladoinf_workzone[1]), (i[0],i[1]), NARANJA, 1)
  change_SR()

  calculate_angle()

  cv2.line(img, (cubos_sorted[0][0][0],cubos_sorted[0][0][1]), (cubos_sorted[0][1][0],cubos_sorted[0][1][1]), AMARILLO, 1)
  cv2.line(img, (cubos_sorted[1][0][0],cubos_sorted[1][0][1]), (cubos_sorted[1][1][0],cubos_sorted[1][1][1]), AMARILLO, 1)
  cv2.line(img, (cubos_sorted[2][0][0],cubos_sorted[2][0][1]), (cubos_sorted[2][1][0],cubos_sorted[2][1][1]), AMARILLO, 1)

  cv2.imshow('Cube detection', img)
  ch = 0xFF & cv2.waitKey()
  cv2.destroyAllWindows()
