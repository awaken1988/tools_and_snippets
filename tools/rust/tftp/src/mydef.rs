use std::sync::{Arc, mpsc::Sender};

pub struct ClientState {
    pub running: Arc<usize>,
    pub tx: Sender<Vec<u8>>
}

