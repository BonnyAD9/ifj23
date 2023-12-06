/*
 * Calculates result using given operation and numbers
 */
func calc(x x: Int, y sec: Int, _ op: String) -> Int? {
    if op == "+" {
        return x + sec
    }
    if op == "-" {
        return x - sec
    }
    if op == "*" {
        return x * sec
    }
    if op == "/" {
        return x / sec
    }
    return nil
}

/*
 * Gets number from the input
 */
func get_num() -> Int {
    write("Enter a number:\n")
    var num = readInt()

    // This makes sure that number is not nill
    while num == nil {
        write("Invalid number given, try again:\n")
        num = readInt()
    }

    return num!
}

/*
 * Gets operator from the input
 */
func get_op() -> String {
    write("Enter an operator:\n")
    var op = readString()

    while 0 == 0 {
        if op == nil {
            write("Error loading operation, try again: \n")
            op = readString()
        } else {
            let op_val = op!
            if op_val == "+" {
                return op_val
            }
            if op_val == "-" {
                return op_val
            }
            if op_val == "*" {
                return op_val
            }
            if op_val == "/" {
                return op_val
            }
            write("Invalid operation given, try again: \n")
            op = readString()
        }
    }
    return ""
}

let x = get_num()
let op = get_op()
let y = get_num()

let res = calc(x: x, y: y, op)
if let res {
    write("Result: ", res, "\n")
} else {
    write("Error in calculating expression\n ")
}
