extern crate my_vector_lib;

fn main() {
    println!("Hello, world!");

    let xyz = my_vector_lib::MyVector::new();

    println!("{:?}", xyz);
    println!("{:?}", xyz.sum());
}
