use std::{
    net::{UdpSocket, SocketAddr}, 
    collections::HashMap, 
    sync::{mpsc::{Sender, Receiver, channel}, Arc, Mutex},
    thread, time::Duration, cell::RefCell};



mod Connection;

fn main() {
    let mut connections = HashMap::<SocketAddr,Sender<Vec<u8>>>::new();

    let root = "C:/tftp".to_string();
    let port: u16 = 69;

    let socket = Arc::new(
        Mutex::new(
            UdpSocket::bind("127.0.0.1:69")
        )
    );

    loop {
        let mut buf = Vec::<u8>::new();
        buf.resize(4096, 0);

        //let (amt, src) = socket.get_mut().unwrap().as_mut().unwrap().recv_from(&mut buf).unwrap();
    
        let (amt, src) = socket.lock().unwrap().as_mut().unwrap().recv_from(&mut buf).unwrap();

        buf.resize(amt, 0);

        if connections.contains_key(&src) {
            connections.get(&src).unwrap().send(buf);
        }
        else {
            let (sender, receiver) = channel();

            connections.insert(src,sender);

            let mut remote = src;
            remote.set_port(port);

            let root = root.clone();
            let socket = Arc::clone(&socket).unwrap();
            thread::spawn(move|| {
                Connection::Connection::new(
                    receiver, 
                    root.clone(), 
                    remote,
                    socket).run();
            });
            connections.get(&src).unwrap().send(buf);
        }

    }


}