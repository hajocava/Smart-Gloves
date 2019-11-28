import serial
import time
import json
import csv
import sys

ser = serial.Serial('COM4', 115200, timeout=0)
with open('sensoresTrain.csv', 'a', newline='') as csvfile: 
	regwriter = csv.writer(csvfile, delimiter=',', quoting=csv.QUOTE_MINIMAL)
	# regwriter.writerow(["ax", "ay", "az", "gx", "gy", "gz", "d1", "d2", "d3", "d4", "target"])
	while True:
		try:
			data1 = ser.readline()
			data = data1.decode('utf8').replace("'",'"')
			#print(data)
			HUD_data = json.loads(data)
			# Obteniendo la lectura 
			a_x = HUD_data['ax']
			a_y = HUD_data['ay']
			a_z = HUD_data['az']
			g_x = HUD_data['gx']
			g_y = HUD_data['gy']
			g_z = HUD_data['gz']
			d1 = HUD_data['d1']
			d2 = HUD_data['d2']
			d3 = HUD_data['d3']
			d4 = HUD_data['d4']
			target = "4"
			print ("acc_x ={} acc_y={} acc_z ={}".format(a_x, a_y, a_z))

			regwriter.writerow([str(a_x), str(a_y),str(a_z),str(g_x), str(g_y), str(g_z), str(d1), str(d2), str(d3), str(d4), target])
			#print(data1)
			#time.sleep(0.01)
		except:
			#print("no reads")
			time.sleep(0.1)
			pass