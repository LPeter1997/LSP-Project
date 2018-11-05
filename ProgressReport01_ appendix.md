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
