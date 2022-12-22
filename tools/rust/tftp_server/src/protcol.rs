use std::ops::Range;
use std::time::Duration;
use std::default::Default;

pub const DEFAULT_BLOCKSIZE: usize    = 512;
pub const MAX_BLOCKSIZE:     usize    = 1024;

pub const MAX_PACKET_SIZE:   usize    = MAX_BLOCKSIZE + DATA_BLOCK_NUM.end;

pub const RECV_TIMEOUT:      Duration = Duration::from_secs(2);
pub const OPCODE_LEN:        usize    = 2;
pub const ACK_LEN:           usize    = 4;
pub const ACK_BLOCK_OFFSET:  usize    = 2;


pub const DATA_OFFSET:       usize        = 4;
pub const DATA_BLOCK_NUM:    Range<usize> = 2..4;




#[derive(Clone,Copy,Debug)]
pub enum Opcode {
    Read  = 1,
    Write = 2,
    Data  = 3,
    Ack   = 4,
    Error = 5,
}

#[allow(dead_code)]
#[derive(Clone,Copy,Debug)]
pub enum ErrorNumber {
    NotDefined           = 0,
    FileNotFound         = 1,
    AccessViolation      = 2,
    DiskFull             = 3,
    IllegalOperation     = 4,
    UnknownTransferID    = 5,
    FileAlreadyExists    = 6,
    NoSuchUser           = 7,
}

pub struct ErrorResponse {
    pub number: ErrorNumber,
    pub msg:    Option<String>,
}

impl ToString for ErrorNumber {
    fn to_string(&self) -> String {
        return match *self {
            ErrorNumber::NotDefined          => "Not defined".to_string(),
            ErrorNumber::FileNotFound        => "File not found.".to_string(),
            ErrorNumber::AccessViolation     => "Access violation.".to_string(),
            ErrorNumber::DiskFull            => "Disk full or allocation exceeded.".to_string(),
            ErrorNumber::IllegalOperation    => "Illegal TFTP operation.".to_string(),
            ErrorNumber::UnknownTransferID   => "Unknown transfer ID.".to_string(),
            ErrorNumber::FileAlreadyExists   => "File already exists.".to_string(),
            ErrorNumber::NoSuchUser          => "No such user.".to_string(),
        };    
    }
}

impl ToString for ErrorResponse {
    fn to_string(&self) -> String {
        if let Some(msg) = &self.msg {
            msg.clone()
        }
        else {
           self.number.to_string() 
        }
    }
}

impl  ErrorResponse {
    // pub fn new(number: ErrorNumber) -> ErrorResponse {
    //     ErrorResponse {
    //         number: number,
    //         msg: None
    //     }
    // }

    pub fn new_custom(msg: String) -> ErrorResponse {
        ErrorResponse {
            number: ErrorNumber::NotDefined,
            msg: Some(msg)
        }
    }
}



impl From<ErrorNumber> for ErrorResponse {
    fn from(number: ErrorNumber) -> Self {
        ErrorResponse { number: number, msg: Option::None }
    }
}



pub fn raw_to_num<T: Copy + From<u8> + core::ops::BitOrAssign + core::ops::Shl<usize,Output=T>+Default>(data: &[u8]) -> Option<T> {
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

impl Opcode {
    pub fn raw(self) -> [u8; OPCODE_LEN] {
       return (self as u16).to_be_bytes();
    }
}

impl From<Opcode> for u16 {
    fn from(value: Opcode) -> Self {
        return value as u16;
    }
}

pub fn parse_opcode(raw: u16) -> Option<Opcode> {
    match raw {
        x if x == Opcode::Read  as u16 => Some(Opcode::Read),
        x if x == Opcode::Write as u16 => Some(Opcode::Write),
        x if x == Opcode::Data  as u16 => Some(Opcode::Data),
        x if x == Opcode::Ack   as u16 => Some(Opcode::Ack),
        x if x == Opcode::Error as u16 => Some(Opcode::Error),
        _ => None,
    }
}

pub fn parse_opcode_raw(data: &[u8]) -> Option<Opcode> {
    let num =   match raw_to_num::<u16>(data) {
        Some(num) => num,
        None          => return None
    };
    return parse_opcode(num);
}

pub fn parse_entries(data: &[u8]) -> Option<Vec<Vec<u8>>> {
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




//TODO: function not required anymore but why does it not work
//pub fn num_to_raw<T>(number: T) -> Vec<u8>
//    where 
//        T: Copy + std::ops::Shr<usize,Output = T> + Into<u8>,
//{
//    let len = std::mem::size_of::<T>();
//
//    let mut ret: Vec<u8> = Vec::new();
//    for i in 0..len {
//        let val_shifted = number>>(i*8);
//        ret.push(Into::<u8>::into(val_shifted));
//    }
//
//    return ret;
//}