use std::io::Read;
use std::collections::HashMap;

pub struct GeneratorArgument
{
    pub name: &'static str,
}

pub type GeneratorArg = HashMap<String,String>;

pub struct GeneratorBuilder {
    pub name:      String, 
    pub generator: fn(arg: &HashMap<String,String>) -> Box<dyn std::io::Read>,
    pub arguments: HashMap<&'static str, GeneratorArgument>,
}

impl GeneratorArgument {
    pub fn new(name: &'static str) -> GeneratorArgument {
        GeneratorArgument {
            name: name
        }
    }
}