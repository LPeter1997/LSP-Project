# Haladási napló: Első bejegyzés
## Bevezetés
Először is érdemes megértenünk, miért is jött létre a protokoll és milyen problémát kíván orvosolni.

A probléma nagyon hasonlít a fordítók és platformok viszonyához: N nyelvhez és M platformhoz - hogy teljesen lefedjük - N * M darab fordító kell. Erre nyújtott megoldást az LLVM, amely egy közös, absztrakt gépi kódról képes rengeteg különböző platformra fordítani (és nagyon sok optimalizációt végezni). Így az LLVM fordító M platformra kell fordítót írjon - egyetlen nyelvhez -, a fordító íróknak pedig elég egyetlen backend-re fordítaniuk, tehát az N * M-ből csupán N + M lesz.

A modern világban elképzelhetetlen, hogy a kódunkat ne ellenőrizné a környezetünk rögtön az írás pillanatában, illetve ne kínálna kiegészítési lehetőségeket, mikor elkezdünk egy változó- vagy függvénynevet gépelni. Mivel fejlesztőkörnyezetből sem egy van, itt is kialakult a bábeli helyzet a nyelvi segéd (általában egy plug-in) és az editor közt. Hogy N editor támogasson M nyelvet, N * M plug-in megírására lenne szükség, melyhez bejönnek további, az editor által meghatározott korlátok (milyen nyelven lehet írni a plug-int, stb). Ezt az N * M-et kívánja a [Language Server Protocol](https://microsoft.github.io/language-server-protocol/) (röviden LSP) megoldani. Mint az LLVM-nél az absztrakt gépi kód, itt is a közös üzenet forma lett a megoldás. Az LSP definiálja a különböző üzeneteket az editor és nyelvi segéd között, így egy plug-in (lásd később hogyan) használható több editorhoz, így ismét N + M lesz a megírandó szoftver szám.

## Egy LSP alapú kommunikáció felépítése
Egy LSP alapú kommunikáció 3 félből áll:
 - Editor: Maga a fejlesztőkörnyezet
 - Language Client
 - Language Server

### Language Client
Ahhoz, hogy az eltérő technológiákat áthidalják a szerkesztők, lehetőséget kell nyújtaniuk egy speciális plug-in, egy ún. nyelvi kliens (Language Client) megírására. Ezt az editor saját technológiáival kell implementáljuk és nincs más feladata, mint konfigurálni és elindítani a nyelvi szervert (Language Server). Ez általában egy 100 sor alatti kis program, Visual Studio-ban például egy Type-Script szkript, Emacs-ben pedig Emacs-LISP kód. Ezt ugyan minden editorhoz meg kell írjuk, de nagyon kicsi és általában mindenhol ugyanazt csinálja (konfigurálja hogy hogyan kommunikáljon a szerver és kliens, majd elindítja a szervert).

### Language Server
A nyelvi szerver (Language Server) egy általunk megválasztott technológiával implementálható, független a fejlesztőkörnyezettől. Itt történik a visszajelzés készítése, kódkiegészítési lista konstruálása, stb. Ehhez a nyelvi kliens minden felhasználói interakciót (szöveg változik, kurzor lépett, stb) elküld a szervernek, ahol a szerver képes erre rengeteg módon reagálni (kiegészítések ajánlása, típushiba jelzése, lexémák változása, ...). A nyelvi szerver lehet például egy fordítóprogram, melynek a hibalistáit felhasználói visszajelzésekké fordítjuk.

### A kommunikációs közeg
A szerver és a kliens közötti közegnek mindössze szöveges üzeneteket kell továbbítania a két fél között, lehetséges például IPC (Inter-Process Communication), Pipe-ok vagy egyszerű STDIO.

### Az üzenetek
Maguk a továbbított üzenetek igen egyszerűek, egy fejrészből és egy tartalmi részből állnak. A fejrészben tartalmi részre vonatkozó információk vannak. Jelenleg összesen két dolog van itt specifikálva, a tartalmi rész hossza mező (kötelező), illetve a tartalmi rész kódolása mező (opcionális). A tartalmi rész egy JSON objektum, mely a [JSON-RPC 2.0](https://www.jsonrpc.org/specification)-ban specifikált _Request_, _Response_, vagy _Notification_ lehet.

## Célkitűzés
Mivel nyelvet tervezek és implementálok, szeretnék tooling-ot faragni a nyelv köré. Sajnos egy félév nem elég egy teljeskörű nyelvi szerver kifejlesztésére, ezért valami alapvetőbb fícsör implementálását szeretném elérni. Első körben ez egy szintaxiskiemelés lenne, ami annyit jelent, hogy a nyelvi szerver kategorizálja a lexémákat az LSP által specifikált kategóriák egyikébe. Emellett sokat segíthet a zárójelpárok kiemelése, ezzel is szeretnék kísérletezni.
Célom tehát egy nyelvi szerver elkészítése, hozzá pedig egy nyelvi kliens prezentációs célból. A nyelvi szervert szeretném oly módon elkészíteni, hogy a nem-nyelvi munkákat egy újrahasználható LSP librarybe lehessen kiemelni, így ha valaki egy nyelvi szervert szeretne készíteni, nem kell a meta-munkákkal (message dispatching, JSON parsing, stb) forglalkoznia.

### A nyelv
Szeretném a nyelv egy részhalmazával kezdeni, és onnan fölfele építeni. Fontosabb elemek a lexikális analízishez:
 - Kulcsszavak
 - Egysoros kommentek
 - Többsoros rekurzív kommentek
 - Többféle zárójel: `(){}[]`
 - Literálok
 - Egyéb elválasztók

A nyelv szintaktikailag nagyon hasonlít a [Rust](https://www.rust-lang.org) programnyelvhez, szemantikailag azonban igen kevés közös van bennük:
```Rust
// Egy soros komment
/*
Több soros és /* egymásba /* ágyazható */ komment */
*/
fn sum(x: i32, y: i32) -> i32 {
	return x + y;
}
```

### Választott technológiák
#### Language Server
A nyelvi szervert (csakúgy mint a fordítót) C++17-ben írom, CMake build generator-ral. JSON elemzéshez a  [nlohmann-féle JSON library](https://github.com/nlohmann)-t használom. A lexikális és szintaktikai elemzéshez nem használok kódgeneráló eszközöket (mint Lex/Flex és Yacc/Bison, bár ismerem és használtam is már), elsősorban mert már implementálva vannak a fordítóban. A JSON-RPC-hez ugyan van library, de - az esetünkben -  3 üzenettípus megkülönböztetésére ez overkill-nek tűnt.

### Language Client
Sajnos scriptelés terén kissé hiányos a tudásom, ezért kis copy-paste kódolással sikeresen összeraktam egy Type-Script klienst [Visual Studio Code](https://code.visualstudio.com/)-hoz. Ha az időm úgy engedi, szeretnék egy régebbi editorhoz is (pl. Emacs) írni egyet, ezzel prezentálva hogy a komplex nyelvi szerver maga tényleg független az apró kliensektől.

### Kommunikációs közeg
Az egyszerűség kedvéért az STDIO-t választottam, de szeretném a megoldásomat a csatornától függetlenné tenni.

## Ami kész van
A protokoll a [specifikációban](https://microsoft.github.io/language-server-protocol/specification) definiálja a lehetséges üzenetek formáit, gyakorlatilag nagyrészt ezeket implementálom úgy, hogy a használat minél kényelmesebb legyen.
A szerver már képes feldolgozni az inicializáciálásra felszólító üzenetet, illetve erre válaszol is. A dokumentumok változtatásánál szintén érkeznek notifikációk, melyben a változott szöveg tartalma is kiolvasható.

### A szerver interfész
A nyelvi szervernek a következő interfészt kell implementálnia (bővülni fog, ez a jelenlegi állapot):
```C++
struct  langserver {
	virtual initialize_result initialize(initialize_params const&) =  0;
	virtual void on_initialized() { }
	virtual void on_text_document_opened(did_open_text_document_params const&) =  0;
	virtual void on_text_document_changed(did_change_text_document_params const&) =  0;
};
```

### JSON szerializáció és de-szerializáció
Sajnos a C++ -ból hiányzik a reflection, így a nem-típusos JSON üzeneteket manuálisan - némi automatizmussal - kell típusos C++ struktúrákká konvertálni, illetve esetlegesen visszafordítani JSON-ná. A teljes automatizmus bár lehetséges volna egy code translator-rel, de nagyon kényelmetlen lenne a használat. Egy `std::optional<bool>` például a legtöbb esetben egy default értékű `bool`-á alakítható, ezzel elkerülve a megtévesztő implicit bool konverzióját az `optional` típusnak.
Emiatt csak részleges automatizmust csináltam (Ruby scriptekkel osztály és szerializációs függvény vázakat generáltam).

### Kód szépség
A kód jelenleg közel sem olyan szép mint szeretném. A szerializációs kód valószínűleg nem is lehet szebb, igyekszem kifaktorálni később külön file-okba, így könnyebb lesz a tényleges logika refaktozálása is.
