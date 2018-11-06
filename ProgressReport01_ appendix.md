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