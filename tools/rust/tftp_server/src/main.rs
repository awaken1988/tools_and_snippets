use clap::{Command, Arg, builder::PossibleValue};

mod server;
mod protcol;

fn main()  {
    let args = Command::new("tftpserver")
        .arg(Arg::new("rootdir")
            .long("rootdir")
            .required(true)
            .help("base dir of the server")
        )
        .arg(Arg::new("writemode")
            .long("writemode")
            .required(false)
            .value_parser([PossibleValue::new("disabled"), PossibleValue::new("new"), PossibleValue::new("overwrite")])
            .default_value("new")
            .help("Disabled: write not possible; New: New files can be uploaded; Overwrite: overwrite existing files allowed")
        )
        .arg(Arg::new("verbose")
            .long("verbose")
            .short('v')
            .required(false)
            .help("print verbose messages")
            .default_value("false")
        )
        .get_matches();

    server::server_main(args);
}


