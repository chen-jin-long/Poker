rm test.out
gcc -o test.out poker_test.c poker_compare.c Utils.c poker.c
./test.out all.txt
