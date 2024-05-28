# IPK Projekt - 1

## Popis zadání
Cílem projektu bylo implementovat aplikaci klienta pro komunikaci se serverem pomocí protokolu IPK24-CHAT[1].
Aplikace umí pracovat s protokolem TCP[2] a UDP[3].

## Spuštění aplikace
Pro spuštění je potřeba zadat příkaz *make*, který vygeneruje spustitelný soubor *ipk24chat-client* a spustit daný soubor spolu s požadovanými parametry přepínačů ve formátu [-t *[tcp/udp]* -s *hostname*].
* Výchozí port je nastaven na hodnotu 4567.
* Výchozí počet pokusů o opětovné zaslání zprávy (jen u *UDP*) je nastaven na hodnotu 3.
* Výchozí doba čekání na přijetí zprávy *CONFIRM* (jen u *UDP*) je nastavena na hodnotu 250ms.

Tyto hodnoty je možné explicitně měnit pomocí přepínačů [-p *port* -d *timeout* -r *retry*].
Pro vypsání krátke nápovědy lze použít přepínač [-h].

## Implementace
Program je implementován v jazyce C a pracuje se vstupy z příkazové řádky, jež udávají adresu serveru, port a mód, ve kterém klient komunikuje se serverem. Při použití protokolu UDP je možnost zvolit počet pokusů o opětovné zaslání zprávy po vypršení času, který je taktéž možno explicitně zvolit v příkazové řádce. Po spuštění aplikace klienta se dle zvoleného módu vytvoří *socket*, který je použit pro následnou komunikaci se serverem. Po startu aplikace se v souboru *ipk.c* zpracují parametry příkazové řádky, vytvoří se *socket* pro spojení se serverem a volá se funkce *run_tcp()* nebo *run_udp()* na základě zvoleného módu.

Uživatel komunikuje se serverem pomocí sady příkazů[4], pomocí kterých se může na serveru autentifikovat, změnit komunikační kanál, změnit uživatelské jméno či vypsat krátkou nápovědu. Vstup, který nezačíná znakem *"/"* (není tedy příkazem), je interpretován jako zpráva od uživatele.

* int run_tcp/udp(int socket,struct sockaddr *address, socklen_t address_size)
    * socket - popisovač spojení
    * address - adresa serveru

### TCP mód
* Po zavolání funkce *run_tcp()* se alokují struktury(1): *tcp_auth* pro autentifikační zprávy, *tcp_join* pro změnu kanálu, *tcp_msg* pro uživatelské zprávy a struktury pro zpracování jednotlivých odpovědí se serveru.
* U **TCP** módu se vytvoří spojení se serverem funkcí *connect()*, následně se čeká na vstup od uživatele.
Po přijetí vstupu od uživatele se tento vstup zpracuje pomocí funkce *parse_commandTCP()* ze souboru *tcp_command*, která daný příkaz či zprávu zpracuje a uloží potřebné hodnoty do struktur(1). Funkce vrací hodnotu *command* použitého příkazu/ hodnoty pro zprávu.
* Následně se podle získané hodnoty *command* pomocí funkce *setup_msgT()* ze souboru *tcp_message* vytvoří příslušná zpráva z uložených hodnot ve strukturách(1). Výsledná zpráva je uložena ve struktuře *Setup_msg*. Následně se zprává odešle na server funkcí *send()*.
Pokud proměnná *command* byla hodnoty *AUTH* nebo *JOIN*, je potřeba před odesláním dalších zpráv nejprve přijmout zprávu ze serveru, která je typu *REPLY*. Do té doby se uzamkne načítání uživatelského vstupu, tuto událost řídí proměnná *getReply*, která značí, zda je třeba čekat na zprávu typu *REPLY* či nikoliv. Po přijetí této zprávy změní proměnná *getReply* svou pravdivostní hodnotu a tím odemkne uživatelský vstup.
* Pokud aplikace přijme zprávu ze serveru, zpracuje její obsah a následně podle typu přijaté zprávy, který zjistí z první části této zprávy, vypíše její obsah v příslušném formátu na standardní nebo chybový výstup. Z přijaté zprávy zjistí také jméno odesílatele nebo úspěšnost předchozího příkazu.

