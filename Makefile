all:
	gcc `pkg-config --cflags gtk4` -Iclient_program/include client_program/screens/second_screen.c client_program/screens/first_screen.c client_program/logic.c client_program/interface.c client_program/main.c  -o client `pkg-config --libs gtk4`
	gcc server1_program/server1.c -o server1 -lX11
	gcc server2_program/server2.c -o server2 -lX11

clean:
	rm -f client server1 server2