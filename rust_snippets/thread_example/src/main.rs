use std::borrow::Borrow;
use std::ops::Add;
use std::ops::AddAssign;
use std::thread;
use std::time::Duration;
use std::vec;
use std::sync::Arc;
use std::sync::Mutex;


fn main() {


    let sum = Arc::new(Mutex::<u64>::new(0));

    let mut joinme = Vec::new();

    for i in 0..10 {
        let a = sum.clone();
        let handle = thread::spawn(move ||{
            let mut x =0;
            loop {
                x = x+1;
                //println!("Hello, world! {} {}", i, x);
                if x > 100000 {
                    break;
                }
                *a.lock()? += 1;
            }
            println!("thread ready {}", i);   
            return x;
        });
        joinme.push(handle);
    }

    for i in joinme.into_iter() {
        let sum = i.join().unwrap();
        println!("thesum {}", sum);   
    }

    println!("sum2 {}", *sum.lock().unwrap());   

}
