watch:
	while true; do make one; sleep 60; done;
one: one.c
	gcc -o one one.c -lm -I. -Ilib
