#[allow(warnings)]
#[allow(bindings_with_variant_name)]
#[allow(unused_variables)]

use std::io::Read;
use std::io::Write;
use std::fs::File;
use std::io::BufWriter;
use std::time::{Duration, Instant};
use clap;

mod generator; 

fn main() -> std::io::Result<()> {
    let generators = generator::get_all();

    let args = clap::App::new("binary_generator")
        .version("0.1")
        .author("Martin K.")
        .about("generate binary files")
        .arg(clap::Arg::new("out")
            .long("out")
            .about("generated output file")
            .takes_value(true)
            .required(true))
        .arg(clap::Arg::new("generator") 
            .long("generator")
            .about("name of the generator")
            .takes_value(true)
            .required(true)
            .possible_values(&["xorshift64"])
        )
        .get_matches();
        
    if let Some(generator_builder) = generators.get( args.value_of("generator").unwrap() ) {
        let mut generator = (generator_builder.generator)();
        let file = File::create(args.value_of("out").unwrap())?;
        let mut  buffered_file = BufWriter::new(file);
        
        let stopwatch = Instant::now();
        for i in 0..(1*1024*1024) {
            let mut out = [0;1];
            generator.read(&mut out);
        
            buffered_file.write(&out).unwrap();
        
            //println!("0x{:x}", out[0] )
        }
        println!("File Generated in {} milliseconds", stopwatch.elapsed().as_millis());
    }
    else {
        println!("Unknown Generator \"{}\"", args.value_of("generator").unwrap());
    }

   
 

    

    return Ok(());
}