### UDP mód
* Po zavolání funkce *run_udp()* se alokují struktury(2): *udp_auth* pro autentifikační zprávy, *udp_join* pro změnu kanálu, *user_msg* pro uživatelské zprávy a struktury pro zpracování jednotlivých odpovědí se serveru. U *UDP* módu se nenavazuje spojení se serverem jako u *TCP*, ale posílají se a příjímají zprávý z předem nastavené adresy cíle a zdroje.
Aplikace je schopna přijímat zprávy z dynamického portu a následně smeřovat další komunikací na tento port. Dynamický port zjistí z funkce *recvfrom()*, která adresu uloží do struktury *sockaddr address*.
* Program po přijetí vstupu od uživatele tento vstup zpracuje pomocí funkce *parse_command()* ze souboru *udp_command*, která daný příkaz či zprávu zpracuje a uloží potřebné hodnoty do struktur(2). Funkce vrací hodnotu *command*, která je předána funkci *setup_message()*, tato funkce sestaví zprávu v binární podobě[5] na základě hodnoty *command* a uloží výslednou zprávu do struktury *Setup_msg*. Všechny odesíláné zprávy mají unikátní identifikátor *messageID*, který se inkrementuje s každou odeslánou zprávou. Tento identifikátor je součástí obsahu odesíláné zprávy, díky kterému server může odpivídat na zprávu s konkrétním identifikátorem nebo potvrzovat její příjetí. Protokol *UDP* je nespolehlivý, a proto je třeba kontrolovat, zda byla zpráva doručena příjemci. Pro tuto kontrolu program využívá proměnnou *messageConfirmed*, která po odeslání zprávy změní svojí pravdivostní hodnotu a tím uzamkne načítání a odesílání dalších zpráv se standardního vstupu dokud nebude potvrzeno doručení předchozí zprávy. Po přijetí zprávy ze serveru se pomocí funkce *parse_message()* ze souboru *udp_receive* zpracuje přijatá zpráva a jednotlivé části obsahu zprávy se uloží to struktur pro přijaté zprávy. Funkce vrací hodnotu *opcode*, jež odpovídá prvnímu bajtu zprávy. Tato hodnota se porovnává s hodnotami ze struktury MSG_type a podle nalezeného typu se na standardní či chybový výstup výpíše její obsah v požadovaném formátu. Pokud se jedná o zprávu typu *CONFIRM*, zkontroluje se číslo, jež je obsahem zprávy, zda souhlasí s identifikátorem *messageID* a je tedy potvrzením poslední odeslané zprávy. Pokud je tomu tak, proměnná *messageConfirmed* změní svou hodnotu a umožní odemknutí funkce pro načítání další zprávy/příkazu ze standardního vstupu. Pokud byla odesláná zpráva typu *AUTH* nebo *JOIN*, proměnná *needReply* uzamkne načítání standardního vstupu, dokud aplikace nepřijme zprávu ze serveru typu *REPLY* s odpivídajícím číslem odkazujícím se na dříve odeslanou zprávu typu *AUTH* nebo *JOIN*. Po přijetí této zprávy proměnná *needReply* změní svojí hodnotu a umožní odemčení standardního vstupu. Pokud byla odesláná zpráva potvrzená a zároveň přijatá odpověď, pokud ji zpráva vyžadovala, je kompletně odemčen standardní vstup.

Podle proměnné *messageConfirmed* se zároveň kontroluje, zda byla zpráva doručena na server v rozmezí 250ms (výchozí hodnota). Pro tuto kontrolu se při odesílání zprávy zároveň spustí časovač. Pokud zpráva nebyla potvrzena do 250ms, odešle se znovu. Tento proces se opakuje dokud nevyprší počet pokusů (výchozí = 3) o opětovné zaslání zprávy. Pokud zpráva nebyla potvrzena ani po těchto pokusech, program se ukončí.

Pro odesílání jednotlivých typů zpráv se používají pomocné funkce jako *send_bye()* *send_error()* *send_confirm()*, jenž využívají funkci *sender()*, která slouží jako hlavní funkce pro odesilání zpráv.

Funkce *parse_command()* a *parse_commandTCP()* využívají pro kontrolu zadaných příkazů a jejich parametrů pomocné funkce:
* *regex()* / *regexTCP()*  pro kontrolu, zda parametry příkazů splňují pravidla regulárních výrazů[7].
* *check_command()* / *check_commandTCP()* pro kontrolu, zda byl zadaný příkaz platný.
Při nalezení chyby těmito funkcemi se zpráva neodešle a program je připraven načítat nový příkaz/zprávu.


Program používá funkci *select()*[6] pro neblokující operace, aby bylo dosaženo správného pořadí odesílání a přijímání zpráv. Funkce sleduje aktivitu na dvou popisovačích: standardním vstupu a *socketu*.
Aplikace pracuje ve smyčce dokud se nenacházi v konečném stavu *state_END* nebo nebyla přerušená signálem *C-c*, který je odchycen pomocí *signal()*[7]. V obou těchto případech aplikace pošle zprávu *BYE* a ukončí se serverem spojení.
V oboud módech si program uchováva svůj aktuální stav, podle kterého řídí zpracovávání jednotlivých příkazů čí přijatých zpráv. Pokud uživatel použil příkaz, který není v daném stavu programu možné použít, vypíše se na standardní chybový výstup hláška upozorňující na toto porušení a uživatelem zadaná zpráva či zpracovaný příkaz se neodešle. V případě, že aplikace přijala zprávu ze serveru typu *ERR* nebo zprávu s neznámým typem, změní svůj stav na *stateERROR*, pošle zprávu *BYE* a přejde do stavu *stateEND* ve kterém se ukončí.

