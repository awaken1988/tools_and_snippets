use std::{sync::{Arc, mpsc::Sender}, thread::JoinHandle, fmt::Write};

use crate::protcol;

#[derive(Clone,PartialEq)]
pub enum WriteMode {
    DISABLED,
    WRITE_NEW,
    WRITE_OVERWRITE,
}

#[derive(Clone)]
pub struct ServerSettings {
    pub write_mode: WriteMode,
    pub root_dir:   String,
    pub blocksize:  usize,
}

impl ServerSettings {
    pub fn new(root_dir: String) -> ServerSettings {
        ServerSettings {
            write_mode: WriteMode::WRITE_NEW,
            root_dir: root_dir,
            blocksize: protcol::DEFAULT_BLOCKSIZE,
        }
    }
}


pub struct ClientState {
    pub tx: Sender<Vec<u8>>,
    pub join_handle: Option<JoinHandle<()>>,
}

