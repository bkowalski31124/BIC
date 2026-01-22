import cv2
import urllib.request
import numpy as np

face_cascade = cv2.CascadeClassifier(
    cv2.data.haarcascades + 'haarcascade_frontalface_default.xml'
)

url = 'http://192.168.0.11/cam.bmp'

cv2.namedWindow("Live Transmission", cv2.WINDOW_AUTOSIZE)

while True:
    try:
        img_resp = urllib.request.urlopen(url, timeout=1)
        imgnp = np.array(bytearray(img_resp.read()), dtype=np.uint8)
        img = cv2.imdecode(imgnp, cv2.IMREAD_COLOR)

        if img is None:
            continue

        #powiekszenie obrazu
        img = cv2.resize(img, None, fx=2, fy=2, interpolation=cv2.INTER_LINEAR)

        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

        faces = face_cascade.detectMultiScale(
            gray,
            scaleFactor=1.05,
            minNeighbors=5,
            minSize=(60, 60)
        )

        for (x, y, w, h) in faces:
            cv2.rectangle(img, (x, y), (x+w, y+h), (0, 0, 255), 2)

        cv2.imshow("Live Transmission", img)

    except Exception as e:
        print("Błąd:", e)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cv2.destroyAllWindows()