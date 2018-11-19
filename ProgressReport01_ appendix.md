# Első appendix
## Téma
Az első függelékben néhány használt programozási paradigmát, hack-et, illetve C++17 fícsört szeretnék megemlíteni, melyeket használtam az implementációnál.

## Monádok
A C++17-tel végre megkaptunk néhány régóta várt STL bővítést, köztük a Haskell-ben már jól ismert Maybe típust, mely az STL-ben az `optional` nevet kapta. Sajnos a legfontosabb műveletek (Haskell-ben a Maybe monad) nem kerültek bele - [de proposal van](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0798r0.html) -, így a használat nagyon kényelmetlen. Ezért definiáltam egy fmap és bind "keveréket", amivel megkönnyítem az `optional`-el való műveleteket. Operátornak az `operator|`-t választottam, mint ahogy sok library ahol monád interfészek vannak (például a [Ranges TS](https://en.cppreference.com/w/cpp/experimental/ranges) [implementációja](https://github.com/ericniebler/range-v3)) Használatra példa:
```c++
int add_two(int x) { return x + 2; }
int negate(int x) { return -x }
auto result = std::make_optional(1) | add_two | negate;
```

## Sum type-ok
Az [ADT](https://en.wikipedia.org/wiki/Algebraic_data_type)-k (Algebraic Data Type) egyike az ún. [sum-type](https://en.wikipedia.org/wiki/Tagged_union) vagy másnéven tagged-union. Implementációja a Boost-ban már régóta van, az STL-be csak a C++17-ben érkezett [std::variant](https://en.cppreference.com/w/cpp/utility/variant)-ként.

Vannak esetek, mikor előnyösebb típusok "összegeként" ábrázolni egy absztrakt típust minthogy öröklési hierarchiát alakítunk ki. Ilyen például az [AST](https://en.wikipedia.org/wiki/Abstract_syntax_tree) (Abstract Syntax Tree), melyet a fordítónak többször is be kell járnia aa folyamat során. Teletömni az őst virtuális [visitor](https://en.wikipedia.org/wiki/Visitor_pattern) függvényekkel, melyeket minden altípusban utána felül kell írni elég fáradságos. Új típust még viszonylag könnyű hozzáadni, de egy új vizitálást annál nehezebb. Ez a probléma analóg az ún. [Expression problémával](https://en.wikipedia.org/wiki/Expression_problem).

Az összeg típusok - bár nem oldják meg, csak megfordítják a problémát - ebben az esetben pont előnyösek lesznek, nagyon könnyű lesz visitort írni anélkül, hogy az AST csomópontokon változtatni kéne.

Az STL sajnos egy elég kényelmetlen vizitáló mechanizmust nyújt az std::visit-tel, de Vittorio Romeo [egyik előadásában](https://www.youtube.com/watch?v=mqei4JJRQ7s) prezentált egy - a funkcionális nyelveket megközelítő szintaxisú - [wrappert](https://vittorioromeo.info/index/blog/variants_lambdas_part_1.html), mely kényelmesebbé teszi a vizitálást. Példa használatra (egyszerűsített type-checker összeadásra):
```c++
struct integer {};
struct real {};
struct string {};

using type = std::variant<string, integer, real>;

type check_addition(type const& left, type const& right) {
	return match(left, right)(
		[](integer const&, integer const&) -> type { return integer{};   }, // int + int : int
		[](real const&,    real const&)    -> type { return real{};      }, // real + real : real
		[](integer const&, real const&)    -> type { return real{};      }, // int + real : real
		[](real const&,    integer const&) -> type { return real{};      }, // real + int : int
		[](string const&,  string const&)  -> type { return string{};    }, // string + string : string
		[](auto const&, auto const&)       -> type { throw type_error(); }  // Bármi más type error
	);
}
```

**Note**: _ADT-ket már C-ben is használtunk - union és egy enum tag -, de nem kaptunk típus biztonságot és automatikus vizitálást._

## Overload set-ek
Függvény overload halmazt átadni szabad (free) függvényekből nehézkes, pedig gyakran szükség lehet rá. Funktorba/objektumba emeléshez standard módszer még nincs ([proposal](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0834r0.html)). Hasonló a helyzet, ha egy template függvényt szeretnénk átadni, ugyanis specifikálni kell a típust még akkor is, ha ez a type inference miatt redundáns. Ehhez definiáltam egy `lift` makrót, mely az összes overload-ot egy funktor - a gyakorlatban lambda - alá gyűjti. Használatra példa:
```c++
int duplicate(int x) { return x * 2; }
double duplicate(double x) { return x * 2.0; }

template <typename T>
T duplicate(T const& val) {
	return val * 2.0;
}

// Használat
std::vector<std::complex<double>> nums = { ... };
std::vector<std::complex<double>> result;
std::transform(std::begin(nums), std::end(nums), std::back_inserter(result), lift(duplicate));
// lift() nélkül:
// std::transform(std::begin(nums), std::end(nums), std::back_inserter(result), duplicate<std::complex<double>>);
```

## Named Parameter Idiom
Sok nyelvben lehet a paramétereket névhez kötni pozíció helyett. Ez sok, vagy hasonló paramétereknél előnyös, hiszen nem cserélünk fel azonos típusú paramétereket:
```C#
var v1 = matrix.GetElement(3, 4); // Ez most row-major vagy column-major?
var v2 = matrix.GetElement(row: 3, column: 4);
```

Sajnos a C++ nem ad erre lehetőséget és a [proposal-t](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4172.htm) már régen elvetették C-ben gyökerező problémák miatt. Részben megoldás lehet a C++20 -os [designated initializer proposal](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0329r0.pdf), melyet a C99-es fícsör ihletett.

Az LSP sok JSON adatot kíván meg, ahol a mappelt struktúrák konstruktora helyenként akár 20 paramétert is megkíván, helyenként default-olható memberekkel. Egyszerű setterek helyett azonban úgy döntöttem, hogy a C++ egy ritkán használt mintáját próbálom ki, a - sokak által nem kedvelt - [named parameter idiom-ot](https://marcoarena.wordpress.com/2014/12/16/bring-named-parameters-in-modern-cpp/). A minta egyszerű:
```c++
struct person {
	template <typename T>
	person& name(T&& n) {
		m_Name = std::forward<T>(n);
		return *this;
	}

	template <typename T>
	person& age(T&& n) {
		m_Age = std::forward<T>(n);
		return *this;
	}

private:
	std::string m_Name;
	std::uint8_t m_Age;
};

// Használat
auto p = person()
	.name("John")
	.age(35);
```

Ehhez természetesen egy getter is szükséges, és a triviális esetekre érdemes lehet makrót írni.

**Note:** _Bár úgy tűnhet, hogy az Objektum-Orientált enkapszuláció így sérül - hisz a getter-setter pár ugyanaz, mintha publikus lenne a member -, azonban ezt a mintát csak [DTO](https://en.wikipedia.org/wiki/Data_transfer_object)-khoz (Data Transfer Object) használom, melyeknek általános megjelenése például C#-ban egy osztály csupa publikus property-vel. DTO-knál a data-class code-smell és enkapszuláció ilyen módon sérthető._