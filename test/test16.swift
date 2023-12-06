func factorial(_ num: Int) -> Int {
    if num < 2 {
        return 1
    }
    let n = num - 1
    return factorial(n) * num
}

let i = factorial(7)

write(i, "\n")
