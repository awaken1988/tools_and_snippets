use std::{
    net::{UdpSocket, SocketAddr}, 
    collections::HashMap, 
    sync::{mpsc::{Sender, channel}, Arc, Mutex},
    thread};



mod connection;
mod mydef;

use mydef::ClientState;

fn main() {
    let mut connections = HashMap::<SocketAddr,ClientState>::new();
    //let root = "C:/tftp".to_string();
    let root = "/home/martin/src/".to_string();

    let socket = UdpSocket::bind("127.0.0.1:69").unwrap();

    let (ctrl_sender, ctrl_recv) = channel::<SocketAddr>();

    loop {
        let mut buf = Vec::<u8>::new();
        buf.resize(4096, 0);

    
        let (amt, src) = socket.recv_from(&mut buf).unwrap();

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
        

    }


}