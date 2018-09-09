rm test.out
gcc -o test.out poker_test.c poker_compare.c Utils.c poker.c -I ./include -g
./test.out special.txt
./test.out data.txt
./test.out all.txt
./test.out error.txt