## Testování
Program byl testován na vývojovém prostředí Nix.

Testy lze spustit příkazem *python3 test.py*. (python verze 3.9 >=)

### Popis
Soubor *test.py* vytvoří dva procesy, jeden pro *tester.py*, který slouží jako jednoduchý server pro komunikaci s klientem. V tomto souboru server čeká na zprávy od klienta a odesílá mu zpět odpovědi v požadovaném formátu. Druhým procesem je spuštění aplikace *ipk24chat-client*, který vytvoří spojení se serverem ze souboru *tester.py*.
Následně se porovnává výstup aplikace *ipk24chat-client*, s požadovaným výstupem. Test kontroluje správnost příjatých odpovědí ze serveru, ale i odhalení chyb ze standardního vstupu klienta.

Testování je rozděleno na testování *TCP* módu a *UDP* módu jednotlivě.

#### TCP
Success testy:
1. První test kontroluje, zda se uživatel dokáže úspěšně autorizovat na serveru.
2. Druhý test kontroluje, zda se uživatel dokáže úspěšně autorizovat na serveru a následně se připojit do jiného kanálu.
* Očekávaným výstupem testů je výstup začínající *Success*.

Err testy:
1. První test se snaží odhalit chybu v zadaném počtu parametrů příkazu
2. Druhý test kontroluje výstup programu, pokud se uživatel chtěl připojit do jiného kanálu ještě než byl úspěšně autentifikován.
3. Třetí test kontroluje výstup programu při použití neznámého příkazu.
* Očekávaným výstupem testů je výstup začínající *ERR*.

Msg testy:
1. Oba testy kontrolují, zda se uživatel dokáže úspěšně autorizovat na serveru a odeslat zprávu.
* Očekávaným výstupem testů je výstup začínající *Success* a následně další příjatá zprává začínající *Server*, značící jméno odesílatele přijaté zprávy.

Regex testy:
1. První test se snaží odhalit chybu v parametru *username* příkazů *auth* podle odpovídajícího regulárního výrazu.
2. Druhý test se snaží odhalit chybu v parametru *username* příkazů *auth* podle odpovídajícího regulárního výrazu.
3. Třetí test se snaží odhalit chybu v parametru *DisplayName* příkazů *auth* podle odpovídajícího regulárního výrazu.
* Očekávaným výstupem testů je výstup začínající *ERR*.

#### UDP
Err testy:
1. První test se snaží odhalit chybu v zadaném počtu parametrů příkazu
2. Druhý test kontroluje výstup programu, pokud se uživatel chtěl připojit do jiného kanálu ještě než byl úspěšně autentifikován.
3. Třetí test kontroluje výstup programu při použití neznámého příkazu.
* Očekávaným výstupem testů je výstup začínající *ERR*.

Regex testy:
1. První test se snaží odhalit chybu v parametru *username* příkazů *auth* podle odpovídajícího regulárního výrazu.
2. Druhý test se snaží odhalit chybu v parametru *username* příkazů *auth* podle odpovídajícího regulárního výrazu.
3. Třetí test se snaží odhalit chybu v parametru *DisplayName* příkazů *auth* podle odpovídajícího regulárního výrazu.
* Očekávaným výstupem testů je výstup začínající *ERR*.

![tests_output](/tests/testing.png)

Finální verze aplikace byla rovněž testováná na referenčním serveru *anton5.fit.vutbr.cz*.
Pro vývoj byl použit program *Wireshark*[8], a program *netcat*[9]. Podle výstupu programu *Wireshark* se kontroloval formát zasílaných a přijatých *packetů*.

## Zdroje
[1] IPK24-CHAT https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Project%201

[2] TCP[RFC 793] https://www.ietf.org/rfc/rfc0793.txt

[3] UDP[RFC 768] https://www.ietf.org/rfc/rfc768.txt

[4] IPK24-CHAT příkazy https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Project%201#user-content-client-behaviour-input-and-commands

[5] Zprávy v binární podobě https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Project%201#user-content-message-contents

[6] Funkce select() https://man7.org/linux/man-pages/man2/select.2.html

[7] Funkce signal() https://man7.org/linux/man-pages/man2/signal.2.html

[8] Wireshark https://www.wireshark.org

[9] Netcat https://linux.die.net/man/1/nc