sudo rmmod buzzer_dev
sudo rmmod servo
sudo rmmod led_dev
sudo rmmod wls

cd Buzzer
sudo make clean
make
sudo insmod buzzer_dev.ko
cd ..

cd servo
sudo make clean
make
sudo insmod servo.ko
cd ..

cd LED
sudo make clean
make
sudo insmod led_dev.ko
cd ..

cd WaterDepth
sudo make clean
sudo make
sudo insmod wls.ko
cd ..

rm Bottle_app
gcc -o Bottle_app Bottle_app.c -lpthread
