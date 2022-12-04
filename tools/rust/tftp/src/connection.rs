use std::error::Error;
use std::ffi::OsString;
use std::fmt;
use std::io::Read;
use std::net::{SocketAddr, UdpSocket};
use std::sync::{Mutex, Arc};
use std::time::Instant;
use std::{sync::mpsc::Receiver, time::Duration};
use std::default::Default;
use std::str;
use std::path::Path;
use std::fs::File;

const PAYLOAD_MAX:     usize    = 1500;
const RECV_TIMEOUT:    Duration = Duration::from_secs(2);
const OPCODE_LEN:      usize    = 2;
const SEP_LEN:         usize    = 1;
const REQUEST_MIN_LEN: usize    = OPCODE_LEN + SEP_LEN + 1 + SEP_LEN;
const ENTRY_IDX:       usize    = OPCODE_LEN + SEP_LEN;
const ACK_COUNT_LEN:   usize    = 2;

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

#[derive(Clone,Copy,Debug)]
enum TftpError {
    NotDefined           = 0,
    FileNotFound         = 1,
    AccessViolation      = 2,
    DiskFull             = 3,
    IllegalOperation     = 4,
    UnknownTransferID    = 5,
    FileAlreadyExists    = 6,
    NoSuchUser           = 7,
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
    state:      State,
    recv:       Receiver<Vec<u8>>,
    blocksize:  usize,
    root:       String,
    remote:     SocketAddr,
    socket:     UdpSocket,
}

impl Connection {
    pub fn new(recva: Receiver<Vec<u8>>, roota: String, remotea: SocketAddr, socketa: UdpSocket) -> Connection {
        return Connection{
            state: State::WAIT_REQUEST, 
            recv: recva,
            blocksize: 512,
            root: roota,
            remote: remotea,
            socket: socketa,
        };
    }

    fn send_raw(&mut self, data: &[u8]) {
        self.socket.send_to(data, self.remote).unwrap();
    }

    fn send_error(&mut self, error: TftpError, msg: Option<&str>) {
        let mut outmsg = match error {
            TftpError::NotDefined          => "Not defined",
            TftpError::FileNotFound        => "File not found.",
            TftpError::AccessViolation     => "Access violation.",
            TftpError::DiskFull            => "Disk full or allocation exceeded.",
            TftpError::IllegalOperation    => "Illegal TFTP operation.",
            TftpError::UnknownTransferID   => "Unknown transfer ID.",
            TftpError::FileAlreadyExists   => "File already exists.",
            TftpError::NoSuchUser          => "No such user.",
        };

        if let Some(x) = msg {
            outmsg = x; 
        }

        let opcode_raw = Opcode::Error as u16;
        let error_raw = error as u16;
        let mut buf = vec![
            (opcode_raw>>8) as u8, 
            (opcode_raw>>0) as u8 ,
            (error_raw>>8)  as u8, 
            (error_raw>>0)  as u8,];

        buf.extend_from_slice(outmsg.as_bytes());
        buf.push(0);

        self.send_raw(&buf);
    }

    fn wait_ack(&mut self, timeout: Duration) -> Result<(),()> {
        let now = Instant::now();

        loop {
           if now.elapsed() > timeout {
            break;
           }

            let data  = &self.recv.recv_timeout(timeout).unwrap()[..];

            let opcode = match parse_opcode_raw(data) {
                Some(val) => val,
                None              => continue
            };

            if data.len() < (OPCODE_LEN + ACK_COUNT_LEN) {
                continue;
            }

            match opcode {
                Opcode::Ack => (),
                _ => continue
            };





        }

        while now.elapsed() < timeout {
            
        }

        return Result::Err(());
    }

    fn read(&mut self, filename: &str) {
        println!("read {}", filename);

        let base_path    = OsString::from(&self.root);
        let request_path = OsString::from(&filename);
        let full_path     = Path::new(&base_path).join(request_path);

        if !full_path.starts_with(base_path) {
            self.send_error(TftpError::FileNotFound, None);
            return;
        }

        let mut file = match File::open(&full_path) {
            Err(_)      => return self.send_error(TftpError::NotDefined, None).to_owned(),
            Ok(x) => x,
        };

        let mut filebuf: Vec<u8> = vec![];
        let mut sendbuf: Vec<u8> = vec![];

        file.read_to_end(&mut filebuf);

        let mut i: usize = 0;

        while i<filebuf.len() {
            let remain = filebuf.len() - i;
            let remain = if remain >= self.blocksize { self.blocksize } else {remain};

            sendbuf.extend_from_slice(&raw_opcode(&Opcode::Data));
            sendbuf.extend_from_slice(&filebuf[i..(i+remain)]);

            //Send Frame
            self.send_raw(&sendbuf);

            //wait for ACK
            match self.wait_ack(RECV_TIMEOUT) {
                Ok(_) => (),
                Error(_) => return
            }
        }
        






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

    
}