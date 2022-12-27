use std::{fmt::write, time::{Instant, Duration}};

use clap::ArgMatches;
use std::net::UdpSocket;
use crate::protcol::{Opcode,PacketBuilder, TransferMode};


pub fn client_main(args: &ArgMatches) {
    
    let read_filename  = args.get_one::<String>("read") ;
    let write_filename = args.get_one::<String>("write") ;

    let (filename, opcode) = match (read_filename,write_filename) {
        (Some(f), Option::None) => (f.clone(), Opcode::Read),
        (Option::None, Some(f)) => (f.clone(), Opcode::Write),
        _ => panic!("Only --read or --write allowed"),
    };

    let mut buf = Vec::new();

    let mut socket = UdpSocket::bind("127.0.0.1:0").expect("Bind to interface failed");
    socket.connect("127.0.0.1:69").expect("Connection failed");

    //send request
    socket.send(PacketBuilder::new(&mut buf)
        .opcode(opcode)
        .str(&filename)
        .separator()
        .transfer_mode(TransferMode::Octet)
        .separator()
        .as_bytes()).expect("ERR  : send tftp request failed");

}

fn wait_ack(socket: &mut UdpSocket) {
    let start_time = Instant::now();

    while start_time.elapsed() < Duration::from_secs(3) {
        socket.read_timeout(Duration::from_secs(1));

        
    }
}
