use std::{
    net::{UdpSocket, SocketAddr}, 
    collections::HashMap, 
    sync::mpsc::channel,
    thread, time::{Duration, Instant}};
use clap::{Command, Arg, builder::PossibleValue};

mod connection;
mod defs;
mod protcol;
use defs::{ClientState, ServerSettings, WriteMode};


const CLEANUP_TIMEOUT: Duration = Duration::from_secs(3);

//TODO:
//  - when in overwrite mode lock one instance


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
            .value_parser([PossibleValue::new("Disabled"), PossibleValue::new("New"), PossibleValue::new("Overwrite")])
            .default_value("New")
            .help("Disabled: write not possible; New: New files can be uploaded; Overwrite: overwrite existing files allowed")
        )
        .get_matches();

    //TODO: there is a more elegant way with clap; but for now simple redundant strings used
    let writemode = args.get_one::<String>("writemode").unwrap();
    let writemode = match writemode.as_str() {
        "Disabled" => WriteMode::Disabled,
        "New" => WriteMode::WriteNew,
        "Overwrite" => WriteMode::WriteOverwrite,
        other => panic!("writemode {} does not exist", other),
    };
   

    let rootdir = args.get_one::<String>("rootdir").unwrap();

    if !std::path::Path::new(rootdir).is_dir(){
        panic!("rootdir = \"{}\" does not exists", rootdir);
    }

    let mut connections = HashMap::<SocketAddr,ClientState>::new();
    let mut cleanpup_stopwatch = Instant::now();

    let socket = UdpSocket::bind("127.0.0.1:69").unwrap();
    let _ = socket.set_read_timeout(Some(Duration::from_secs(1)));  //TODO: check for error
    let settings = ServerSettings::new(rootdir.clone())
        .set_write_mode(writemode);

    loop {
        let mut buf = Vec::<u8>::new();
        buf.resize(4096, 0);

        let (amt, src) = match socket.recv_from(&mut buf) {
            Ok((size,socket)) => (size,socket),
            Err(_) => {
                cleanup_connections(&mut connections, &mut cleanpup_stopwatch);
                continue;
            }
        };

        buf.resize(amt, 0);
    
        if connections.contains_key(&src) {
            let _ = connections.get(&src).unwrap().tx.send(buf);
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
            client_state.join_handle = Some(thread::spawn(move|| {
                connection::Connection::new(
                    receiver, 
                    remote,
                    socket,
                    settings).run();
            }));


            connections.insert(src,client_state);
            let _ = connections.get(&src).unwrap().tx.send(buf);
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
