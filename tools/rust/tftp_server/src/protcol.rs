use std::ffi::OsString;
use std::io::Read;
use std::net::{SocketAddr, UdpSocket};
use std::sync::Arc;
use std::sync::mpsc::Sender;
use std::time::Instant;
use std::{sync::mpsc::Receiver, time::Duration};
use std::default::Default;
use std::str;
use std::path::Path;
use std::fs::File;


pub const RECV_TIMEOUT:     Duration = Duration::from_secs(2);
pub const OPCODE_LEN:       usize    = 2;
pub const ACK_LEN:          usize    = 4;
pub const ACK_BLOCK_OFFSET: usize    = 2;


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
pub enum TftpError {
    NotDefined           = 0,
    FileNotFound         = 1,
    AccessViolation      = 2,
    DiskFull             = 3,
    IllegalOperation     = 4,
    UnknownTransferID    = 5,
    FileAlreadyExists    = 6,
    NoSuchUser           = 7,
}

pub fn raw_opcode(opcode: &Opcode) -> [u8;OPCODE_LEN] {
    let raw = *opcode as u16;
    return [(raw>>8) as u8, *opcode as u8];
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

pub fn num_to_raw<T>(number: T) -> Vec<u8>
    where 
        T: Copy + std::ops::Shr<usize,Output = T> + Into<u8>,
{
    let len = std::mem::size_of::<T>();

    let mut ret: Vec<u8> = Vec::new();
    for i in 0..len {
        let val_shifted = number>>(i*8);
        ret.push(Into::<u8>::into(val_shifted));
    }

    return ret;
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
    let num = match raw_to_num::<u16>(data) {
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
