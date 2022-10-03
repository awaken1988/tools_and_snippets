#[macro_use] extern crate bitflags;


use std::path::PathBuf;
use clap;
use std::str::FromStr;

mod diff_tool;
mod reporter;
use reporter::ToolFlags;

//TODO: what exactly is &mut dyn FnMut :-O ???
//TODO: why  info: Vec<Option<Box<DirEntry>>> doesn't work
//TODO: make a global static variable of argument names
//TODO: replace unwrap with e.g error chains?

fn main() {
    
    // Specifiy arguments
    let app = clap::App::new("Multi Dir Diff")
        .version("0.0.1")
        .author("Martin K.")
        .about("Compare multiple dirs to get an overview over the changes")
        .arg( clap::Arg::with_name("format")
                .short("f")
                .long("format")
                .value_name("FORMAT")
                .help("sets the outputformat, available formats are \"html\" ")
                .required(true) )
        .arg( clap::Arg::with_name("outfile")
                .short("o")
                .long("outfile")
                .value_name("OUTFILE")
                .help("sets the output file") )
         .arg( clap::Arg::with_name("exclude_same")
                .long("exclude_same")
                .help("do not show same files/dirs in the output") )
        .arg( clap::Arg::with_name("directory")
                .short("d")
                .long("directory")
                .value_name("DIRECTORY")
                .help("specify the directories to diff")
                .multiple(true) )
        .get_matches();

    let mut toolflags = ToolFlags::NO_FLAG;

    // Check argument DIRECTORIES
    let dirs_converted: Vec<Option<PathBuf>> = {
        let dirs: Vec<&str> = app.values_of("directory").unwrap().collect();
        let mut dirs_ret: Vec<Option<PathBuf>> = Vec::new();
        for i_dir in dirs {
            dirs_ret.push( Some( PathBuf::from_str(i_dir).unwrap() ) );
        }
        dirs_ret
    };

    // Exclude same entries
    if let Some(exclude_same) = app.index_of("exclude_same") {
        toolflags |= ToolFlags::EXCLUDE_SAME;
    }

    // Output to file (Default is to console)
    let mut custom_writer = reporter::CustomWriter::new();

    if let Some(outfile) = app.value_of("outfile") {
        custom_writer.set_out_file( std::fs::File::create(outfile).unwrap()  ) ;    
    }

    let root = diff_tool::diff_dirs( &dirs_converted);

    match app.value_of("format").unwrap() {
        "html" => reporter::html(&root, &mut custom_writer, &toolflags),
        _ => panic!("FORMAT is invalid"),
    }
}



