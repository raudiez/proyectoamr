import numpy as np
import cv2
from math import atan, degrees , sqrt, pow

##########Variables globales#################
squares = [] #Cuadrados encontrados, respecto a (0,0) de img.
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
            center = [(cnt[0][0]+cnt[2][0])/2,(cnt[0][1]+cnt[2][1])/2]
            #alpha = degrees(atan((cnt[1][1]-cnt[0][1])/(cnt[1][0]-cnt[0][0])))
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

def change_SR():
  for square in squares:
    p1 = square[0]
    p2 = square[1]
    p3 = square[2]
    p4 = square[3]
    print p1
    print p2
    print p3
    print p4
    xList = [p[0] for p in square]
    yList = [p[1] for p in square]
    print xList
    r=[xList[0]/yList[0],xList[1]/yList[1],xList[2]/yList[2],xList[3]/yList[3]]
    print r
    print map(min, [r])

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
  cv2.imshow('Cube detection', img)
  ch = 0xFF & cv2.waitKey()
  cv2.destroyAllWindows()
