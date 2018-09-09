rm send.out
gcc -o send.out poker_send_test.c poker.c Utils.c -I ./include
./send.out 0
#./send.out 1
