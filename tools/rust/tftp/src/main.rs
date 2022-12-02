use std::{
    net::{UdpSocket, SocketAddr}, 
    collections::HashMap, 
    sync::{mpsc::{Sender, Receiver, channel}, Arc},
    thread, time::Duration};



mod Connection;

fn main() {
    let mut connections = HashMap::<SocketAddr,Sender<Vec<u8>>>::new();

    let root = "C:/tftp".to_string();

    let socket = UdpSocket::bind("127.0.0.1:69").unwrap();

    loop {
        let mut buf = Vec::<u8>::new();
        buf.resize(4096, 0);

        let (amt, src) = socket.recv_from(&mut buf).unwrap();

        buf.resize(amt, 0);

        if connections.contains_key(&src) {
            connections.get(&src).unwrap().send(buf);
        }
        else {
            let (sender, receiver) = channel();

            connections.insert(src,sender);

            let root = root.clone();
            thread::spawn(move|| {
                Connection::Connection::new(receiver, root.clone()).run();
            });
            connections.get(&src).unwrap().send(buf);
        }

    }


}