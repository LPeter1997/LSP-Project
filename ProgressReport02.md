
# Haladási napló: Második bejegyzés
## Célmódosítás
Előzőleg sikeresen félrenéztem az LSP képességeit, és azt hittem lehetséges már a syntax highlight tisztán a protokollból. Mint kiderült, ez még mindössze egy [proposal](https://github.com/Microsoft/language-server-protocol/pull/124). Így ehelyett első feladatomnak brace párok kiemelését tűztem ki, ezután pedig egy komolyabb fícsört (változhat, de szeretnék syntax error report-ot és go to definition-t). Hogy min múlik mi lesz az, később kifejtem.

## Compiler design 101
Mielőtt interakciókat tudnánk leírni a kliens és szerver között, meg kell nézzük hogy hogyan épül fel a szerver legfontosabb eleme, a fordító.

### A klasszikus fordítási modell
Néhány extrémet leszámítva a mai fordítók nagyjából ugyanazt a fordítási modellt követik:
![A fordítás folyamata](./CompilationProcess.svg)
**Note:** _Sokszor nem különböztetnek meg Parse Tree-t és AST-t, mert egy és ugyanazon struktúrát használják az implementációnál. Elviekben a különbség annyi, hogy az AST nem tartalmaz fölösleges szintaktikai elemeket és szemantikai információt hordoz, például referenciákat szimbólumokhoz._

Manapság divat a backend részleget egy létező megoldásra, leggyakoribb esetben például az [LLVM](https://llvm.org/)-re hagyni.

Ideális esetben a fenti pipeline folyamatok teljesen elkülönülnek, nem hatnak egymásra. A gyakorlatban néhány nyelv ennél bonyolultabb, a folyamatok visszahatnak korábbi folyamatokra - például a [C lexer hack](https://en.wikipedia.org/wiki/The_lexer_hack).

A C++ például odáig fokozza a visszahatást korábbi folyamatokra, hogy a [fordítási folyamat leállása](http://blog.reverberate.org/2013/08/parsing-c-is-literally-undecidable.html) ekvivalens a [leállási problémával](https://en.wikipedia.org/wiki/Halting_problem). Természetesen ez csak egy elvi limitáció, a gyakorlatban korlátok vannak bevezetve, mint például template példányosítási mélységhatár.

A mi esetünkben - a metaprogramozási képességeket leszámítva - a folyamatokat teljesen el tudjuk különíteni.

### Probléma a klasszikus modellel
A legtöbb nyelvi segédeszköznek - mint ami az editorokban is van - a fordító front-endjét kell használniuk, a felhasználói hibák a legtöbb esetben itt történnek. Azonban ha minden egyes karakter változására lefutna a teljes folyamat, az nagyon lassú és pazarló volna. Szükséges, hogy a front-end képes legyen csak a változásokra reflektálni.

Az egyszerűség kedvéért ezt a részét mellőzöm a projektnek, de megemlítek módszert mely alkalmazható.

A klasszikus fordítókat feketedobozként írták. Ez jó volt akkor, amikor ténylegesen csak parancssori alkalmazásokként használták őket, azonban - mint előbb láthattuk - megszületett az igény, hogy a front-end aktív kommunikációt végezzen egy protokollon keresztük.

## Lexikális analízis, brace matching
A brace matching-hez első lépésben elegendő egy lexikai analízist végrehajtani, és párba állítani a megfelelő brace szimbólumokat. Itt még szintaktikai elemzésre sincs szükség.

** Note:** _Ez csak akkor tehető meg, ahol a nyelv elvárja a brace-k egyensúlyát. A legtöbb nyelv szerencsére ilyen, ennek ellenére ezt a fícsört is érdemes lehet átmozgatni a Parse Tree elkészítése utánra. Most a prezentáció erejéig jó lesz a lexikai elemzés után is._

A lexikai elemzést az ún. _Lexer_ végzi. Elméletben ezt egy egyszerű állapotgép is el tudja végezni, és célja egyszerűsíteni a szintaktikai elemzést. Ignorálja a szöveg azon részeit, melyek nem fontosak a nyelvtannak (szünet, új sor, komment, stb) és normalizálja azokat a lexémákat, melyeknek akár végtelen karakteres reprezentációja lehet - mint például a számok vagy változónevek.

A lexikális analízis végcélja egy _token_ sorozat előállítása. Egy token lényegében egy lényeges, egységnyi része a nyelv nyelvtanának. Ilyen egy brace, egy név, egy zárójel, stb. A lexer implementációja:
- lexer.hpp
- lexer.cpp

