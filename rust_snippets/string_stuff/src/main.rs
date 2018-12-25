
static change_value : &'static str = "!(changed)";

fn main() {
    let mut my_s = String::from("Das ist ein TestString");

    println!("1.simply output the string slice");
    my_print_string(&my_s);

    println!("1. change string and output");
    my_change_string(&mut my_s);
    my_print_string(&my_s);
}


fn my_print_string(s: &str) {
    println!("  Print string slice \"{}\"", s);
}

fn my_change_string(s: &mut String) {
    s.push(' ');
    s.push_str(change_value);
}