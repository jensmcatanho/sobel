watch:
	while true; do make one; sleep 60; done;
one: one.c
	gcc -o one one.c -lm -I.
two: two.c
	gcc -o two two.c -lm -lpthread -I.
three: three.c
	gcc -o three three.c -lm -fopenmp -I.
four: four.c
	gcc -o four four.c -lm -I.
