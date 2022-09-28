use std::io::Read;
use std::collections::VecDeque;
use std::collections::HashMap;
use num_bigint::BigUint;

use crate::generator::defs::GeneratorBuilder;
use crate::generator::defs::GeneratorArgument;

struct Counter
{
    width:   usize,
    buff:    VecDeque<u8>,
    counter: BigUint,
}

impl Counter {
    fn new(arg: &HashMap<String,String>) -> Counter {
        let mut start: u64 = 0;

        let width: usize = {
            if let Some(arg) = arg.get("width") {
                arg.parse::<usize>().expect("Counter: with width wrong format; must be a number")
            }
            else {
                0
            }
        };

        let counter: BigUint = {
            if let Some(arg) = arg.get("start") {
                BigUint::parse_bytes(arg.clone().into_bytes().as_slice(), 10).unwrap()
            }
            else {
                BigUint::from(0u8)
            }
        };

        return Counter {
            width:   width,
            counter: counter,
            buff:    VecDeque::<u8>::new(),
        }
    }

    fn next(&mut self) -> u8 {
        if self.buff.len() < 1 {
           let data = self.counter.to_bytes_le();
           for iByte in &data {
               self.buff.push_back(*iByte);
           }

           if data.len() < self.width {
               for i in 0..(self.width - data.len()) {
                   self.buff.push_back(0);
               }
           }

        }

        self.counter += 1u8;

        self.buff.pop_front().unwrap()
    }
}

impl std::io::Read for Counter
{
    fn read(&mut self, out: &mut [u8]) -> std::result::Result<usize, std::io::Error> 
    { 
        for i in 0..out.len() {
            out[i] = self.next();
        }

        Ok(out.len())
    }
}

pub fn get() -> GeneratorBuilder {
    let mut args = HashMap::new();

    let arg_start = GeneratorArgument::new("start");
    let arg_width = GeneratorArgument::new("width");
    let arg_incr  = GeneratorArgument::new("increment");

    args.insert(arg_start.name, arg_start);
    args.insert(arg_width.name, arg_width);
    args.insert(arg_incr.name, arg_incr);

    return GeneratorBuilder {
        name: String::from("counter"),
        generator: |arg| {
            return Box::new( Counter::new(arg) );
        },
        arguments: args,
    }
}