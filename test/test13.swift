write("Enter number: ")

let a : Int? = readInt()

if let a {
    if a < 10 {
        write("The number is less than 10.\n")
    } else {
        write("The number is at least 10\n")
    }
} else {
    write("Failed to read a number\n")
}
