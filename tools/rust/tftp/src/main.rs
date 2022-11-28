use std::{
    net::{UdpSocket, SocketAddr}, 
    collections::HashMap, 
    sync::mpsc::{Sender, Receiver, channel},
    thread, time::Duration};

const PAYLOAD_MAX:  usize    = 1500;
const RECV_TIMEOUT: Duration = Duration::from_secs(2);

fn main() {
    let mut connections = HashMap::<SocketAddr,Sender<Vec<u8>>>::new();

    let socket = UdpSocket::bind("127.0.0.1:11000").unwrap();

    loop {
        let mut buf = Vec::<u8>::new();

        let (amt, src) = socket.recv_from(&mut buf).unwrap();

        if connections.contains_key(&src) {
            connections.get(&src).unwrap().send(buf);
        }
        else {
            let (sender, receiver) = channel();

            connections.insert(src,sender);

            thread::spawn(move|| {
                loop {
                    let data  = receiver.recv_timeout(RECV_TIMEOUT).unwrap();
                    let data = &data[..];

                    
                }
            });
            connections.get(&src).unwrap().send(buf);
        }

    }


}