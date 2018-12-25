#![allow(dead_code)]
#![allow(unused_variables)]
#![allow(unused_import)]

use std::fmt;

#[derive(Debug)]
enum State {
    Success,
    InProgress(u32),
    Error{num: u32, desc: String},
    FatalError,
}

fn u32_to_state(st: u32) -> Option<State>
{
    match st {
        0 => Some(State::Success),
        1 => Some(State::InProgress(0)),
        2 => Some(State::Error{num: 0, desc: "bla_v2".to_string()}),
        _ => None,
    }
}

fn get_state(num: u32) -> State 
{
    match num {
        0 => State::Success,
        1 => State::InProgress(1),
        2 => State::Error{num: 1, desc: "bla".to_string()},
        _ => State::FatalError,
    }
}

fn main() {
    println!("Hello, world!");

    let min: u32 = 0;
    let max: u32 = 10;

    println!("--- 1. ---");
    for i in min..max {
        let x =get_state(i);
        println!("    {:?}", x);
    }

    println!("--- 2. ---");
    for i in min..max {
        let x =u32_to_state(i);
        println!("    {:?}", x);
    }
    
}
