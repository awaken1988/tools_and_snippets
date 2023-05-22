#[allow(warnings)]
#[allow(bindings_with_variant_name)]
#[allow(unused_variables)]

use std::io::Read;
use std::io::Write;
use std::fs::File;
use std::io::BufWriter;
use std::time::{Duration, Instant};
use clap::{Command,Arg,Subcommand};
use std::collections::HashMap;

#[macro_use]
extern crate lazy_static;

mod generator; 

lazy_static! {
    static ref GENERATORS: HashMap<String,generator::defs::GeneratorBuilder> = generator::get_all();
}

fn main() -> std::io::Result<()> {
    //common generator args
    let mut arg_checker = Command::new("binary_generator")
        .version("0.1")
        .author("Martin K.")
        .about("generate binary files")
        .arg(Arg::new("out")
            .long("out")
            .required(true))
        .arg(clap::Arg::new("size")
            .long("size")
            .required(true)
    );

    //subcommands for each generator
    let ref generators = *GENERATORS;
    for (iGenName, iGen) in generators {
        let mut sub = Command::new(iGenName.as_str());
        for (iArgName, iArg) in iGen.arguments.iter() {
            sub = sub.arg(
              Arg::new(*iArgName)
              .long(*iArgName)
          );
        }
        arg_checker = arg_checker.subcommand(sub);
    }
        
    let args = arg_checker.get_matches();
    
    //get the generator
    let mut generator = {
        if let Some(subcmd_name) = args.subcommand_name() {
            let mut generator_builder = generators.get(subcmd_name).unwrap();
            let mut generator_arg: HashMap<String,String> = HashMap::new();
    
    
            if let Some(sub_args) = args.subcommand_matches(&generator_builder.name) {
                for (iArgName, iArg) in &generator_builder.arguments {
    
                    if let Some(arg) = sub_args.get_one::<String>(iArgName) {
                        println!("arg added {}", iArgName);
                        generator_arg.insert(iArgName.to_string(), arg.to_string());
                    }
                }
            }

            (generator_builder.generator)(&generator_arg)
        }
        else {
            panic!("no subcommand choosen");
        }
    };
    
    //out size
    let out_size = {
        let size_str = args.get_one::<String>("size")
            .expect("no --size given")
            .to_lowercase();
        let radix_skip = {
            if size_str.starts_with("0x") { (16, 2) }
            else { (10,0) }
        };
        usize::from_str_radix(&size_str[radix_skip.1..], radix_skip.0).expect("cannot parse --size")
    }; 

    //write to file
    let file = File::create(args.get_one::<String>("out").unwrap())?;
    let mut  buffered_file = BufWriter::new(file);
    
    let stopwatch = Instant::now();
    for i in 0..out_size {
        let mut out = [0;1];
        generator.read(&mut out);
    
        buffered_file.write(&out).unwrap();
    
        //println!("0x{:x}", out[0] )
    }
    println!("File Generated in {} milliseconds", stopwatch.elapsed().as_millis());

    return Ok(());
}
