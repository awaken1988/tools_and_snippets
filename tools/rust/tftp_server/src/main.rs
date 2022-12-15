use std::{
    net::{UdpSocket, SocketAddr}, 
    collections::HashMap, 
    sync::{mpsc::{Sender, channel}, Arc, Mutex},
    thread, time::{Duration, Instant}};



mod connection;
mod mydef;
mod protcol;
use mydef::ClientState;

const CLEANUP_TIMEOUT: Duration = Duration::from_secs(10);

fn main() {
    let mut connections = HashMap::<SocketAddr,ClientState>::new();
    //let root = "C:/tftp".to_string();
    let root = "/home/martin/src/".to_string();

    let mut cleanpup_stopwatch = Instant::now();


    let socket = UdpSocket::bind("127.0.0.1:69").unwrap();
    socket.set_read_timeout(Some(Duration::from_secs(1)));

    let (ctrl_sender, ctrl_recv) = channel::<SocketAddr>();

    loop {
        let mut buf = Vec::<u8>::new();
        buf.resize(4096, 0);

        let (amt, src) = match socket.recv_from(&mut buf) {
            Ok((size,socket)) => (size,socket),
            Error => {
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

            let running: Arc<usize> = Arc::new(0);

            let mut client_state = ClientState {
                tx: sender,
                join_handle: Option::None,
            };

            let remote = src;
            let ctrl_sender = ctrl_sender.clone();
            
            let root = root.clone();
            let socket = socket.try_clone().unwrap();
            client_state.join_handle = Some(thread::spawn(move|| {
                connection::Connection::new(
                    receiver, 
                    root.clone(), 
                    remote,
                    socket).run();
            }));


            connections.insert(src,client_state);
            let _ = connections.get(&src).unwrap().tx.send(buf);
        }

        //cleanup
        cleanup_connections(&mut connections, &mut cleanpup_stopwatch);

        let value = u32::from_le_bytes([0x78, 0x56, 0x34, 0x12]);

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
        println!("Cleanput connection={:?}", i_con);
        state.join_handle.unwrap().join();
    }


    *stopwatch = Instant::now();
}
