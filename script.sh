sudo rmmod buzzer_dev
sudo rmmod servo_dev
sudo rmmod led_dev

cd Buzzer
sudo make clean
make
sudo insmod Buzzer_dev.ko
cd ..

cd Servo
sudo make clean
make
sudo insmod servo_dev.ko
cd ..

cd LED
sudo make clean
make
sudo insmod led_dev.ko
cd ..

cd WaterDepth
sudo make clean
make
sudo insmod wls.ko
cd..

rm Bottle_app
gcc -o Bottle_app Bottle_app.c -lpthread
