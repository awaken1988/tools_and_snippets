use std::{
    net::{UdpSocket, SocketAddr}, 
    collections::HashMap, 
    sync::{mpsc::{Sender, channel}, Arc},
    thread};



mod connection;
mod mydef;
mod Connection;

use mydef::ClientState;

fn main() {
    let mut connections = HashMap::<SocketAddr,ClientState>::new();

    let root = "C:/tftp".to_string();

    let socket = UdpSocket::bind("127.0.0.1:69").unwrap();

    loop {
        let mut buf = Vec::<u8>::new();
        buf.resize(4096, 0);

        //let (amt, src) = socket.get_mut().unwrap().as_mut().unwrap().recv_from(&mut buf).unwrap();
    
        let (amt, src) = socket.recv_from(&mut buf).unwrap();

        buf.resize(amt, 0);

        if connections.contains_key(&src) {
            let _ = connections.get(&src).unwrap().tx.send(buf);
        }
        else {
            let (sender, receiver) = channel();

            let running: Arc<usize> = Arc::new(0);

            let client_state = ClientState {
                running: running.clone() ,
                tx: sender
            };

            connections.insert(src,client_state);

            let remote = src;
            
            let root = root.clone();
            let socket = socket.try_clone().unwrap();
            thread::spawn(move|| {
                connection::Connection::new(
                    receiver, 
                    root.clone(), 
                    remote,
                    socket,
                    running.clone()).run();
            });
            let _ = connections.get(&src).unwrap().tx.send(buf);
        }

        //cleanup
        

    }


}