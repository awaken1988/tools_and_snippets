use std::{sync::{Arc, mpsc::Sender}, thread::JoinHandle};

pub struct ClientState {
    pub tx: Sender<Vec<u8>>,
    pub join_handle: Option<JoinHandle<()>>,
}

