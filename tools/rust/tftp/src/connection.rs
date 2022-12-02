use std::error::Error;
use std::fmt;
use std::{sync::mpsc::Receiver, time::Duration};
use std::default::Default;
use std::str;

const PAYLOAD_MAX:     usize    = 1500;
const RECV_TIMEOUT:    Duration = Duration::from_secs(2);
const OPCODE_LEN:      usize    = 2;
const SEP_LEN:         usize    = 1;
const REQUEST_MIN_LEN: usize    = OPCODE_LEN + SEP_LEN + 1 + SEP_LEN;
const ENTRY_IDX:       usize    = OPCODE_LEN + SEP_LEN;

#[derive(Debug)]
struct MyError {
    details: String
}

impl MyError {
    fn new(msg: &str) -> MyError {
        MyError{details: msg.to_string()}
    }
}

impl fmt::Display for MyError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f,"{}",self.details)
    }
}

#[derive(Clone,Copy,Debug)]
enum Opcode {
    Read  = 1,
    Write = 2,
    Data  = 3,
    Ack   = 4,
    Error = 5,
}

enum State {
    WAIT_REQUEST,
}

fn raw_opcode(opcode: &Opcode) -> [u8;OPCODE_LEN] {
    let raw = *opcode as u16;
    return [(raw>>8) as u8, *opcode as u8];
}

struct Request {
    opcode:   Opcode,
    filename: String,
}

fn raw_to_num<T: Copy + From<u8> + core::ops::BitOrAssign + core::ops::Shl<usize,Output=T>+Default>(data: &[u8]) -> Option<T> {
    let outlen = std::mem::size_of::<T>();
    if outlen > data.len() {
        return None
    }

    let mut out: T = Default::default();
    for i in 0..outlen {
        let curr = T::from(data[i]);
        out |= curr  << ((outlen-1-i)*8);
    }

    return Some(out);
}

fn parse_opcode(raw: u16) -> Option<Opcode> {
    match (raw) {
        x if x == Opcode::Read  as u16 => Some(Opcode::Read),
        x if x == Opcode::Write as u16 => Some(Opcode::Write),
        x if x == Opcode::Data  as u16 => Some(Opcode::Data),
        x if x == Opcode::Ack   as u16 => Some(Opcode::Ack),
        x if x == Opcode::Error as u16 => Some(Opcode::Error),
        _ => None,
    }
}

fn parse_opcode_raw(data: &[u8]) -> Option<Opcode> {
    let num = match raw_to_num::<u16>(data) {
        Some(num) => num,
        None          => return None
    };
    return parse_opcode(num);
}

fn parse_entries(data: &[u8]) -> Option<Vec<Vec<u8>>> {
    let mut ret: Vec<Vec<u8>> = vec![];

    if data.len() <= (OPCODE_LEN+1) {
        return None;
    };

    let entry_data = &data[OPCODE_LEN..];

    let mut next: Vec<u8> = vec![];
    for i in entry_data {
        if *i == 0x00 {
            ret.push(next.clone());
            next.resize(0, 0);
        }
        else {
            next.push(*i);
        }
    }

    if !next.is_empty() {
        return None;
    }
  
    return Some(ret);
}

enum HandshakeState {
    None,
}

pub struct Connection {
    state:     State,
    recv:      Receiver<Vec<u8>>,
    blocksize: usize,
    root:      String,
}

impl Connection {
    pub fn new(recva: Receiver<Vec<u8>>, roota: String) -> Connection {
        return Connection{
            state: State::WAIT_REQUEST, 
            recv: recva,
            blocksize: 512,
            root: roota,
        };
    }

    fn read(&mut self, filename: &str) {
        println!("read {}", filename);
    }
    fn write(&mut self, filename: &str) {

    }


    fn priv_run(&mut self)  {
        let data  = &self.recv.recv_timeout(RECV_TIMEOUT).unwrap()[..];

        println!("recv {:?}", data);

        let opcode = match parse_opcode_raw(data) {
            Some(val) => val,
            None              => return
        };

        println!("opcode {:?}", opcode);

        let entries = match parse_entries(data) {
            Some(x) => x,
            None => return
        };

        if entries.is_empty() {
            return;
        }

        let filename = match str::from_utf8(&entries[0][..]) {
            Ok(x) => x,
            Err(_) => return
        };

        println!("filename {}", filename);

        match opcode {
            Opcode::Read  => self.read(filename),
            Opcode::Write => self.write(filename),
            _             => return 
        }
    }

    pub fn run(&mut self) {
        self.priv_run();
    }

    fn recv(data: &[u8]) {

    }
}