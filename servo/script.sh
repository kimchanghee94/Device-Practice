sudo rmmod servo
make clean
make
sudo insmod servo.ko
rm app
gcc -o app app.c