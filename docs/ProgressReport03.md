# Haladási napló: Harmadik bejegyzés
## Az AST dizájn és problémái
A fordítás egyik legfontosabb struktúrája kétségkívül az [absztrakt szintaxisfa](https://en.wikipedia.org/wiki/Abstract_syntax_tree), vagy röviden AST (Abstract Syntax Tree). Az elemző a nyelvtani szabályok alapján állítja elő*, tükrözi a nyelv struktúráltságát. Mint ahogyan neve is sugallja, egy fa struktúráról van szó.

*: Néha az elemző egy, az AST-hez nagyon hasonló struktúrát állít elő, melyet [konkrét szintaxisfának](https://en.wikipedia.org/wiki/Parse_tree) hívunk - angolul Parse Tree. Ez tartalmazhat esetleges köztes csomópontokat illetve fölösleges terminálisokat, de ezeket leszámítva AST-nek tekinthetjük.

A klasszikus AST csomópontjai nyelvtani szabályok - például elágazási szabály -, levelei pedig a lényeges terminálisok a forrásban. Ilyen egy szám, egy azonosító vagy egyéb literál. Példaképp, egy kód és a lehetséges hozzá tartozó AST:
```
if (x > y) {
    y += 5;
}
```
![A fenti kód szintaxisfája](./res/ASTExample.svg)

Amint elkészült, a szintaxisfa több átalakításon is keresztülmehet, egészen az IR kód generálásáig ezzel dolgozik a fordító. Éppen emiatt fontos, hogy a struktúra időtálló legyen, hiszen egy változtatás több lépést is érinthet egyszerre.

A lépések alatt történhet szintaktikai átalakítás, mikoris egy csomópontot lecserélünk egy vagy több másikra - ilyen például a [desugaring](https://en.wikipedia.org/wiki/Syntactic_sugar) -, de fűzhetünk szemantikai információt a levelekhez/csomópontokhoz - a teljes szemantikai elemzés ezt csinálja.

**Fontos kérdés:** Hogyan ábrázoljuk a szükséges struktúrákat úgy, hogy tükrözze a fordítási folyamat állását, könnyen bővíthető legyen, állja a változást és még a használata is kényelmes legyen?

A meglévő probléma - egyelőre még csak elvi kérdés - mellé még bejön az, hogy a diagnosztikai üzenetekhez pozíciókat is kell tárolnunk a levelekhez. Viszont nem bízhatunk abban, hogy pozícionális információ mindenhol rendelkezésre fog állni, az injektált elemeknek például nem is lehet - tipikusan metaprogramozásból eredő elemek.

A GHC kapcsán már foglalkoztak a fa struktúrák - különösképp az AST - bővíthetőségének problémájával egy [publikációban](https://www.microsoft.com/en-us/research/uploads/prod/2016/11/trees-that-grow.pdf).

## Elemzés
Az AST előállításához szükséges egy elemzési "taktika". A (program)nyelvészet kezdete óta rengeteg módszert és automatizáló eszközt dolgoztak ki. Már a 70-es évek elején megjelent [Yacc](https://en.wikipedia.org/wiki/Yacc) neve - Yet Another Compiler-Compiler - is azt sugallja, hogy a nyelvi elemzés gyökerei mélyre nyúlnak, közel sem új probléma.

Mivel szeretnénk maximalizálni a visszajelzés minőségét, ezért egy olyan módszert kell válasszunk, melyet könnyen írhatunk kézzel a nyelvtanból. Ehhez tökéletes az úgynevezett [recursive descent parser](https://en.wikipedia.org/wiki/Recursive_descent_parser). Lényegében a nemterminálisok függvények lesznek, a terminálisok pedig léptetés a token bemenetben. Hátránya, hogy a [balrekurziót manuálisan eliminálni kell](https://en.wikipedia.org/wiki/Left_recursion), ha matematikai kifejezéseket akarunk elemezni.

## Hibák, visszaállítás
Ha elemzés közben nem várt tokenhez érkezünk, hibát jelzünk. De egy fontos kérdés, hogy hogyan folytassuk ez után? Egy jó nyelvi eszköztől elvárjuk hogy egynél több hibát jelezzen egyszerre, de egy felhasználói hibáról csak egyetlen releváns értesítést kapjunk - tehát a hibák ne eszkalálódjanak. Még jobb lenne, ha a fordítás ennek ellenére folytatódhatna, és a szintaktikai hibák ellenére szemantikai visszajelzést is kaphatnánk.

### Az egyszerű út
Egy viszonylag egyszerű módszer, ha választunk néhány tokent, melyek garantálják, hogy viszonylag "értelmes" folytatási pontok, ha hibába ütköztünk. Ez esetünkben lehet például az `fn` kulcsszó. Ennek hátránya hogy néha sok elemet hagy ki a fából, ezt a módszert használva nem lehet megbízhatóan folytatni a szemantikai analízissel.

Vannak tokenek, melyek csak megszokásból - és a szimmetria miatt - vannak jelen a nyelvekben. Ezek egyszerűen kihagyhatőak az elemzés során. Bár az AST emiatt még teljes lehet, értesítjük a felhasználót a hibáról. Példa:
```
fn foo( -> i32 {
   // Function body
}
```
A nyelvtanból egyértelmű, hogy a `->` már a paraméterlista végét jelzi, a lezáró zárójel csupán megszokás miatt irandó ki. Sajnos a legtöbb hiba nincs ilyen egyszerű helyzetben a nyelvtan szempontjából.

### A nehéz út
Rengeteg elmélet van a parser error recovery és részleges szintaxisfák építése mögött. Sajnos még nem volt időm belenézni ezekbe. A cél, hogy az AST tükrözze az összes helyes és értelmes bemenetet, az elemző pedig próbáljon értelmes helyettesítést adni a hibás részre. A [Futhark](https://futhark-lang.org/) programnyelv készítőjével beszélve arra jutottam, hogy: _"It's 50% heuristics and 50% magic :)"_.

## Változtatások reflektálása
Ha változik a bemenet, a teljes forrás újraelemzése igencsak pazarló, egy nagyobb file-nál pedig kifejezetten lassú lehet. Tovább bonyolítja a dolgot, hogy írás közben az elemző hibának vehet egy konstrukciót, mely abban a pillanatban még tényleg hibának tűnik, azonban mindössze még nem írtuk végig az adott elemet. Az elemzőket, melyek csupán a változás alapján képesek módosítani az eredményüket inkrementális elemzőknek hívjuk. Ugyanúgy mint hibakezelésnél, rengeteg és bonyolult elmélet tartozik hozzá, szintén viszonylag ismeretlen számomra a terület. Az úgynevezett [shift-reduce elemzők](https://en.wikipedia.org/wiki/Shift-reduce_parser)nél például a shift és reduce műveletek mellé betársul egy unshift és unreduce művelet.

## Összefoglalás
Ez az írás inkább egy keret volt ahhoz, amit utoljára implementáltam a nyelvi szerverben - AST, elemző, szintaktikai hiba. Több dolgot is említettem, melyeket érdemes jobban csinálni mint a triviális megoldás, de még sosem érintettem a témát. A jövőben szeretném ezeket mélyebben megismerni és implementálni ebbe a projektbe.

A projekt átment egy protokoll megismeréséből abba, hogy hogyan tudunk egy klasszikus compilert service-é tenni - Compiler as a Service, CaaS -, melyet már a [Roslyn](https://github.com/dotnet/roslyn) projekt sikeresen meg is tett. Szerintem ez egy fontos "jellemfejlődés" volt. Egy protokoll csupán megszabott formátum, de az igazi probléma nem itt rejtőzött, hanem hogy hogyan tudjuk újrahasznosítani azokat a képességeket, melyek már léteznek egy fordítóban. Ez nem csak egy nyelvi szerverre érvényes. Egy jó CaaS képes ellátni minden fontosabb nyelvi feladatot, mint egy [linter](https://en.wikipedia.org/wiki/Lint_(software)), auto-formatter, stb.

### A továbbiak
A projektet szeretném tovább vinni a tanulmányaimban és egyszer egy teljes compiler infrastruktúrát kihozni belőle. A front-endből hátra van még a szemantikai elemzés, mely egy igen nagy és érdekes terület. A front-end után pedig generálhatunk egy absztrakt gépi kódot, melyet optimalizálhatunk és tovább fordíthatunk gépi kódra.

### Források
Nyilvánvaló, hogy itt nem a saját tollammal ékeskedtem, hanem rengeteg okos ember kemény munkájával. Forrásaim:
- Melvin Smith, aki megtanított a fordítók alapjaira
- Az [ANTLR](https://www.antlr.org/) mögötti zseni, Terence Parr könyve: [Language Implementation Patterns](https://pragprog.com/book/tpdsl/language-implementation-patterns)
- Az [r/ProgrammingLanguages](https://old.reddit.com/r/ProgrammingLanguages/) subreddit, ahol rengeteg hozzám hasonló, de nálam jóval tapasztaltabb nyelvésszel tudtam beszélni