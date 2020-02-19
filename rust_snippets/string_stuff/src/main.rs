
static change_value : &'static str = " (changed &'static str)";

fn main() {
    let mut my_s = String::from("This      is a String in Rust");
    let mut my_s: String = "This is a String in Rust".into();
    let mut my_s = "This is a String in Rust".to_string();

    my_print_headline("1.simply output the string slice");
    my_print_string(&my_s);

    my_print_headline("2. change string and output");
    my_change_string(&mut my_s);
    my_print_string(&my_s);

    my_print_headline("3. split by whitespace");
    for iWs in my_s.split_whitespace() {
        println!("  {}", iWs)
    }
}

fn my_print_headline(s: &str) {
    let len = s.chars().count();

    fn my_print_fill(c: usize) {
        for i in 0..c {
            print!("-");
        }
        println!("");
    }

    my_print_fill(len);
    println!("{}", s);
    my_print_fill(len);
}

fn my_print_string(s: &str) {
    println!("  Print string slice \"{}\"", s);
}

fn my_change_string(s: &mut String) {
    s.push(' ');
    s.push_str(" (push_str)");
    s.push_str(change_value);
}