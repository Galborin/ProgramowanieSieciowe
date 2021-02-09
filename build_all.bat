g++ -o server .\src\*.cpp -lws2_32 -losapi -I.\src\ -I.\include\ -L.
g++ -o app_client .\client\*.cpp -lws2_32