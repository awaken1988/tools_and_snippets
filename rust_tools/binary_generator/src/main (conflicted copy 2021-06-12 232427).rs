#[allow(warnings)]
#[allow(bindings_with_variant_name)]
#[allow(unused_variables)]

use std::collections::VecDeque;
use std::io::Read;
use std::io::Write;
use std::fs::File;
use std::io::BufWriter;
use std::time::{Duration, Instant};
use clap;

trait Generator {
    fn name() -> String;
}

fn get_xorshift64() -> Box<dyn Generator> {
    Box::new( Xorshift64::new() )
}

struct Xorshift64
{
    state: u64,
    buff:  VecDeque<u8>,
}

impl Generator for Xorshift64 {
    fn name() -> String {
        String::from("Xorshift64")
    }
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

enum GeneratorType
{
    Xorshift64,
}

fn get_generator(gen: GeneratorType) -> Box<dyn std::io::Read>
{
    match gen {
        Xorshift64 => Box::new( Xorshift64::new() )
    }
}


fn main() -> std::io::Result<()> {
    let args = clap::App::new("binary_generator")
        .version("0.1")
        .author("Martin K.")
        .about("generate binary files")
        .arg(clap::Arg::new("out")
            .long("out")
            .about("generated output file")
            .takes_value(true)
            .required(true))
        .get_matches();


    let mut generator = Xorshift64::new();
    let file = File::create(args.value_of("out").unwrap())?;
    let mut  buffered_file = BufWriter::new(file);

    let stopwatch = Instant::now();
    for i in 0..(1*1024*1024) {
        let mut out = [0;1];
        generator.read(&mut out);

        buffered_file.write(&out).unwrap();


        //println!("0x{:x}", out[0] )
    }

    println!("{}", stopwatch.elapsed().as_millis());

    return Ok(());
}
