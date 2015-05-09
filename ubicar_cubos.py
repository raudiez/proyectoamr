import numpy as np
import cv2
from math import atan, degrees , sqrt, pow

squares = []
centers = []

def angle_cos(p0, p1, p2):
  d1, d2 = (p0-p1).astype('float'), (p2-p1).astype('float')
  return abs( np.dot(d1, d2) / np.sqrt( np.dot(d1, d1)*np.dot(d2, d2) ) )

def find_squares(img):
  img = cv2.GaussianBlur(img, (5, 5), 0)
  global squares
  global centers
  previous = []
  cont = 0
  center = [0,0]
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
            # print "P1. X=",cnt[0][0]/3.78,"Y=",cnt[0][1]/3.78
            previous.append(cnt[1])
            # print "P2. X=",cnt[1][0]/3.78,"Y=",cnt[1][1]/3.78
            previous.append(cnt[2])
            # print "P3. X=",cnt[2][0]/3.78,"Y=",cnt[2][1]/3.78
            previous.append(cnt[3])
            # print "P4. X=",cnt[3][0]/3.78,"Y=",cnt[3][1]/3.78
            center[0]=(cnt[0][0]+cnt[2][0])/2 # Center's X.
            cx=center[0]/3.78
            center[1]=(cnt[0][1]+cnt[2][1])/2  # Center's Y.
            cy=center[1]/3.78
            alpha=degrees(atan((cnt[1][1]-cnt[0][1])/(cnt[1][0]-cnt[0][0])))
            centers.append([(cnt[0][0]+cnt[2][0])/2,(cnt[0][1]+cnt[2][1])/2])
            imggray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
            sideColor=imggray[center[1]][center[0]] # Color de cara superior
            if sideColor >= 100 :
              color = "blanco"
            else :
              color = "negro"
            print "Encontrado cubo numero",cont+1,"con centro en [",cx,",",cy,"] mm y es de color",color,"y con inclinacion",alpha,"grados"
            cont+=1
  return squares

def find_workzone(img):
  img = cv2.GaussianBlur(img, (5, 5), 0)
  squares = []
  previous = []
  global centers
  center = [0,0]
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
        if len(cnt) == 4 and cv2.contourArea(cnt) > 40000 and cv2.contourArea(cnt) < 110000 and cv2.isContourConvex(cnt):
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
            centers.append([(cnt[0][0]+cnt[2][0])/2,(cnt[0][1]+cnt[2][1])/2])
            print "Encontrado centro de zona de trabajo en [",((cnt[0][0]+cnt[2][0])/2)/3.78,",",((cnt[0][1]+cnt[2][1])/2)/3.78,"] mm"
  return squares

if __name__ == '__main__':
#  cam = cv2.VideoCapture(0)
#  ret, img = cam.read()
  img = cv2.imread('cubos4.png')
  workzone = find_workzone(img)
  cv2.drawContours( img, workzone, -1, (0, 255, 0), 2 )
  squares = find_squares(img)
  cv2.drawContours( img, squares, -1, (0, 0, 255), 3 )
  cv2.imshow('Cube detection', img)
  ch = 0xFF & cv2.waitKey()
  cv2.destroyAllWindows()
