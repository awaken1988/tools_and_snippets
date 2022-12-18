use std::ffi::OsString;
use std::io::{Read, Write};
use std::net::{SocketAddr, UdpSocket};
use std::str::FromStr;
use std::sync::Arc;
use std::sync::mpsc::Sender;
use std::time::Instant;
use std::{sync::mpsc::Receiver, time::Duration};
use std::default::Default;
use std::{str, num};
use std::path::{Path, PathBuf};
use std::fs::File;
use std::path;

use byteorder::BigEndian;

use crate::mydef::{ServerSettings,WriteMode};
use crate::protcol::*;

pub struct Connection {
    recv:       Receiver<Vec<u8>>,
    remote:     SocketAddr,
    socket:     UdpSocket,
    settings:   ServerSettings,
}

impl Connection {

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

    fn send_ack(&mut self, blocknr: u16) {
        let mut msg: Vec<u8> = vec![];
        
        msg.extend_from_slice(&Opcode::Ack.raw());
        msg.extend_from_slice(&blocknr.to_be_bytes());

        self.send_raw(&msg);
    }

    fn wait_ack(&mut self, timeout: Duration, blocknr: u16) -> Result<(),()> {
        let now = Instant::now();

        loop {
           if now.elapsed() > timeout {
            break;
           }

            let data  = &self.recv.recv_timeout(Duration::from_secs(100)).unwrap()[..];

            let opcode = match parse_opcode_raw(data) {
                Some(val) => val,
                None              => continue
            };

            if data.len() != ACK_LEN {
                continue;
            }

            match opcode {
                Opcode::Ack => (),
                _ => continue
            };

            let (_,recv_blocknr) = data.split_at(ACK_BLOCK_OFFSET);
            let recv_blocknr            = raw_to_num::<u16>(recv_blocknr).unwrap();

            if recv_blocknr == blocknr {
                return Ok(());
            }
        }

        return Result::Err(());
    }

    fn wait_data(&mut self, timeout: Duration, blocknr: u16, out: &mut Vec<u8>) -> Result<(),()> {
        let now = Instant::now();

        loop {
           if now.elapsed() > timeout {
            break;
           }

            let data  = &self.recv.recv_timeout(Duration::from_secs(100)).unwrap()[..];

            let opcode = match parse_opcode_raw(data) {
                Some(val) => val,
                None              => continue
            };
        
            match opcode {
                Opcode::Data => (),
                _ => continue
            };

            if data.len() < (DATA_OFFSET+1) {
                continue;
            }

            //TODO: save ranges in protol
            let recv_blocknr = &data[DATA_BLOCK_NUM];
            let recv_blocknr = if let Some(nr) = raw_to_num::<u16>(recv_blocknr) {
                nr
            } else {
                continue;
            };

            if blocknr != recv_blocknr {
                continue;
            }
            
            let recv_data    = &data[DATA_OFFSET..];
            println!("rx block={}; len={}", recv_blocknr, recv_data.len());
            out.clear();
            out.extend_from_slice(recv_data);
            return Ok(());
        }

        return Result::Err(());
    }

    fn get_file_path(&self, path_relative: &str) -> Result<PathBuf,TftpError> {
        let base_path    = OsString::from(&self.settings.root_dir);
        let request_path = OsString::from(&path_relative);
        let full_path     = Path::new(&base_path).join(request_path);

        if !full_path.starts_with(base_path) {
            return Err(TftpError::FileNotFound);
        }

        return Ok(full_path.to_path_buf());
    }

    fn read(&mut self, filename: &str) {
        let full_path     = match self.get_file_path(filename) {
            Ok(full_path) => full_path,
            Err(errno) => {
                self.send_error(errno, Option::None);
                return;
            }
        };

        let mut file = match File::open(&full_path) {
            Err(_)      => return self.send_error(TftpError::NotDefined, None).to_owned(),
            Ok(x) => x,
        };

        let mut filebuf: Vec<u8> = vec![];
        let mut sendbuf: Vec<u8> = vec![];
        let mut blocknr: u16     = 1;

        filebuf.resize(self.settings.blocksize,0);
        

        loop {
            let payload_len = match file.read(&mut filebuf[0..self.settings.blocksize]) {
                Ok(len) => len,
                _ => return,
            };
            
            //send data
            sendbuf.resize(0, 0);
            sendbuf.extend_from_slice(&Opcode::Data.raw());
            sendbuf.push( (blocknr>>8) as u8 );
            sendbuf.push( (blocknr>>0) as u8 );

            if payload_len > 0 {
                sendbuf.extend_from_slice(&filebuf[0..payload_len]);
            }

            let _ = self.socket.send_to(&sendbuf, self.remote);

            //wait for ACK
            match self.wait_ack(RECV_TIMEOUT, blocknr) {
                Ok(_) => (),
                _ => return
            };

            //break condition
            if payload_len < self.settings.blocksize {
                break;
            }

            blocknr = blocknr.overflowing_add(1).0;
        }       
    }

    fn write(&mut self, filename: &str) {
        if self.settings.write_mode == WriteMode::DISABLED {
            self.send_error(TftpError::AccessViolation, Option::None);
            return;
        }

        let full_path     = match self.get_file_path(filename) {
            Ok(full_path) => full_path,
            Err(errno) => {
                self.send_error(errno, Option::None);
                return;
            }
        };

        let is_file = path::Path::new(full_path.as_os_str()).exists();
        let is_overwrite = self.settings.write_mode == WriteMode::WRITE_OVERWRITE;

        if is_file && !is_overwrite {
            self.send_error(TftpError::FileAlreadyExists, Option::None);
            return;
        }

        let mut file = match File::create(&full_path) {
            Err(_)      => return self.send_error(TftpError::NotDefined, None).to_owned(),
            Ok(x) => x,
        };  

        let mut block_num = 0u16;
        let mut data: Vec<u8> = vec![];
        self.send_ack(block_num);
        loop {
            block_num = block_num.wrapping_add(1);
            
            match self.wait_data(RECV_TIMEOUT, block_num, &mut data) {
                Ok(_) => {},
                Err(_) => {
                    self.send_error(TftpError::NotDefined, Some("write timeout"));
                    break;
                }
            };

            file.write(&data); 
            self.send_ack(block_num);

            if data.len() < self.settings.blocksize {
                break;
            }
        }
    }

    pub fn new(recv: Receiver<Vec<u8>>, remote: SocketAddr, socket: UdpSocket, settings: ServerSettings) -> Connection {
        return Connection{
            recv:      recv,
            remote:    remote,
            socket:    socket,
            settings:  settings,
        };
    }

    pub fn run(&mut self)  {
        let data   = &self.recv.recv_timeout(RECV_TIMEOUT).unwrap()[..];
        let opcode = match parse_opcode_raw(data) {
            Some(val) => val,
            None              => return
        };

        println!("{:?}", data);

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

        println!("Connection from {:?}; {:?} {}", self.remote, opcode, filename);

        match opcode {
            Opcode::Read  => self.read(filename),
            Opcode::Write => self.write(filename),
            _             => return 
        }
    }    
}