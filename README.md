# FaDeS
Face_Detecting_Safe

It's basically a DIY safe or a locker which unlocks with face detection...

Coming to the technical stuff I used a ESP-32 cam to detect my face.

Components used:
1.ESP 32 CAM
2. USB to TTL converter
3.Arduino
4. servo motor(for door unlocking)
5.Arduino IDE
6.PIR sensor

#One might find it weird that why did I used an arduino apart from esp-32 , Its because I was not able to power my servo using my esp as its being heating up due to face detection alone.

DESCRIPTION:
So it basically works in this way, IT Has a motion sensor , which switches on the esp 32 cam which until is in deep sleep mode , when the former detects motion.

The cam then switches on the  starts recognising the face. If it recognises then it starts the servo motor.

