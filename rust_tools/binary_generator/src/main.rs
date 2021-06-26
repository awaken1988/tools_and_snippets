#[allow(warnings)]
#[allow(bindings_with_variant_name)]
#[allow(unused_variables)]

use std::io::Read;
use std::io::Write;
use std::fs::File;
use std::io::BufWriter;
use std::time::{Duration, Instant};
use clap::{App,Arg,Subcommand};
use std::collections::HashMap;

mod generator; 

fn main() -> std::io::Result<()> {
    let generators = generator::get_all();

    let mut arg_checker = clap::App::new("binary_generator")
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
        );

    for (iGenName, iGen) in generators.iter() {
        let mut sub = App::new(iGenName);
        
        for (iArgName, iArg) in iGen.arguments.iter() {
            sub = sub.arg(
              Arg::new(*iArgName)
              .long(*iArgName)
              .takes_value(true)
              .clone()
          );
        }

        arg_checker = arg_checker.subcommand(sub);
    }
        
    
    let args = arg_checker.get_matches();
    
    if let Some(generator_builder) = generators.get( args.value_of("generator").unwrap() ) {
        let mut generator_arg: HashMap<String,String> = HashMap::new();

        if let Some(sub_args) = args.subcommand_matches(&generator_builder.name) {
            for (iArgName, iArg) in &generator_builder.arguments {

                if let Some(arg) = sub_args.value_of(iArgName) {
                    println!("arg added {}", iArgName);
                    generator_arg.insert(iArgName.to_string(), arg.to_string());
                }
            }
        }
 
        let mut generator = (generator_builder.generator)(&generator_arg);
        


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
