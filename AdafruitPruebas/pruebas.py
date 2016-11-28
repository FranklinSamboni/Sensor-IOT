import time
import Adafruit_BBIO.ADC as ADC

ADC.setup()

while True:

	valueX = ADC.read("P9_39") # x
	valueY = ADC.read("P9_37") # y
	valueZ = ADC.read("P9_35") # z

	valueRawX = ADC.read_raw("P9_39") # x
	valueRawY = ADC.read_raw("P9_37") # y
	valueRawZ = ADC.read_raw("P9_35") # z

	voltageX = valueX * 1.8 #1.8V # x
	voltageY = valueY * 1.8 # 1.8V # y
	voltageZ = valueZ * 1.8 # z

	#print("Vx: ", voltageX,"| Vy: ", voltageY, "| Vz: ", voltageZ)
	print("Raw x: ", valueRawX,"| Raw y: ", valueRawY,"| Raw z: ", valueRawZ)
	#gedit mm
	#otra linea
	time.sleep(1)



