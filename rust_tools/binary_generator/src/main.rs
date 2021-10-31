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

    //create args for each generator
    let mut arg_checker = clap::App::new("binary_generator")
        .version("0.1")
        .author("Martin K.")
        .about("generate binary files")
        .arg(clap::Arg::new("out")
            .long("out")
            .about("generated output file")
            .takes_value(true)
            .required(true))
        .arg(clap::Arg::new("size")
            .long("size")
            .about("size of the generated output")
            .takes_value(true)
            .required(true)
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
    
    //get the generator
    let mut generator = {
        if let Some(subcmd_name) = args.subcommand_name() {
            let mut generator_builder = generators.get(subcmd_name).unwrap();
            let mut generator_arg: HashMap<String,String> = HashMap::new();
    
    
            if let Some(sub_args) = args.subcommand_matches(&generator_builder.name) {
                for (iArgName, iArg) in &generator_builder.arguments {
    
                    if let Some(arg) = sub_args.value_of(iArgName) {
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
    let out_size = (args.value_of("size").unwrap()).parse::<usize>().unwrap();

    //write to file
    let file = File::create(args.value_of("out").unwrap())?;
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
