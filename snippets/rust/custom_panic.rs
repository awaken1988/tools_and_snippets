#![allow(unused)]
fn main() {

use std::panic;

panic::set_hook(Box::new(|panic_info| {
    if let Some(s) = panic_info.payload().downcast_ref::<String>() {
        println!("{}", s);
    }
    else if let Some(s) = panic_info.payload().downcast_ref::<&str>() {
        println!("{}", s);
    } else {
        println!("unknown panic");
    }
}));

//panic!("Normal panic");

let x: Option<usize> = None;

x.expect("whooops");



}
