use std::{fmt::write, time::{Instant, Duration}};

use clap::ArgMatches;
use std::net::UdpSocket;
use crate::protcol::{Opcode,PacketBuilder, TransferMode, Timeout, RECV_TIMEOUT, check_datablock, self, DATA_OFFSET, DEFAULT_BLOCKSIZE};


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

    let mut timeout = Timeout::new(RECV_TIMEOUT);

    loop {
        if timeout.is_timeout() {
            break;
        }

        match opcode {
            Opcode::Read => {

            }
            _ => panic!("not yet implemented"),
        }



    }


}

fn read_action(socket: &mut UdpSocket) {
    let mut timeout =  Timeout::new(RECV_TIMEOUT);
    let mut expected_block = 1u16;

    let _ = socket.set_read_timeout(Some(Duration::from_secs(1))); 

    let mut buf: Vec<u8> = Vec::new();

    loop {
        buf.resize(protcol::MAX_PACKET_SIZE, 0);

        if timeout.is_timeout() {
            println!("timeout");
            break;
        }

        let (amt, src) = match socket.recv_from(&mut buf) {
            Ok((size,socket)) => (size,socket),
            Err(_) => {
               println!("recv failed");
               break;
            }
        };

        if !check_datablock(&buf, expected_block) {
            continue;
        }

        //send ACK
        let _ = socket.send(PacketBuilder::new(&mut buf)
            .opcode(Opcode::Ack)
            .number16(expected_block).as_bytes());

        //handle  data
        {
            let recv_data = &buf[DATA_OFFSET..];

            if recv_data.len() < DEFAULT_BLOCKSIZE {
                break;
            }
        }

        //increase Blockcounter
        expected_block = expected_block.overflowing_add(1).0;
    }
}
