func concat(_ x : String, with y : String) -> String {
    var x = x + y
    return x
}

let str = concat("ahoj ", with: "ifj")

write(str, "\n")
