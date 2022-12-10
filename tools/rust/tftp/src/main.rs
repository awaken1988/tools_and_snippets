use std::{
    net::{UdpSocket, SocketAddr}, 
    collections::HashMap, 
    sync::{mpsc::{Sender, channel}},
    thread};



mod connection;

fn main() {
    let mut connections = HashMap::<SocketAddr,Sender<Vec<u8>>>::new();

    let root = "C:/tftp".to_string();

    let socket = UdpSocket::bind("127.0.0.1:69").unwrap();

    loop {
        let mut buf = Vec::<u8>::new();
        buf.resize(4096, 0);

        //let (amt, src) = socket.get_mut().unwrap().as_mut().unwrap().recv_from(&mut buf).unwrap();
    
        let (amt, src) = socket.recv_from(&mut buf).unwrap();

        buf.resize(amt, 0);

        if connections.contains_key(&src) {
            let _ = connections.get(&src).unwrap().send(buf);
        }
        else {
            let (sender, receiver) = channel();

            connections.insert(src,sender);

            let remote = src;
            //remote.set_port(port);

            let root = root.clone();
            let socket = socket.try_clone().unwrap();
            thread::spawn(move|| {
                connection::Connection::new(
                    receiver, 
                    root.clone(), 
                    remote,
                    socket).run();
            });
            let _ = connections.get(&src).unwrap().send(buf);
        }

    }


}