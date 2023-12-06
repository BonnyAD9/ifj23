write("Enter a string: ")
let str = readString()!

write("Enter a number: ")
let num = readInt()!

let res = caesar(of: str, with: num)

write(res, "\n")

func caesar(of str: String, with shift: Int) -> String {
    let len = length(str)
    var res = ""
    var i = 0
    while i < len {
        let next_i = i + 1
        let c = substring(of: str, startingAt: i, endingBefore: next_i)!
        let num = ord(c) + shift
        res = res + chr(num)
        i = next_i
    }
    return res
}
