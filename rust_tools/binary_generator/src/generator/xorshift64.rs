use std::io::Read;
use std::collections::VecDeque;

use crate::generator::defs::GeneratorBuilder;

struct Xorshift64
{
    state: u64,
    buff:  VecDeque<u8>,
}

impl Xorshift64 {
    fn new() -> Xorshift64 {
        return Xorshift64 {
            state: 88172645463325252,
            buff:  VecDeque::<u8>::new(),
        }
    }

    fn next(&mut self) -> u8 {
        if self.buff.len() < 1 {
            self.state ^= self.state << 13;
            self.state ^= self.state >> 7;
            self.state ^= self.state << 17;

            for i in self.state.to_le_bytes().iter() {
                self.buff.push_front(*i);
            }
        }

        self.buff.pop_back().unwrap()
    }
}

impl std::io::Read for Xorshift64
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
    return GeneratorBuilder {
        name: String::from("xorshift64"),
        generator: || {
            return Box::new( Xorshift64::new() )
        },
    }
}