let a: Int? = nil

// let a=a??0

5*(5+i)

// This was once issue while parsing
let i = 5-(i+5)

func concat(_ x : String, with y : String) -> String {
    let x = x + y
    return x + " " + y
}

let tq = """
    triple quote test
    """

let b = "ahoj "
var ct : String
ct = concat(b, with: "svete")
write(ct, b)

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
        // Nested comments
        /* START
            /*BODY****A*/
        END */
        /*/*ON-LINE*/*/
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
