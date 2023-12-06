func repeat(_ str: String, _ n: Int) -> String {
    var i = 0
    var res = ""
    while i < n {
        res = res + str
        i = i + 1
    }
    return res
}

let res = repeat("knock ", 5)
write(res, "\n")
