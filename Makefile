watch:
	while true; do make one; sleep 60; done;
one: one.c
	gcc -o one one.c -lm -I. -Ilib
three: three.c
	gcc -o three three.c -lm -fopenmp -I. -Ilib
