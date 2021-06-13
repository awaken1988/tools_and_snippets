use std::io::Read;

pub struct GeneratorBuilder {
    pub name:      String, 
    pub generator: fn() -> Box<dyn std::io::Read>,
}