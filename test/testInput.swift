let a=a??0

5*(5+i)

// This was once issue while parsing
let i = 5-(i+5)

func concat(_ x : String, with y : String) -> String {
    let x = x + y
    return x + " " + y
}

let a = "ahoj "
var ct : String
ct = concat(a, with: "svete")
write(ct, a)

write("Ahoj\n\"Sve'te \\\u{22}")

let inp : Int? = readInt()
// pomocna funkce pro dekrementaci celeho cisla o zadane cislo
func decrement(of n: Int, by m: Int) -> Int {
    return n - m
}

func factorial(_ n : Int /* comments anywhere */) -> Int {
    var result : Int?
    if (n < 2) {
        result = 2.5E5
    } else {
        let decremented_n = decrement(of: n, by: 1)
        let temp_result = factorial(decremented_n)
        result = n * temp_result
    }
    return result!
}

if let inp {
    if (inp < 0) {
        write("Faktorial nelze spocitat!")
    } else {
        let vysl = factorial(inp)
        write("Vysledek je: ", vysl)
    }
} else {
    write("Chyba pri nacitani celeho cisla!")
}