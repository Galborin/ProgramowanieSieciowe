# ProgramowanieSieciowe

Projekt to komunikator tekstowy obsługujący chatroomy.

Klienci na początku muszą wybrać unikalną nazwę, następnie wraz z adresem zapisywani są do listy użytkowników.
Serwer przetwarza żądania klientów - mogą oni za pomocą komend wyśiwietlić dostępne komendy, dostępnych użytkowników, dostępne chatroomy, stworzyć chatroom, dołączyć/opuścić chatroom.

Klienci komunikują się po dołączeniu do jakiegokolwiek chatroomu - napisana przez nich wiadomosć jest przesylana do wszystkich użytkowników w chatroomie.

Program korzysta z biblioteki POSIX thread (pthread).

Przykładowa kompilacja w gcc:
gcc -o app app.c chatroom.c client.c command_interface.c userlist.c -lpthread
