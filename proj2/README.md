# IPK - Projekt 2

## Popis zadání
Cílem projektu bylo implementovat aplikaci serveru pro komunikaci s klienty pomocí protokolu IPK24-CHAT[1].
Aplikace umí pracovat s protokolem TCP[2] a UDP[3].

## Spuštění aplikace
Pro spuštění je potřeba zadat příkaz `make`, který vygeneruje spustitelný soubor `ipk24chat-server` a spustit daný soubor pomocí `ipk24chat-server`.
* Výchozí adresa je nastavena na 0.0.0.0
* Výchozí port je nastaven na hodnotu 4567.
* Výchozí počet pokusů o opětovné zaslání zprávy (jen u `UDP`) je nastaven na hodnotu 3.
* Výchozí doba čekání na přijetí zprávy `CONFIRM` (jen u `UDP`) je nastavena na hodnotu 250ms.

Tyto hodnoty je možné explicitně měnit pomocí přepínačů [-l `adresa` -p `port` -d `timeout` -r `retry`].
Pro vypsání krátke nápovědy lze použít přepínač [-h].

## Implementace
Program je implementován v jazyce C a pracuje se vstupy z příkazové řádky, jež udávají adresu serveru a port. Pro zpracování zpráv protokolu UDP je možnost zvolit počet pokusů o opětovné zaslání zprávy po vypršení času, který je taktéž možno explicitně zvolit v příkazové řádce. Po spuštění aplikace se vytvoří 2 `sockety`[7] (`TCP`[2] a `UDP`[3]), které je možno použit pro následnou komunikaci s klienty. Po startu aplikace se v souboru `ipk.c` zpracují parametry příkazové řádky a volá se funkce `create_socket()`, která vytvoří dříve zmíněné 2 `sockety`[7].

Funkce `create_socket()` aplikuje `bind`[8] na `socket`[7] typu `TCP`[2] a `UDP`[3] a volá funkci `concrete_connection()`, která vytvoří nové vlákno programu. Nově vytvořené vlákno dále zpracovává `UDP`[3] komunikaci. Hlavní vlákno programu pokračuje funkcí `listener()`, která připraví `TCP welcome socket`[] pro naslouchání a dále zpracovává přijatou komunikaci funkcí `accept()`[4].

### TCP komunikace
* Každá přijatá komunikace funkcí `accept()`[4] vytvoří nové vlákno pro komunikací s tímto klientem. Původní vlákno čeká na další příchozí komunikace.
* Nové vlákno komunikuje s konkrétním klientem pomocí nově vytvořeného `socketu`[7], tento `socket`[7] je nastaven jako neblokující, aby bylo možné zachytit signál `C-c`[6] a ukončit tak komunikaci a následně i celou aplikaci.
* Příjaté zprávy od klienta jsou zpracovávány funkcí `receive_message()`, která používá pomocné funkce pro kontrolu správnosti přijaté zprávy a ukládá parametry zprávy do příslušné struktury.

### UDP komunikace
* Přijímání UDP komunikace probíhá v jednom vlákně, které čeká na přijaté zprávy na výchozím `socketu`[7].
* Při přijetí zprávy na výchozí `socket`[7] se vytvoří dynamický port a nový `socket`[7], pro následnou komunikaci s tímto klientem. Dané parametry dynamické adresy se uloží do příslušné struktury, jež uchovává všechny adresy pro komunikaci konkretními klienty.
* Komunikace se zachytává funkcí `select()`[5], která kontroluje aktivitu na výchozím i všech dynamických `socketech`[7].
* První zpráva od nového klienta musí být typu `AUTH`, následně se tomuto klientovi odpoví zprávou typu `REPLY` z nově přířazené adresy tomuto klientovi a další komunikace s tímto klientem probíhá výhradně na této adrese.
* Na každou přijatou zprávu server odešle klientovi potvrzení o přijetí pomocí `CONFIRM` zprávy.

### Společné chování
* Při spuštění aplikace se vytvoří seznam kanálů a vloží se do něj výchozí kanál s názvem `General`. Každý kanál obsahuje adresy svých klientů. Po příjetí zprávy typu `AUTH` je klient připojen do výchozího kanálu. Při přijetí zprávy typu `JOIN` se vytvoří nový kanál, do nějž se chce klient připojit, pokud dosud neexistuje kanál s tímto názvem, a klient je do tohoto kanálu připojen. Všichni uživatelé, kteří jsou připojení v tomto kanálů jsou obeznámení o přípojení a odpojení jiných uživatelů.
* Pro přijetí zprávy typu `BYE`, je uživatel odpojen z daného kanálu.
* Pří zachycení signálu `C-c` se všem uživatelům napříč všemi kanály zašle zpráva `BYE` pro ukončení komunikace s nimi, server uzavře všechny `sockety`[7] a uvolní se všechny alokované prostředky.
* Výstupem serveru jsou informace o přijatých a odeslaných zprávách ve formátu `RECV/SENT {FROM_IP}:{FROM_PORT} | {MESSAGE_TYPE}`.

### Testování
* Server byl testován pomocí aplikace klienta `IPK24chat-client` z prvního projektu IPK.

### Zdroje
[1] [Project1] Dolejška, D. Client for a chat server using IPK24-CHAT protocol [online]. February 2024. [cited 2024-02-18]. Available at: https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Project%201

[2] [RFC793] Postel, J., "Transmission Control Protocol", STD 7, RFC 793, DOI 10.17487/RFC0793, September 1981. [Online]. Available: https://www.ietf.org/rfc/rfc0793.txt. [cited 2024-04-18].

[3] [RFC768] Postel, J., "User Datagram Protocol", RFC 768, DOI 10.17487/RFC0768, August 1980. [Online]. Available: https://www.ietf.org/rfc/rfc768.txt. [cited 2024-04-14].

[4] [Linux Programmer's Manual :: accept(2) - Linux manual page] man7.org. [Online]. Available at: https://man7.org/linux/man-pages/man2/accept.2.html. [cited 2024-04-18].

[5] [Linux Programmer's Manual :: select(2) - Linux manual page] man7.org. [Online]. Available at: https://man7.org/linux/man-pages/man2/select.2.html. [cited 2024-04-18].

[6] [Linux Programmer's Manual :: signal(2) - Linux manual page] man7.org. [Online]. Available at: https://man7.org/linux/man-pages/man2/signal.2.html [cited 2024-04-18].

[7] [Linux Programmer's Manual :: socket(2) - Linux manual page] man7.org. [Online]. Available at: https://man7.org/linux/man-pages/man2/socket.2.html. [cited 2024-04-18].

[8] 
[Linux Programmer's Manual :: bind(2) - Linux manual page] man7.org. [Online]. Available at: https://man7.org/linux/man-pages/man2/bind.2.html. [cited 2024-04-18].
