use std::{
    net::{UdpSocket, SocketAddr}, 
    collections::{HashMap, HashSet}, 
    sync::{mpsc::channel, Mutex, Arc},
    thread, time::{Duration, Instant}, path::PathBuf, fmt::write};
use clap::{Command, Arg, builder::PossibleValue};

mod connection;
mod defs;
mod protcol;
use defs::{ClientState, ServerSettings, WriteMode, FileLockMode};

#[macro_use(defer)] 
extern crate scopeguard;


const CLEANUP_TIMEOUT: Duration = Duration::from_secs(3);

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

    //TODO: there is a more elegant way with clap; but for now simple redundant strings used
    let writemode = args.get_one::<String>("writemode").unwrap();
    let writemode = match writemode.as_str() {
        "disabled" => WriteMode::Disabled,
        "new" => WriteMode::WriteNew,
        "overwrite" => WriteMode::WriteOverwrite,
        other => panic!("writemode {} does not exist", other),
    };

    //TODO: let verbose = args.get_one::<bool>("verbose").unwrap();
   

    let rootdir = args.get_one::<String>("rootdir").unwrap();

    if !std::path::Path::new(rootdir).is_dir(){
        panic!("rootdir = \"{}\" does not exists", rootdir);
    }
 
    let settings = ServerSettings {
        write_mode: writemode,
        root_dir:   rootdir.clone(),
        blocksize:  protcol::DEFAULT_BLOCKSIZE,
        verbose:    true, 
    };

    run_server(settings);
}

fn run_server(settings: ServerSettings) {
    let socket = UdpSocket::bind("127.0.0.1:69").unwrap();
    let _ = socket.set_read_timeout(Some(Duration::from_secs(1)));  //TODO: check for error
    let mut connections = HashMap::<SocketAddr,ClientState>::new();
    let mut cleanpup_stopwatch = Instant::now();

    let files_locked = Arc::new(Mutex::new(HashMap::<PathBuf,FileLockMode>::new()));

    let mut buf = Vec::<u8>::new();

    loop {
        buf.resize(protcol::MAX_PACKET_SIZE, 0);

        let (amt, src) = match socket.recv_from(&mut buf) {
            Ok((size,socket)) => (size,socket),
            Err(_) => {
                cleanup_connections(&mut connections, &mut cleanpup_stopwatch);
                continue;
            }
        };

        buf.resize(amt, 0);
    
        if connections.contains_key(&src) {
            let _ = connections.get(&src).unwrap().tx.send(buf.clone());
        }
        else {
            let (sender, receiver) = channel();

            let mut client_state = ClientState {
                tx: sender,
                join_handle: Option::None,
            };

            let remote = src;
            
            let socket = socket.try_clone().unwrap();
            let settings = settings.clone();
            let files_locked = files_locked.clone();
            client_state.join_handle = Some(thread::spawn(move|| {
                connection::Connection::new(
                    receiver, 
                    remote,
                    socket,
                    settings,
                    files_locked).run();
            }));


            connections.insert(src,client_state);
            let _ = connections.get(&src).unwrap().tx.send(buf.clone());
        }

        //cleanup
        cleanup_connections(&mut connections, &mut cleanpup_stopwatch);
    }
}

fn cleanup_connections(connections: &mut HashMap::<SocketAddr,ClientState>, stopwatch: &mut Instant) {
    if stopwatch.elapsed() < CLEANUP_TIMEOUT {
        return;
    }

    let mut todo_delete: Vec<SocketAddr> = vec![];
    for i_con in connections.iter_mut() {
        if i_con.1.join_handle.as_mut().unwrap().is_finished() {
            todo_delete.push(*i_con.0);
        }
    }
    for i_con in todo_delete {
        let state = connections.remove(&i_con).unwrap();
        println!("INFO: {:?} quit", i_con);
        let _ = state.join_handle.unwrap().join();
    }


    *stopwatch = Instant::now();
}
